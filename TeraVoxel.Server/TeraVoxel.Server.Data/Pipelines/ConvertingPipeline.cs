/*
 * Author: Jan Svoboda 
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY 
 */

using System.Drawing;
using System.Linq;
using System.Numerics;
using TeraVoxel.Server.Core;
using TeraVoxel.Server.Data.Models;
using System.IO.Compression;

namespace TeraVoxel.Server.Data.Pipelines
{
    public class ConvertingPipeline: IConvertingPipeline
    {
        private StorageOptions _storageOptions;
        public ConvertingPipeline(StorageOptions options)
        {
            _storageOptions = options;
        }
        private void ReadFrame(FrameBase frame, ISourceFileDataReader reader)
        {
            if (reader.DataType == typeof(sbyte))
            {
                reader.ReadFrame(frame as Frame<sbyte> ?? throw new Exception());
                return;
            }
            else if (reader.DataType == typeof(byte))
            {
                reader.ReadFrame(frame as Frame<byte> ?? throw new Exception());
                return;
            }
            else if (reader.DataType == typeof(short))
            {
                reader.ReadFrame(frame as Frame<short> ?? throw new Exception());
                return;
            }
            else if (reader.DataType == typeof(ushort))
            {
                reader.ReadFrame(frame as Frame<ushort> ?? throw new Exception());
                return;
            }
            else if (reader.DataType == typeof(int))
            {
                reader.ReadFrame(frame as Frame<int> ?? throw new Exception());
                return;
            }
            else if (reader.DataType == typeof(uint))
            {
                reader.ReadFrame(frame as Frame<uint> ?? throw new Exception());
                return;
            }
            else if (reader.DataType == typeof(float))
            {
                reader.ReadFrame(frame as Frame<float> ?? throw new Exception());
                return;
            }
            else if (reader.DataType == typeof(double))
            {
                reader.ReadFrame(frame as Frame<double> ?? throw new Exception());
                return;
            }
            else if (reader.DataType == typeof(long))
            {
                reader.ReadFrame(frame as Frame<long> ?? throw new Exception());
                return;
            }
            else if (reader.DataType == typeof(ulong))
            {
                reader.ReadFrame(frame as Frame<ulong> ?? throw new Exception());
                return;
            }
            else if (reader.DataType == typeof(Color))
            {
                reader.ReadFrame(frame as Frame<Color> ?? throw new Exception());
                return;
            }
        }

        private VolumeSegment<T>[] MakeSegments<T>(int segmentSize, int count) where T : INumber<T>, IMinMaxValue<T>
        {
            VolumeSegment<T>[] segments = new VolumeSegment<T>[count];
            for (int i = 0; i < count; i++)
            {
                segments[i] = new VolumeSegment<T>(segmentSize);
            }
            return segments;
        }

        private RGBASegment[] MakeRGBASegments(int segmentSize, int count)
        {
            RGBASegment[] segments = new RGBASegment[count];
            for (int i = 0; i < count; i++)
            {
                segments[i] = new RGBASegment(segmentSize);
            }
            return segments;
        }

        private async Task CompressAndWrite(byte[] data, Stream fileStream)
        {
            using var compressStream = new GZipStream(fileStream, _storageOptions.CompressionLevel, true);
            await compressStream.WriteAsync(data, 0, data.Length);
        }

