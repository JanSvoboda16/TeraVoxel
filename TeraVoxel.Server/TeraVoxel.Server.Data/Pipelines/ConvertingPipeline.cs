﻿/*
 * Author: Jan Svoboda 
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY 
 */

using System.Drawing;
using System.Linq;
using System.Numerics;
using TeraVoxel.Server.Core;
using TeraVoxel.Server.Data.Models;
using System.IO.Compression;
using System.Threading.Tasks;
using System;
using System.Buffers;
using System.Runtime.CompilerServices;
using System.Runtime.Intrinsics.X86;

namespace TeraVoxel.Server.Data.Pipelines
{
    public class ConvertingPipeline : IConvertingPipeline
    {
        private readonly StorageOptions _storageOptions;
        private readonly IProjectInfoProvider _projectInfoProvider;

        private const uint Z_CURVE_MASK = 0b1001001001001001001001001001;

        public ConvertingPipeline(StorageOptions options, IProjectInfoProvider projectInfoProvider)
        {
            _storageOptions = options;
            _projectInfoProvider = projectInfoProvider;
        }

        private void ReadFrame(FrameBase frame, IVolumetricDataReader reader)
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
        }

        private VolumeSegment<T>[] MakeSegments<T>(int segmentSize, int count) where T : INumber<T>, IMinMaxValue<T>
        {
            VolumeSegment<T>[] segments = new VolumeSegment<T>[count];
            for (int i = 0; i < count; i++)
            {
                segments[i] = new VolumeSegment<T>(segmentSize, segmentSize, _storageOptions.ProcessLayerSize);
            }
            return segments;
        }

        private RGBASegment[] MakeRGBASegments(int segmentSize, int count)
        {
            RGBASegment[] segments = new RGBASegment[count];
            for (int i = 0; i < count; i++)
            {
                segments[i] = new RGBASegment(segmentSize, segmentSize, 8);
            }
            return segments;
        }

        private async Task Compress(Stream source, Stream destination)
        {
            using (GZipStream gzipStream = new GZipStream(destination, _storageOptions.CompressionLevel, true))
            {         
                await source.CopyToAsync(gzipStream);        
            }                
        }

        private async Task WriteToFile(byte[] data, string filePath, FileMode mode = FileMode.Create)
        {
            using var fileStream = File.Open(filePath, mode);
            await fileStream.WriteAsync(data, 0, data.Length);
        }

