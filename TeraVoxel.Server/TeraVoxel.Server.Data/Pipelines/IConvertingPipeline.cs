/*
 * Author: Jan Svoboda 
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY 
 */

namespace TeraVoxel.Server.Data.Pipelines
{
    public interface IConvertingPipeline
    {
        public Task Apply(IVolumetricDataReader input, string destinatioDirectoryPath, string projectName, bool restoreImcomplete = false);
    }
}
