/*
 * Author: Jan Svoboda 
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY 
 */

using System.Collections.Generic;

namespace TeraVoxel.Server.Data.Models
{
    public enum ProjectState
    {
        ProjectCreated, 
        SourceFileUploading,
        SourceFileUploaded, 
        ProjectConverting,
        ProjectConverted
    }
    public class ProjectInfo
    {
        public string Name { get; set; } = "";
        public int SizeX { get; set; }
        public int SizeY { get; set; }
        public int SizeZ { get; set; }
        public string DataType { get; set; }= "";
        public float[] VoxelDimensions { get; set; } = new float[3];
        public int SegmentSize { get; set; }
        public int DataSizeX { get; set; }
        public int DataSizeY { get; set; }
        public int DataSizeZ { get; set; }
        public int LastProcessedSegmentLayer { get; set; }
        public bool IsLittleEndian { get; set; }
        public ProjectState State { get; set; }
    }
}
