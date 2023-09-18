/*
 * Author: Jan Svoboda 
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY 
 */
using System.Drawing;
using System.Numerics;
using System.Runtime.InteropServices;
using System.IO.Compression;

namespace TeraVoxel.Server.Data.Models
{
    public abstract class VolumeSegmentBase
    {
        public int SegmentSize { get; protected set; }
        public int DownscaledSegmentSize { get; protected set; }
        public int ActualDepth { get; protected set; } 
        public int XIndex { get; protected set; }
        public int YIndex { get; protected set; }
        public int ZIndex { get; protected set; }        

        public VolumeSegmentBase(int segmentSize) 
        { 
            SegmentSize = segmentSize;
        }
        public abstract VolumeSegmentBase Init(int xIndex, int yIndex, int zIndex);
        
        public abstract byte[] ConvertToByteArray();
        public abstract VolumeSegmentBase AddFrame(FrameBase frame);
        public abstract VolumeSegmentBase Downscale();
    }

    public class VolumeSegment<T> : VolumeSegmentBase where T : INumber<T>, IMinMaxValue<T>
    {
        private T[][]? Data { get; set; }

        public VolumeSegment(int segmentSize) : base(segmentSize)
        {
            Data = new T[segmentSize][];
            for (int i = 0; i < segmentSize; i++)
            {
                Data[i] = new T[segmentSize * segmentSize];
            }
        }

        public override VolumeSegmentBase AddFrame(FrameBase frame)
        {
            int frameX = frame.SizeX;
            int frameY = frame.SizeY;

            var genFrame = (frame as Frame<T>);            

            if (genFrame?.Data != null && Data != null)
            {                
                for (int y = 0; y < DownscaledSegmentSize; y++)
                {
                    for (int x = 0; x < DownscaledSegmentSize; x++)
                    {
                        if (y < frameY && x < frameX)
                            Data[ActualDepth][y * DownscaledSegmentSize + x] = genFrame.Data[y * frameX + x];
                        else
                            Data[ActualDepth][y * DownscaledSegmentSize + x] = T.MinValue;
                    }
                }

                ActualDepth++;
            }

            return this;
        }

        public override VolumeSegmentBase Init(int xIndex, int yIndex, int zIndex)
        {                
            XIndex = xIndex;
            YIndex = yIndex;
            ZIndex = zIndex;
            DownscaledSegmentSize = SegmentSize;
            ActualDepth = 0;

            return this;
        }

        public override byte[] ConvertToByteArray()
        {
            byte[] buffer = new byte[DownscaledSegmentSize *DownscaledSegmentSize *DownscaledSegmentSize *Marshal.SizeOf<T>()];

            for (int i = 0; i < DownscaledSegmentSize; i++)
            {
                Buffer.BlockCopy(Data[i], 0, buffer, i * DownscaledSegmentSize * DownscaledSegmentSize * Marshal.SizeOf<T>(), DownscaledSegmentSize * DownscaledSegmentSize * Marshal.SizeOf<T>());
            }           

            return buffer;
        }

        public override VolumeSegmentBase Downscale()
        {
            DownscaledSegmentSize /= 2;            

            for (int z = 0; z < DownscaledSegmentSize; z++)
            {
                for (int y = 0; y < DownscaledSegmentSize; y++)
                {
                    for (int x = 0; x < DownscaledSegmentSize; x++)
                    {
                        // Couted as avarage of the 8 cells
                        var a0 = Convert.ToDouble(Data[z * 2][y * 4 * DownscaledSegmentSize + x * 2]);
                        var a1 = Convert.ToDouble(Data[z * 2][(y * 2) * 2 * DownscaledSegmentSize + x * 2+1]);
                        var a2 = Convert.ToDouble(Data[z * 2][(y * 2 + 1) * 2 * DownscaledSegmentSize + x * 2]);
                        var a3 = Convert.ToDouble(Data[z * 2][(y * 2 + 1) * 2 * DownscaledSegmentSize + x * 2 + 1]);
                        var a4 = Convert.ToDouble(Data[z * 2+1][y * 4 * DownscaledSegmentSize + x * 2]);
                        var a5 = Convert.ToDouble(Data[z * 2+1][(y * 2) * 2 * DownscaledSegmentSize + x * 2 + 1]);
                        var a6 = Convert.ToDouble(Data[z * 2+1][(y * 2 + 1) * 2 * DownscaledSegmentSize + x * 2]);
                        var a7 = Convert.ToDouble(Data[z * 2+1][(y * 2 + 1) * 2 * DownscaledSegmentSize + x * 2 + 1]);
                        Data[z][y * DownscaledSegmentSize + x] = (T)Convert.ChangeType((a0 + a1 + a2 + a3 + a4 + a5 + a6 + a7) / 8.0, typeof(T));
                    }
                }
            }

            return this;
        }
    }

    public class RGBASegment : VolumeSegmentBase 
    {
        private Color[][]? Data { get; set; }

        public RGBASegment(int segmentSize) : base(segmentSize) { }

        public override VolumeSegmentBase Init( int xIndex, int yIndex, int zIndex)
        {
           throw new NotImplementedException();
        }

        public override VolumeSegmentBase AddFrame(FrameBase frame)
        {
            throw new NotImplementedException();
        }

        public override byte[] ConvertToByteArray()
        {
            throw new NotImplementedException();
        }

        public override VolumeSegmentBase Downscale()
        {
            throw new NotImplementedException();
        }
    }
}
