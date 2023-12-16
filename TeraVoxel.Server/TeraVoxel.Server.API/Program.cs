/*
 * Author: Jan Svoboda 
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY 
 */
using TeraVoxel.Server.API;
using TeraVoxel.Server.Core;
using TeraVoxel.Server.Data;
using TeraVoxel.Server.Data.Pipelines;

namespace TeraVoxelServer;
public class Program
{
    public static void Main(string[] args)
    {
        var builder = WebApplication.CreateBuilder(args);
        var configuration = new ConfigurationBuilder()
            .AddJsonFile("appsettings.json")
            .Build();        

        ConfigureServices(builder.Services, configuration);             

        var app = builder.Build();
        app.UseAuthorization();
        app.MapControllers();
        app.Run();        
    }

    private static void ConfigureServices(IServiceCollection services, IConfiguration configuration)
    {       
        var settingsSection = configuration.GetSection(SettingsOptions.SectionKey).Get<SettingsOptions>() ?? throw new Exception();

        services
            .AddSingleton<IProjectManager, ProjectManager>()
            .AddSingleton(settingsSection.Storage)
            .AddSingleton<IConvertingPipeline, ConvertingPipeline>()
            .AddSingleton<IVolumeDataRepository, VolumeDataRepository>()
            .AddSingleton<IEventLogger, EmptyEventLogger>()//((provider) => new FileEventLogger("log.csv"))
            .AddSingleton<IProjectInfoProvider, ProjectInfoProvider>()            
            .AddHostedService<AppRecoveryService>();

        services.AddControllers();
        services.AddEndpointsApiExplorer();
    }
}
