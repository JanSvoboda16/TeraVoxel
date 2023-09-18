/*
 * Author: Jan Svoboda 
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY 
 */
using Microsoft.Extensions.Options;
using NuGet.Configuration;
using System.Buffers;
using System.ComponentModel.DataAnnotations;
using System.Text.Json;
using TeraVoxel.Server.Core;
using TeraVoxel.Server.Data.Models;
using TeraVoxel.Server.Data.Pipelines;

namespace TeraVoxel.Server.Data
{
    public class ProjectManager : IProjectManager
    {
        private StorageOptions _options;
        private IConvertingPipeline _convertor;
        private const string _deleteSuffix = ".DELETE";

        public ProjectManager(StorageOptions options, IConvertingPipeline convertor)
        {
            _options = options;
            _convertor = convertor;
        }

        public async Task CreateProject(string projectName)
        {
            var path = $"{_options.StoragePath}{projectName}";
            
            Directory.CreateDirectory($"{_options.StoragePath}{projectName}/{_options.SourceFileDirectory}");
            Directory.CreateDirectory($"{_options.StoragePath}{projectName}/{_options.DataDirectory}");

            var resultPath = $"{_options.StoragePath}{projectName}/{_options.DataDirectory}";
            ProjectInfo info = new ProjectInfo { Name = projectName, State = ProjectState.ProjectCreated };
            using var infoFile = File.Create($"{resultPath}info.json");
            await infoFile.WriteAsync(JsonSerializer.SerializeToUtf8Bytes(info));
        }

        public bool ProjectExists(string projectName)
        {
            return Directory.Exists(_options.StoragePath + projectName);
        }
        
        public void DeleteProject(string projectName)
        {
            // Hides original project until it's deleted. 
            Directory.Move(_options.StoragePath + projectName, _options.StoragePath + projectName + _deleteSuffix); 
            Task.Run(() => { Directory.Delete(_options.StoragePath + projectName + _deleteSuffix, true); });
        }

        public async Task ConvertProject(string projectName)
        {
            try
            {
                var sourcePath = $"{_options.StoragePath}{projectName}/{_options.SourceFileDirectory}";
                var resultPath = $"{_options.StoragePath}{projectName}/{_options.DataDirectory}";
                var sourceFiles = Directory.GetFiles(sourcePath);

                if (sourceFiles.Length == 0)
                {
                    throw new Exception("Project does not exist or does not contain any source file.");
                }
                var sourceFile = sourceFiles[0];

                ISourceFileDataReader reader;

                // HERE YOU CAN ADD OTHER SUPPORTED FILE TYPES
                if (sourceFile.Split('.').Last() == "nii")
                {
                    reader = new NiftiFileDataReader(sourceFile);
                }
                else
                {
                    throw new Exception("Unsupported file type");
                }

                await _convertor.Apply(reader, resultPath);
                reader.Dispose();
                File.Delete(sourceFile);

                var volumeInfo = new ProjectInfo()
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
                await WriteProjectInfo(projectName, volumeInfo);
            }
            catch
            {
                var info = await ReadProjectInfo(projectName);
                info.State = ProjectState.SourceFileUploaded;
                await WriteProjectInfo(projectName, info);
                throw;
            }
        }

        public async Task<List<ProjectInfo>> GetAllProjectsInfo()
        {
            var infos = new List<ProjectInfo>();
            var directories = Directory.GetDirectories(_options.StoragePath).Where(d => !d.Contains(_deleteSuffix));
            foreach (var dir in directories)
            {
                var filePath = $"{dir}//{_options.DataDirectory}info.json";
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
                    if (JsonSerializer.Deserialize<ProjectInfo>(File.ReadAllText(filePath)) is var json && json != null)
                    {
                        infos.Add(json);
                    }                    
                }
            }

            return infos;
        }

        public async Task<ProjectInfo> ReadProjectInfo(string projectName)
        {
            var filePath = $"{_options.StoragePath}{projectName}/{_options.DataDirectory}info.json";
            int retryCount = 0;
            string? data = null;

            
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
                
            return JsonSerializer.Deserialize<ProjectInfo>(data) ?? throw new Exception();
        }

        public async Task WriteProjectInfo(string projectName, ProjectInfo projectInfo)
        {
            var filePath = $"{_options.StoragePath}{projectName}/{_options.DataDirectory}info.json";
            FileStream? infoFile = null;
            int retryCount = 0;
            while (infoFile == null)
            {
                if (retryCount >= 5)
                {
                    throw new Exception("Unable to write into info file");
                }
                try
                {
                    infoFile = File.OpenWrite(filePath);
                }
                catch 
                {
                    await Task.Delay(1000);
                    retryCount++;
                }                
            }
            
            await infoFile.WriteAsync(JsonSerializer.SerializeToUtf8Bytes(projectInfo));
            await infoFile.DisposeAsync();
        }

        public async Task UploadSourceFile(string projectName, string fileName, Stream sourceStream)
        {
            var info = await ReadProjectInfo(projectName);
            byte[] buffer = ArrayPool<byte>.Shared.Rent(2_000_000);

            try
            {
                var sourceFileDirectoryPath = $"{_options.StoragePath}{projectName}/{_options.SourceFileDirectory}";
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

                info.State = ProjectState.SourceFileUploaded;
                await WriteProjectInfo(projectName, info);
            }
            catch
            {
                ArrayPool<byte>.Shared.Return(buffer);
                info.State = ProjectState.ProjectCreated;
                await WriteProjectInfo(projectName, info);
                throw;
            }
        }

        private FileStream GetSourceFileStream(string projectName, string fileName)
        {
            return File.Open($"{_options.StoragePath}{projectName}/{_options.SourceFileDirectory}{fileName}",FileMode.OpenOrCreate);
        }
    }
}