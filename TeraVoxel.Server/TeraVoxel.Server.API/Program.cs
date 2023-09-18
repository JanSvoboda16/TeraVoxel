/*
 * Author: Jan Svoboda 
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY 
 */
using TeraVoxel.Server.API;
using TeraVoxel.Server.Data;
using TeraVoxel.Server.Data.Pipelines;

namespace TeraVoxelServer;
public class Program
{
    private static IConfiguration _configuration;
    public static void Main(string[] args)
    {
        var builder = WebApplication.CreateBuilder(args);
        _configuration = new ConfigurationBuilder()
            .AddJsonFile("appsettings.json")
            .Build();

        ConfigureServices(builder.Services);                

        var app = builder.Build();
        app.UseAuthorization();
        app.MapControllers();
        app.Run();
    }

    public static void ConfigureServices(IServiceCollection services)
    {
        services.AddControllers();
        services.AddEndpointsApiExplorer();

        var settingsSection = _configuration.GetSection(SettingsOptions.SectionKey).Get<SettingsOptions>() ?? throw new Exception();

        services
            .AddSingleton<IProjectManager, ProjectManager>()
            .AddSingleton(settingsSection.Storage)
            .AddSingleton<IConvertingPipeline, ConvertingPipeline>()
            .AddSingleton<IVolumeDataRepository, VolumeDataRepository>();

    }
}
