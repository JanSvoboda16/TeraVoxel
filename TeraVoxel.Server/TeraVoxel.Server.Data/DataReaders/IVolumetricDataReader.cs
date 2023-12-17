/*
 * Author: Jan Svoboda 
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY 
 */
using TeraVoxel.Server.Data.Models;

namespace TeraVoxel.Server.Data
{
    public interface IVolumetricDataReader : IDisposable
    {
        public int FrameWidth { get; }
        public int FrameHeight { get; }
        public int CountOfFrames { get; }
        public Type DataType { get; }
        public float[] VoxelDimensions { get; }
        public void ReadFrame<T>(Frame<T> frame) where T : unmanaged;
        void SetFrameIndex(int frameIndex);
    }
}
