/*
 * Author: Jan Svoboda 
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY 
 */
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc;
using Microsoft.AspNetCore.RateLimiting;
using Microsoft.AspNetCore.WebUtilities;
using Microsoft.Net.Http.Headers;
using System;
using System.Buffers;
using TeraVoxel.Server.Data;

namespace TeraVoxel.Server.API.Controllers
{
    [Route("[controller]/[action]")]    
    [ApiController]
    public class DataController : ControllerBase
    {
        private IVolumeDataRepository _tileRepository;
        private IProjectManager _projectManager;

        public DataController(IVolumeDataRepository tileRepository, IProjectManager projectManager) 
        {
            _tileRepository = tileRepository;
            _projectManager = projectManager;
        }

        [DisableRateLimiting]
        public async Task<ActionResult> GetBlock(string projectName, int xIndex, int yIndex, int zIndex, int downscale)
        {
            if (!_projectManager.ProjectExists(projectName))
            {
                return NotFound();
            }

            byte[] buffer = ArrayPool<byte>.Shared.Rent(2_000_000);
            try
            {
                return File(_tileRepository.GetSegmentStream(projectName, xIndex, yIndex, zIndex, downscale), "application/octet-stream");                
            }
            catch
            {
                ArrayPool<byte>.Shared.Return(buffer);
                return NotFound();
            }            
        }
    }
}
