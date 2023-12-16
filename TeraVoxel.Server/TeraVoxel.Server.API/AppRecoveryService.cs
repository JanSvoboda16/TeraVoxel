
using TeraVoxel.Server.Core;
using TeraVoxel.Server.Data;

namespace TeraVoxel.Server.API
{
    public class AppRecoveryService : IHostedService
    {
        private readonly IProjectManager _projectManager;
        private readonly IEventLogger _logger;

        public AppRecoveryService(IProjectManager projectManager, IEventLogger logger)
        {
            _projectManager = projectManager;
            _logger = logger;
        }

        public Task StartAsync(CancellationToken cancellationToken)
        {
            _ = Task.Run(TryRecoverUnfinishedConversions);
            return Task.CompletedTask;
        }

        public Task StopAsync(CancellationToken cancellationToken)
        {
            return Task.CompletedTask;
        }

        public async Task TryRecoverUnfinishedConversions()
        {
            try
            {
                var unFinishedProjects = (await _projectManager.GetAllProjectsInfo()).Where(i => i.State == TeraVoxel.Server.Data.Models.ProjectState.ProjectConverting).ToArray();

                foreach (var project in unFinishedProjects)
                {
                    try
                    {
                        _logger.Log(nameof(AppRecoveryService), "ProjectRecovery:Try", project.Name);
                        await _projectManager.ConvertProject(project.Name);
                    }
                    catch
                    {
                        _logger.Log(nameof(AppRecoveryService), "ProjectRecovery:Failed", project.Name);
                    }
                }
            }
            catch { }
        }
    }
}
