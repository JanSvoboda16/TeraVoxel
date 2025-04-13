using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Text.Json;
using System.Threading.Tasks;
using TeraVoxel.Server.Core;
using TeraVoxel.Server.Data.Models;

namespace TeraVoxel.Server.Data
{
    public class NeuroVoxelDataRepository : INeuroVoxelDataRepository
    {
        private NeuroVoxelStorageOptions _storageOptions;
        private string _deleteSuffix = ".DELETE";
        private string _createSuffix = ".CREATE";

        public NeuroVoxelDataRepository(NeuroVoxelStorageOptions storageOptions)
        {
            _storageOptions = storageOptions;
        }

        public async Task<string> GetDatasetMetadata(string datasetName)
        {
            var filePath = $"{_storageOptions.StoragePath}/{datasetName}/metadata.json";
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

                return data;
            }

            throw new FileNotFoundException();
        }

        public FileStream GetNodeStream(string dataset, int index, int level)
        {
            string path = $"{_storageOptions.StoragePath}/{dataset}/l{level}/{index}.json";
            if (!File.Exists(path)) 
            {
                path = $"{_storageOptions.StoragePath}/{dataset}/l{level}/{index}.bson.zlib";
            }

            return new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.Read);
        }

        public bool DatasetExists(string dataset)
        {
            var filePath = $"{_storageOptions.StoragePath}/{dataset}/metadata.json";
            return File.Exists(filePath);
        }

        public async Task<List<string>> GetDatasets()
        {
            var result = new List<string>();
            var storagePath = _storageOptions.StoragePath;

            var directories = Directory.GetDirectories(storagePath).Where(d => !d.Contains(_deleteSuffix) && !d.Contains(_createSuffix));
            foreach (var dir in directories)
            {
                var name = Path.GetFileName(dir);
                
                if(name is not null)
                {
                    result.Add(name);
                }               
            }

            return result;
        }
    }
}
