/*
 * Author: Jan Svoboda 
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY 
 */
using System;
using System.Buffers;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using TeraVoxel.Server.Data.Models;
using TiffLibrary;
using TiffLibrary.PixelFormats;

namespace TeraVoxel.Server.Data.DataReaders.Tiff
{
    internal class TiffDataReader: IVolumetricDataReader
    {
        public int FrameWidth { get; private set; }
        public int FrameHeight { get; private set; }
        public int CountOfFrames { get; private set; }
        public Type DataType { get; private set; }
        public float[] VoxelDimensions { get; private set; }

        private string[] _imagePaths;
        private int _readIndex;
        private string _directoryPath;

        public TiffDataReader(string directoryPath)
        {
            _imagePaths = Directory.GetFiles(directoryPath).Order().ToArray();
            var tiff = TiffFileReader.Open(_imagePaths[0]);
            TiffImageFileDirectory ifd = tiff.ReadImageFileDirectory();
            // Create the decoder for the specified IFD.
            TiffImageDecoder decoder = tiff.CreateImageDecoder(ifd);
            FrameHeight = decoder.Height;
            FrameWidth = decoder.Width;
            CountOfFrames = _imagePaths.Count();
            VoxelDimensions = new float[]{ 1f, 1f, 1f};
            _directoryPath = directoryPath;
            _readIndex = 0;

            TiffFieldReader fieldReader = tiff.CreateFieldReader();
            TiffTagReader tagReader = new TiffTagReader(fieldReader, ifd);
   
            if (tagReader.ReadBitsPerSample().GetFirstOrDefault() == 8)
            {
                DataType = typeof(byte);
            }
            else
            {
                DataType = typeof(UInt16);
            }
        }

        public void ReadFrame<T>(Frame<T> frame) where T : unmanaged
        {
            using var tiff = TiffFileReader.Open(_imagePaths[_readIndex++]);
            TiffImageFileDirectory ifd = tiff.ReadImageFileDirectory();
            // Create the decoder for the specified IFD.
            TiffImageDecoder decoder = tiff.CreateImageDecoder(ifd);

            if (typeof(T) == typeof(byte))
            {
                var data = ArrayPool<TiffGray8>.Shared.Rent(FrameWidth * FrameHeight);

                TiffMemoryPixelBuffer<TiffGray8> pixelBuffer = new TiffMemoryPixelBuffer<TiffGray8>(data, FrameWidth, FrameHeight, writable: true);
                decoder.Decode<TiffGray8>(pixelBuffer);

                frame.Data = new T[FrameWidth * FrameHeight];

                for (int i = 0; i < FrameWidth * FrameHeight; i++)
                {
                    frame.Data[i] = (T)Convert.ChangeType(data[i].Intensity, typeof(T));
                }

                ArrayPool<TiffGray8>.Shared.Return(data);
            }
            else
            {
                var data = ArrayPool<TiffGray16>.Shared.Rent(FrameWidth * FrameHeight);

                TiffMemoryPixelBuffer<TiffGray16> pixelBuffer = new TiffMemoryPixelBuffer<TiffGray16>(data, FrameWidth, FrameHeight, writable: true);
                decoder.Decode<TiffGray16>(pixelBuffer);

                frame.Data = new T[FrameWidth * FrameHeight];

                for (int i = 0; i < FrameWidth * FrameHeight; i++)
                {
                    frame.Data[i] = (T)Convert.ChangeType(data[i].Intensity, typeof(T));
                }

                ArrayPool<TiffGray16>.Shared.Return(data);
            }
        }

        public void Dispose()
        {

        }

        public void SetFrameIndex(int frameIndex)
        {
            _readIndex = frameIndex;
        }
    }
}
