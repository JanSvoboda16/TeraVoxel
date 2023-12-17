/*
 * Author: Jan Svoboda 
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY 
 */
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;
using TeraVoxel.Server.Core;
using TeraVoxel.Server.Data.Models;
using TeraVoxel.Server.Data.Pipelines;

namespace TeraVoxel.Server.Data
{
    public interface IProjectInfoProvider
    {
        Task<ProjectInfo> ReadProjectInfo(string projectName);
        Task WriteProjectInfo(string projectName, ProjectInfo projectInfo);
    }

    public class ProjectInfoProvider : IProjectInfoProvider
    {
        private readonly StorageOptions _options;

        public ProjectInfoProvider(StorageOptions options)
        {
            _options = options;
        }

        public async Task<ProjectInfo> ReadProjectInfo(string projectName)
        {
            var filePath = $"{_options.StoragePath}/{projectName}/{_options.DataDirectory}/info.json";
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
            var filePath = $"{_options.StoragePath}/{projectName}/{_options.DataDirectory}/info.json";
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
    }
}
