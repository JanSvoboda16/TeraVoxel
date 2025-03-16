
namespace TeraVoxel.Server.Data
{
    public interface INeuroVoxelDataRepository
    {
        Task<string> GetDatasetMetadata(string datasetName);
        Task<List<string>> GetDatasets();
        FileStream GetNodeStream(string dataset, int index, int level);
        bool DatasetExists(string dataset);
    }
}