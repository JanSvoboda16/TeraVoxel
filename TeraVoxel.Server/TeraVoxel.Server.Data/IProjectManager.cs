/*
 * Author: Jan Svoboda 
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY 
 */
using TeraVoxel.Server.Data.Models;

namespace TeraVoxel.Server.Data
{
    public interface IProjectManager
    {
        Task CreateProject(string projectName);
        bool ProjectExists(string projectName);
        void DeleteProject(string projectName);
        Task ConvertProject(string projectName);
        public Task<ProjectInfo> ReadProjectInfo(string projectName);
        Task<List<ProjectInfo>> GetAllProjectsInfo();
        Task WriteProjectInfo(string projectName, ProjectInfo projectInfo);
        Task UploadSourceFile(string projectName, string fileName, Stream sourceStream);
    }
}