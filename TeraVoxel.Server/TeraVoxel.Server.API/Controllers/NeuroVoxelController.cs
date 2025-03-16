using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.RateLimiting;
using System.Buffers;
using TeraVoxel.Server.Data;

namespace TeraVoxel.Server.API.Controllers
{
    [Route("[controller]/[action]")]
    [ApiController]
    public class NeuroVoxelController : ControllerBase
    {
        INeuroVoxelDataRepository _dataRepository;

        public NeuroVoxelController(INeuroVoxelDataRepository repository)
        {
            _dataRepository = repository;
        }

        [HttpGet]
        public async Task<IActionResult> GetDatasets()
        {
            return Ok(await _dataRepository.GetDatasets());
        }

        [HttpGet]
        public async Task<IActionResult> GetDatasetMetadata(string datasetName)
        {
            return Ok(await _dataRepository.GetDatasetMetadata(datasetName));
        }

        [HttpGet]
        [DisableRateLimiting]
        public async Task<ActionResult> GetNode(string datasetName, int index, int level)
        {
            if (!_dataRepository.DatasetExists(datasetName))
            {
                return NotFound();
            }

            try
            {
                return File(_dataRepository.GetNodeStream(datasetName, index, level), "application/octet-stream");
            }
            catch
            {
                return NotFound();
            }
        }
    }
}