        public async Task Apply(IVolumetricDataReader reader, string destinationDirectoryPath, string projectName, bool restoreIncomplete = false)
        {
            FrameBase? frame = null;
            VolumeSegmentBase[]? subsegments = null;
            int segmentSize = _storageOptions.SegmentSize;
            int bytesPerItem = 0;
            
            int xCount = (int)Math.Round(reader.FrameWidth/ (decimal)segmentSize, 0, MidpointRounding.ToPositiveInfinity);
            int yCount = (int)Math.Round(reader.FrameHeight / (decimal)segmentSize, 0, MidpointRounding.ToPositiveInfinity);
            int zCount = (int)Math.Round(reader.CountOfFrames / (decimal)segmentSize, 0, MidpointRounding.ToPositiveInfinity);
            
            // Segments initialization
            if (reader.DataType == typeof(sbyte))
            {
                frame = new Frame<sbyte>(reader.FrameWidth, reader.FrameHeight);
                subsegments = MakeSegments<sbyte>(segmentSize, xCount * yCount);
                bytesPerItem = sizeof(sbyte);
            }
            else if (reader.DataType == typeof(byte))
            {
                frame = new Frame<byte>(reader.FrameWidth, reader.FrameHeight);
                subsegments = MakeSegments<byte>(segmentSize, xCount * yCount);
                bytesPerItem = sizeof(byte);
            }
            else if (reader.DataType == typeof(short))
            {
                frame = new Frame<short>(reader.FrameWidth, reader.FrameHeight);
                subsegments = MakeSegments<short>(segmentSize, xCount * yCount);
                bytesPerItem = sizeof(short);
            }
            else if (reader.DataType == typeof(ushort))
            {
                frame = new Frame<ushort>(reader.FrameWidth, reader.FrameHeight);
                subsegments = MakeSegments<ushort>(segmentSize, xCount * yCount);
                bytesPerItem = sizeof(ushort);
            }
            else if (reader.DataType == typeof(int))
            {
                frame = new Frame<int>(reader.FrameWidth, reader.FrameHeight);
                subsegments = MakeSegments<int>(segmentSize, xCount * yCount);
                bytesPerItem = sizeof(int);
            }
            else if (reader.DataType == typeof(uint))
            {
                frame = new Frame<uint>(reader.FrameWidth, reader.FrameHeight);
                subsegments = MakeSegments<uint>(segmentSize, xCount * yCount);
                bytesPerItem = sizeof(uint);
            }
            else if (reader.DataType == typeof(float))
            {
                frame = new Frame<float>(reader.FrameWidth, reader.FrameHeight);
                subsegments = MakeSegments<float>(segmentSize, xCount * yCount);
                bytesPerItem = sizeof(float);
            }
            else if (reader.DataType == typeof(double))
            {
                frame = new Frame<double>(reader.FrameWidth, reader.FrameHeight);
                subsegments = MakeSegments<double>(segmentSize, xCount * yCount);
                bytesPerItem = sizeof(double);
            }
            else if (reader.DataType == typeof(long))
            {
                frame = new Frame<long>(reader.FrameWidth, reader.FrameHeight);
                subsegments = MakeSegments<long>(segmentSize, xCount * yCount);
                bytesPerItem = sizeof(long);
            }
            else if (reader.DataType == typeof(ulong))
            {
                frame = new Frame<ulong>(reader.FrameWidth, reader.FrameHeight);
                subsegments = MakeSegments<ulong>(segmentSize, xCount * yCount);
                bytesPerItem = sizeof(ulong);
            }
            else if (reader.DataType == typeof(Color))
            {
                frame = new Frame<Color>(reader.FrameWidth, reader.FrameHeight);
                subsegments = MakeRGBASegments(segmentSize, xCount * yCount);
                bytesPerItem = 4;
            }           
            else
            {
                throw new Exception();
            }

            // Process
            int subsegmentZIndex = 0;
            int frameIndex = 0;

            if (restoreIncomplete)
            {
                var projectInfo = await _projectInfoProvider.ReadProjectInfo(projectName);
                frameIndex = projectInfo.LastProcessedSegmentLayer * segmentSize;
                subsegmentZIndex = projectInfo.LastProcessedSegmentLayer * (segmentSize / _storageOptions.ProcessLayerSize);

                reader.SetFrameIndex(frameIndex);
            }

            Directory.CreateDirectory(destinationDirectoryPath);            
                     
            while (frameIndex < reader.CountOfFrames)
            {
                for (int subsegmentIndex= 0; subsegmentIndex < segmentSize / _storageOptions.ProcessLayerSize; subsegmentIndex++)
                {               
                    var segmentDepth = reader.CountOfFrames - frameIndex >= _storageOptions.ProcessLayerSize ? _storageOptions.ProcessLayerSize : reader.CountOfFrames - frameIndex;

                    for (int i = 0; i < subsegments.Length; i++) {
                        subsegments[i].Init(i % xCount, i / xCount, subsegmentZIndex);
                    }

                    // Loading a layer of the frames and splitting it into segments
                    for (int segD = 0; segD < segmentDepth; segD++)
                    {
                        ReadFrame(frame, reader);
                        int index=0;
                        foreach(var subframe in frame.Split(segmentSize, segmentSize))
                        {
                            subsegments[index].AddFrame(subframe);
                            index++;
                        };
                        frameIndex++;
                    }
                
                    // Conversion compresion downscaling
                    List<Task> tasks = new List<Task>();
                    foreach (var subsegment in subsegments)
                    {
                        tasks.Add(Task.Run(async () =>
                        {
                            for (int downscale = 0; downscale < 4; downscale++)
                            {
                                int subsegmentsPerSegment = segmentSize / _storageOptions.ProcessLayerSize;
                                string rawPath = $"{destinationDirectoryPath}/data_{subsegment.XIndex}_{subsegment.YIndex}_{subsegment.ZIndex / subsegmentsPerSegment}_{downscale}.raw";

                                // Writing into file
                                await WriteToFile(subsegment.ConvertToByteArray(), rawPath, subsegment.ZIndex % subsegmentsPerSegment == 0 ? FileMode.Create : FileMode.Append);
                                
                                // Downscaling
                                subsegment.Downscale();                                

                                // if last subsegment in segment -> compression
                                if (subsegment.ZIndex % subsegmentsPerSegment == subsegmentsPerSegment - 1)
                                {
                                    int segmentLayer = subsegment.ZIndex / subsegmentsPerSegment;
                                    string resultPath = $"{destinationDirectoryPath}/data_{subsegment.XIndex}_{subsegment.YIndex}_{segmentLayer}_{downscale}";
                                    await TransformAndDelete(resultPath, rawPath, bytesPerItem, downscale);                                  
                                }
                            }
                        }));
                    }

                    await Task.WhenAll(tasks);

                    var projectInfo = await _projectInfoProvider.ReadProjectInfo(projectName);
                    projectInfo.LastProcessedSegmentLayer = frameIndex/segmentSize;
                    await _projectInfoProvider.WriteProjectInfo(projectName, projectInfo);                    

                    subsegmentZIndex++;
                }
            }
        }

