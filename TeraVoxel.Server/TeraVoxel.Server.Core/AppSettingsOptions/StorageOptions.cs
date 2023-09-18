/*
 * Author: Jan Svoboda 
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY 
 */

using System.IO.Compression;

namespace TeraVoxel.Server.Core
{
    public class StorageOptions
    {
        public string? StoragePath { get; set; }      
        public string? SourceFileDirectory { get; set; }
        public string? DataDirectory { get; set; }
        public int SegmentSize { get; set; }
        public CompressionLevel CompressionLevel { get; set; }
    }
}
