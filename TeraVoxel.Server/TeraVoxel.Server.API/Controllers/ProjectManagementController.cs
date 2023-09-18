/*
 * Author: Jan Svoboda 
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY 
 */
using Microsoft.AspNetCore.Mvc;
using System.Diagnostics;
using TeraVoxel.Server.Data;
using TeraVoxel.Server.Data.Models;

namespace TeraVoxelServer.Controllers
{
    [ApiController]
    [Route("[controller]/[action]")]
    public class ProjectManagementController : ControllerBase
    {
        private IProjectManager _projectManager;
        SemaphoreSlim projectInfoSemaphore = new (1,1);

        public ProjectManagementController(IProjectManager projectManager)
        {
            _projectManager = projectManager;
        }

        [HttpGet]
        public async Task<IActionResult> CreateProject(string projectName)
        {
            await projectInfoSemaphore.WaitAsync();
            try
            {
                if (_projectManager.ProjectExists(projectName))
                {
                    return Conflict();
                }
                await _projectManager.CreateProject(projectName);
            }
            finally
            {
                projectInfoSemaphore.Release();
            }
          
            return Ok();
        }

        [HttpGet]
        public async Task<IActionResult> GetAllProjectsInfo()
        {
            return Ok(await _projectManager.GetAllProjectsInfo()); 
        }

        [HttpGet]
        public async Task<IActionResult> ConvertProject(string projectName)
        {
            await projectInfoSemaphore.WaitAsync(); // Prevent to convert project multiple times
            try
            {
                // Controlling and updating state of the project
                if (!_projectManager.ProjectExists(projectName))
                {
                    return NotFound();
                }

                var info = await _projectManager.ReadProjectInfo(projectName);
                if (info.State != ProjectState.SourceFileUploaded)
                {
                    return BadRequest();
                }
                info.State = ProjectState.ProjectConverting;
                await _projectManager.WriteProjectInfo(projectName, info);
            }
            finally
            {
               projectInfoSemaphore.Release();
            }          

            // Runnig project convertsion in a new thread.
            _ = Task.Run(() => _projectManager.ConvertProject(projectName));

            return Ok();
        }
        
        [HttpGet]
        public async Task<IActionResult> DeleteProject(string projectName)
        {
            await projectInfoSemaphore.WaitAsync(); // Prevent to delete a project when is used
            try
            {
                if (!_projectManager.ProjectExists(projectName))
                {
                    return NotFound();
                }

                var info = await _projectManager.ReadProjectInfo(projectName);
                if (info.State == ProjectState.ProjectConverting || info.State == ProjectState.SourceFileUploading)
                {
                    return Conflict();
                }

                _projectManager.DeleteProject(projectName);
            }
            finally
            {
                projectInfoSemaphore.Release();
            }
            return Ok();
        }

        [HttpPost]
        [DisableRequestSizeLimit]
        public async Task<IActionResult> UploadFile(string projectName, string fileName)
        {
            if (fileName.Split('.').Last() != "nii")
            {
                return BadRequest();
            }

            await projectInfoSemaphore.WaitAsync(); // Prevent to upload a file multiple times
            try
            {
                if (!_projectManager.ProjectExists(projectName))
                {
                    return NotFound();
                }

                var info = await _projectManager.ReadProjectInfo(projectName);
                if(info.State != ProjectState.ProjectCreated)
                {
                    return BadRequest();
                }
                info.State = ProjectState.SourceFileUploading;
                await _projectManager.WriteProjectInfo(projectName, info);
            }
            finally
            {
                projectInfoSemaphore.Release();
            }

            await _projectManager.UploadSourceFile(projectName, fileName, Request.Body);
            
            return Ok();            
        }
    }    
}