        [MethodImpl(MethodImplOptions.AggressiveInlining)]
        private uint GetZetCurveIndex(UInt16 x, UInt16 y, UInt16 z)
        {
            return Bmi2.ParallelBitDeposit(x, Z_CURVE_MASK) | Bmi2.ParallelBitDeposit(y, Z_CURVE_MASK << 1) | Bmi2.ParallelBitDeposit(z, Z_CURVE_MASK << 2);
        }

        private async Task TransformAndDelete(string resultPath, string rawPath, int bytesPerItem, int downscale)
        {
            using (var segFileRaw = File.Open(rawPath, FileMode.Open))
            {
                using var resultFile = File.Open(resultPath, FileMode.Create);

                if (_storageOptions.ZTransformation)
                {
                    var segmentSize = _storageOptions.SegmentSize >> downscale;
                    var buffer = ArrayPool<byte>.Shared.Rent(segmentSize * segmentSize * segmentSize * bytesPerItem);

                    for (ushort z = 0; z < segmentSize; z++)
                    {
                        for (ushort y = 0; y < segmentSize; y++)
                        {
                            for (ushort x = 0; x < segmentSize; x++)
                            {
                                for (int i = 0; i < bytesPerItem; i++)
                                {
                                    buffer[GetZetCurveIndex(x, y, z) * bytesPerItem + i] = (byte)segFileRaw.ReadByte();
                                }
                            }
                        }
                    }

                    using var inputStream = new MemoryStream(buffer,0, segmentSize * segmentSize * segmentSize * bytesPerItem);
                    
                    if (_storageOptions.CompressionLevel == CompressionLevel.NoCompression)
                    {
                        await inputStream.CopyToAsync(resultFile);
                    }
                    else
                    {
                        await Compress(inputStream, resultFile);
                    }
                    
                    ArrayPool<byte>.Shared.Return(buffer);
                }
                else
                {
                    if (_storageOptions.CompressionLevel == CompressionLevel.NoCompression)
                    {
                        await segFileRaw.CopyToAsync(resultFile);
                    }
                    else
                    {
                        await Compress(segFileRaw, resultFile);
                    }                    
                }                
            };
            File.Delete(rawPath);          
        }
    }
}