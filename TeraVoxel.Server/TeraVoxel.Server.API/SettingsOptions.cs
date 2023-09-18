/*
 * Author: Jan Svoboda 
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY 
 */
using TeraVoxel.Server.Core;

namespace TeraVoxel.Server.API
{
    public class SettingsOptions
    {
        public const string SectionKey = "Settings";
        public StorageOptions Storage { get; set; } = null!;       
    }
}