        public async Task Apply(ISourceFileDataReader reader, string destinationDirectoryPath)
        {
            FrameBase? frame = null;
            VolumeSegmentBase[]? segments = null;
            int segmentSize = _storageOptions.SegmentSize;
            
            int xCount = (int)Math.Round(reader.FrameWidth/ (decimal)segmentSize, 0, MidpointRounding.ToPositiveInfinity);
            int yCount = (int)Math.Round(reader.FrameHeight / (decimal)segmentSize, 0, MidpointRounding.ToPositiveInfinity);
            
            // Segments initialization
            if (reader.DataType == typeof(sbyte))
            {
                frame = new Frame<sbyte>(reader.FrameWidth, reader.FrameHeight);
                segments = MakeSegments<sbyte>(segmentSize, xCount * yCount);
            }
            else if (reader.DataType == typeof(byte))
            {
                frame = new Frame<byte>(reader.FrameWidth, reader.FrameHeight);
                segments = MakeSegments<byte>(segmentSize, xCount * yCount);
            }
            else if (reader.DataType == typeof(short))
            {
                frame = new Frame<short>(reader.FrameWidth, reader.FrameHeight);
                segments = MakeSegments<short>(segmentSize, xCount * yCount);
            }
            else if (reader.DataType == typeof(ushort))
            {
                frame = new Frame<ushort>(reader.FrameWidth, reader.FrameHeight);
                segments = MakeSegments<ushort>(segmentSize, xCount * yCount);
            }
            else if (reader.DataType == typeof(int))
            {
                frame = new Frame<int>(reader.FrameWidth, reader.FrameHeight);
                segments = MakeSegments<int>(segmentSize, xCount * yCount);                
            }
            else if (reader.DataType == typeof(uint))
            {
                frame = new Frame<uint>(reader.FrameWidth, reader.FrameHeight);
                segments = MakeSegments<uint>(segmentSize, xCount * yCount);
            }
            else if (reader.DataType == typeof(float))
            {
                frame = new Frame<float>(reader.FrameWidth, reader.FrameHeight);
                segments = MakeSegments<float>(segmentSize, xCount * yCount);
            }
            else if (reader.DataType == typeof(double))
            {
                frame = new Frame<double>(reader.FrameWidth, reader.FrameHeight);
                segments = MakeSegments<double>(segmentSize, xCount * yCount);
            }
            else if (reader.DataType == typeof(long))
            {
                frame = new Frame<long>(reader.FrameWidth, reader.FrameHeight);
                segments = MakeSegments<long>(segmentSize, xCount * yCount);
            }
            else if (reader.DataType == typeof(ulong))
            {
                frame = new Frame<ulong>(reader.FrameWidth, reader.FrameHeight);
                segments = MakeSegments<ulong>(segmentSize, xCount * yCount);
            }
            else if (reader.DataType == typeof(Color))
            {
                frame = new Frame<Color>(reader.FrameWidth, reader.FrameHeight);
                segments = MakeRGBASegments(segmentSize, xCount * yCount);
            }           
            else
            {
                throw new Exception();
            }

            Directory.CreateDirectory(destinationDirectoryPath);
            
            // Process
            int zIndex = 0;
            int frameIndex = 0;            
            while (frameIndex < reader.CountOfFrames)
            {
                var segmentDepth = reader.CountOfFrames - frameIndex >= segmentSize ? segmentSize : reader.CountOfFrames - frameIndex;
                
                // Loading a layer of the frames and splitting it into segments
                for (int segD = 0; segD < segmentDepth; segD++)
                {
                    ReadFrame(frame, reader);
                    int index=0;
                    foreach(var subframe in frame.Split(segmentSize, segmentSize))
                    {
                        if (segD == 0) {
                            segments[index].Init(index%xCount, index/xCount, zIndex);
                        }
                        segments[index].AddFrame(subframe);
                        index++;
                    };
                    frameIndex++;
                }
                
                // Conversion compresion downscaling
                List<Task> tasks = new List<Task>();
                foreach (var segment in segments)
                {
                    tasks.Add(Task.Run(async () =>
                    {
                        for (int downscale = 0; downscale < 4; downscale++)
                        {
                            using (var segFile = File.Create($"{destinationDirectoryPath}/data_{segment.XIndex}_{segment.YIndex}_{segment.ZIndex}_{downscale}"))
                            {
                                // Compressing data and writing into file
                                await CompressAndWrite(segment.ConvertToByteArray(), segFile);
                                // Downscaling
                                await Task.Run(() => segment.Downscale());
                            }
                        }
                    }));
                }
                await Task.WhenAll(tasks);
                zIndex++;
            }
        }
    }
}