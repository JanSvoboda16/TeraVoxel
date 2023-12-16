/*
 * Author: Jan Svoboda 
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY 
 */
using Microsoft.Extensions.Options;
using NuGet.Configuration;
using System.Buffers;
using System.ComponentModel.DataAnnotations;
using System.IO.Compression;
using System.Text.Json;
using TeraVoxel.Server.Core;
using TeraVoxel.Server.Data.DataReaders.Tiff;
using TeraVoxel.Server.Data.Models;
using TeraVoxel.Server.Data.Pipelines;

namespace TeraVoxel.Server.Data
{
    public class ProjectManager : IProjectManager
    {
        private readonly StorageOptions _options;
        private readonly IProjectInfoProvider _projectInfoProvider;
        private readonly IConvertingPipeline _convertor;
        private readonly IEventLogger _logger;
        private const string _deleteSuffix = ".DELETE";

        public ProjectManager(StorageOptions options, IConvertingPipeline convertor, IEventLogger logger, IProjectInfoProvider projectInfoProvider)
        {
            _options = options;
            _convertor = convertor;
            _logger = logger;
            _projectInfoProvider = projectInfoProvider;
        }

        public async Task CreateProject(string projectName)
        {
            var path = $"{_options.StoragePath}/{projectName}";
            
            Directory.CreateDirectory($"{_options.StoragePath}/{projectName}/{_options.SourceFileDirectory}");
            Directory.CreateDirectory($"{_options.StoragePath}/{projectName}/{_options.DataDirectory}");

            var resultPath = $"{_options.StoragePath}/{projectName}/{_options.DataDirectory}";
            ProjectInfo info = new ProjectInfo { Name = projectName, State = ProjectState.ProjectCreated };
            using var infoFile = File.Create($"{resultPath}/info.json");
            await infoFile.WriteAsync(JsonSerializer.SerializeToUtf8Bytes(info));

            _logger.Log(nameof(ProjectManager), "ProjectCreated", projectName);
        }

        public bool ProjectExists(string projectName)
        {
            return Directory.Exists($"{_options.StoragePath}/{projectName}");
        }
        
        public void DeleteProject(string projectName)
        {
            // Hides original project until it's deleted. 
            Directory.Move($"{_options.StoragePath}/{projectName}", $"{_options.StoragePath}/{projectName}{_deleteSuffix}"); 
            Task.Run(() => { Directory.Delete($"{_options.StoragePath}/{projectName}{_deleteSuffix}", true); });

            _logger.Log(nameof(ProjectManager), "ProjectDeleted", projectName);
        }

        public async Task ConvertProject(string projectName)
        {
            try
            {
                _logger.Log(nameof(ProjectManager), "ProjectConversion:Started", projectName);

                var sourcePath = $"{_options.StoragePath}/{projectName}/{_options.SourceFileDirectory}";
                var resultPath = $"{_options.StoragePath}/{projectName}/{_options.DataDirectory}";
                var sourceFiles = Directory.GetFiles(sourcePath);

                if (sourceFiles.Length == 0)
                {
                    throw new Exception("Project does not exist or does not contain any source file.");
                }
                var sourceFile = sourceFiles[0];

                IVolumetricDataReader reader;

                // HERE YOU CAN ADD OTHER SUPPORTED FILE TYPES
                var fileType = sourceFile.Split('.').Last();
                if (fileType == "tif" || fileType == "tiff")
                {
                    reader = new TiffDataReader(Directory.GetParent(sourceFile)!.FullName);
                }
                else if (fileType == "nii")
                {
                    reader = new NiftiDataReader(sourceFile);
                }
                else
                {
                    throw new Exception("Unsupported file type");
                }

                var projectInfo = await _projectInfoProvider.ReadProjectInfo(projectName);
                projectInfo.State = ProjectState.ProjectConverting;
                await _projectInfoProvider.WriteProjectInfo(projectName, projectInfo);

                for (int i = 0; i < 5; i++)
                {
                    try
                    {
                        await _convertor.Apply(reader, resultPath, projectName, true);
                        break;
                    }
                    catch (Exception e)
                    {
                        await Task.Delay(2000);

                        _logger.Log(nameof(ProjectManager), "ProjectConversion:Failed", projectName, e.ToString());

                        if(i == 4)
                        {
                            throw;
                        }                        
                    }
                }
               
                reader.Dispose();

                foreach (var file in sourceFiles)
                {
                    File.Delete(file);
                }

                projectInfo = new ProjectInfo()
                {
                    DataSizeX = reader.FrameWidth,
                    DataSizeY = reader.FrameHeight,
                    DataSizeZ = reader.CountOfFrames,
                    SizeX = (int)Math.Ceiling(reader.FrameWidth / (float)_options.SegmentSize) * _options.SegmentSize,
                    SizeY = (int)Math.Ceiling(reader.FrameHeight / (float)_options.SegmentSize) * _options.SegmentSize,
                    SizeZ = (int)Math.Ceiling(reader.CountOfFrames / (float)_options.SegmentSize) * _options.SegmentSize,
                    DataType = reader.DataType.ToString(),
                    VoxelDimensions = reader.VoxelDimensions,
                    SegmentSize = _options.SegmentSize,
                    Name = projectName,
                    IsLittleEndian = BitConverter.IsLittleEndian,
                    State = ProjectState.ProjectConverted
                };

                await _projectInfoProvider.WriteProjectInfo(projectName, projectInfo);

                _logger.Log(nameof(ProjectManager), "ProjectConversion:Ended", projectName);
            }
            catch
            {
                var info = await _projectInfoProvider.ReadProjectInfo(projectName);
                info.State = ProjectState.SourceFileUploaded;
                await _projectInfoProvider.WriteProjectInfo(projectName, info);
                throw;
            }
        }

