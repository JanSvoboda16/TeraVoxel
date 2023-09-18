/*
 * Author: Jan Svoboda 
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY 
 */

namespace TeraVoxel.Server.Data
{
    public interface IVolumeDataRepository
    {
        FileStream GetSegmentStream(string projectName, int xIndex, int yIndex, int zIndex, int downscale);
    }
}
