/*
 * Author: Jan Svoboda 
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY 
 */
using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using TeraVoxel.Server.Core;

namespace TeraVoxel.Server.Data
{
    public class VolumeDataRepository : IVolumeDataRepository
    {
        private StorageOptions _options;

        public VolumeDataRepository(StorageOptions options) 
        {
            _options = options;
        }

        public FileStream GetSegmentStream(string projectName, int xIndex, int yIndex, int zIndex, int downscale)
        {
            return new FileStream($"{_options.StoragePath}{projectName}/{_options.DataDirectory}data_{xIndex}_{yIndex}_{zIndex}_{downscale}", FileMode.Open, FileAccess.Read, FileShare.Read);
        }
    }
}