        public async Task<List<ProjectInfo>> GetAllProjectsInfo()
        {
            var infos = new List<ProjectInfo>();
            var directories = Directory.GetDirectories(_options.StoragePath).Where(d => !d.Contains(_deleteSuffix));
            foreach (var dir in directories)
            {
                var filePath = $"{dir}//{_options.DataDirectory}/info.json";
                if (File.Exists(filePath))
                {
                    string? data = null;
                    int retryCount = 0;
                    while (data == null)
                    {
                        if (retryCount >= 10)
                        {
                            throw new Exception("Unable to read info file");
                        }
                        try
                        {
                            data = await File.ReadAllTextAsync(filePath);
                        }
                        catch
                        {
                            await Task.Delay(200);
                            retryCount++;
                        }
                    }

                    if (JsonSerializer.Deserialize<ProjectInfo>(data) is var json && json != null)
                    {
                        infos.Add(json);
                    }                    
                }
            }

            return infos;
        }
        

        public async Task UploadSourceFile(string projectName, string fileName, Stream sourceStream)
        {
            _logger.Log(nameof(ProjectManager), "SourceFileUploading:Started", projectName);

            var info = await _projectInfoProvider.ReadProjectInfo(projectName);
            byte[] buffer = ArrayPool<byte>.Shared.Rent(2_000_000);

            try
            {
                var sourceFileDirectoryPath = $"{_options.StoragePath}/{projectName}/{_options.SourceFileDirectory}";
                Directory.Delete(sourceFileDirectoryPath, true);
                Directory.CreateDirectory(sourceFileDirectoryPath);

                using var file = GetSourceFileStream(projectName, fileName);

                // Much faster than CopyTo (Caused by implementation in HttpRequestStream)
                int bytesRead;
                do
                {
                    bytesRead = await sourceStream.ReadAsync(new Memory<byte>(buffer)).ConfigureAwait(false);
                    if (bytesRead > 0)
                    {
                        await file.WriteAsync(new ReadOnlyMemory<byte>(buffer, 0, bytesRead)).ConfigureAwait(false);
                    }
                }
                while (bytesRead > 0);

                file.Close();

                if (fileName.Split('.').Last() == "zip")
                {
                    var filePath = $"{_options.StoragePath}/{projectName}/{_options.SourceFileDirectory}/{fileName}";
                    ZipFile.ExtractToDirectory(filePath, sourceFileDirectoryPath);
                    File.Delete(filePath);
                }

                info.State = ProjectState.SourceFileUploaded;
                await _projectInfoProvider.WriteProjectInfo(projectName, info);

                _logger.Log(nameof(ProjectManager), "SourceFileUploading:Ended", projectName);
            }
            catch
            {
                info.State = ProjectState.ProjectCreated;
                await _projectInfoProvider.WriteProjectInfo(projectName, info);

                _logger.Log(nameof(ProjectManager), "SourceFileUploading:Failed", projectName);

                throw;
            }
            finally
            {
                ArrayPool<byte>.Shared.Return(buffer);
            }
        }

        private FileStream GetSourceFileStream(string projectName, string fileName)
        {
            return File.Open($"{_options.StoragePath}/{projectName}/{_options.SourceFileDirectory}/{fileName}",FileMode.OpenOrCreate);
        }

        public async Task<ProjectInfo> ReadProjectInfo(string projectName)
        {
            return await _projectInfoProvider.ReadProjectInfo(projectName);
        }

        public async Task WriteProjectInfo(string projectName, ProjectInfo projectInfo)
        {
            await _projectInfoProvider.WriteProjectInfo(projectName, projectInfo);
        }
    }
}