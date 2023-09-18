/*---------------------------------------------------------------------------*/
/* Authors: Patrick Prendergast, Jan Svoboda 
 * This class is based on library published by Patrick Prendergast 
 * under MIT license. For more information see LICENSE.txt.
 * His library can be found here: https://github.com/plwp/Nifti.NET.git
-----------------------------------------------------------------------------*/
using System.Drawing;
using System.IO;
using System.IO.Compression;
using System.Linq;
using System.Numerics;
using System.Text;
using System.Threading.Tasks;
using TeraVoxel.Server.Data.Models;

namespace TeraVoxel.Server.Data
{
    internal class NiftiFileDataReader : ISourceFileDataReader
    {
        private NiftiHeader _header;
        private Stream _stream;
        private bool _disposedValue;
        
        public int FrameWidth { get; private set; }
        public int FrameHeight { get; private set; }
        public int CountOfFrames { get; private set; }
        public Type DataType { get; private set; }
        public float[] VoxelDimensions { get; private set; } = new float[3];

        public NiftiFileDataReader(string filePath) 
        {
            _stream = ReadStream(filePath);
            _header = ReadHeader(_stream);

            if (FileType.HDR == TypeOf(_header))
            {
                var imgpath = filePath.ToLower().Replace(".hdr", ".img");

                if (File.Exists(imgpath))
                {
                    _stream = ReadStream(imgpath);
                }
            }

            if (TypeOf(_header) == FileType.UNKNOWN) throw new InvalidDataException("Not a NIfTI file (no magic bytes)");

            FrameWidth = _header.dim[1];
            FrameHeight = _header.dim[2];
            CountOfFrames = _header.dim[3];
            VoxelDimensions = _header.pixdim[1..4];
            DataType = GetDataType();
        }

        public NiftiHeader GetHeader()
        {
            return _header;
        }

        public enum FileType { NII, HDR, UNKNOWN }

        /// <summary>
        /// Read only the header of a nifti file (or header).
        /// </summary>
        /// <param name="path"></param>
        /// <returns></returns>
        public NiftiHeader ReadHeader(string path)
        {
            using (var stream = ReadStream(path))
            {
                var hdr = ReadHeader(stream);
                return hdr;
            }
        }

        /// <summary>
        /// Returns true is the file is GZipped (according to the magic bytes).
        /// </summary>
        /// <param name="path"></param>
        /// <returns></returns>
        public static bool IsCompressed(string path)
        {
            bool isCompressed = false;
            using (FileStream fs = File.OpenRead(path))
            {
                var b1 = fs.ReadByte();
                var b2 = fs.ReadByte();
                isCompressed = b1 == 0x01f && b2 == 0x8b;
            }
            return isCompressed;
        }

        private static Stream ReadStream(string path)
        {
            Stream fs = File.OpenRead(path);
            Stream s;
            
            if (IsCompressed(path))
            {
                s = new GZipStream(fs, CompressionMode.Decompress);
            }
            else
            {
                s = fs;
            }

            s.Position = 0;
            return s;
        }

        private static FileType TypeOf(NiftiHeader hdr)
        {
            // Magic bytes are "ni1\0" for hdr/img files and "n+1\0" for nii.
            if (hdr.magic[0] != 0x6E || hdr.magic[2] != 0x31 || hdr.magic[3] != 0x00) return FileType.UNKNOWN;
            else if (hdr.magic[1] == 0x69) return FileType.HDR;
            else if (hdr.magic[1] == 0x2B) return FileType.NII;
            else return FileType.UNKNOWN;
        }

        private T[] ReadData<T>(int lengt)
        {
            var datatype = _header.datatype;
            var reverseBytes = _header.SourceIsBigEndian();
            T[] data = new T[lengt];

            switch (data)
            {
                case sbyte[] dataT:
                    for (int i = 0; i < data.Length; ++i) dataT[i] = ReadSByte(_stream);
                    break;
                case byte[] dataT:
                    for (int i = 0; i < data.Length; ++i) dataT[i] = ReadByte(_stream);
                    break;                
                case int[] dataT:
                    for (int i = 0; i < data.Length; ++i) dataT[i] = ReadInt(_stream, reverseBytes);
                    break;
                case uint[] dataT:
                    for (int i = 0; i < data.Length; ++i) dataT[i] = ReadUInt(_stream, reverseBytes);
                    break;
                case short[] dataT:
                    for (int i = 0; i < data.Length; ++i) dataT[i] = ReadShort(_stream, reverseBytes);
                    break;
                case ushort[] dataT:
                    for (int i = 0; i < data.Length; ++i) dataT[i] = ReadUShort(_stream, reverseBytes);
                    break;
                case float[] dataT:
                    for (int i = 0; i < data.Length; ++i) dataT[i] = ReadFloat(_stream, reverseBytes);
                    break;
                case double[] dataT:
                    for (int i = 0; i < data.Length; ++i) dataT[i] = ReadDouble(_stream, reverseBytes);
                    break;
                case long[] dataT:
                    for (int i = 0; i < data.Length; ++i) dataT[i] = ReadLong(_stream, reverseBytes);
                    break;
                case ulong[] dataT:
                    for (int i = 0; i < data.Length; ++i) dataT[i] = ReadULong(_stream, reverseBytes);
                    break;
                case Color[] dataT:
                    if (datatype == NiftiHeader.DT_RGBA32)
                    {
                        for (int i = 0; i < data.Length; ++i) dataT[i] = ReadRGB(_stream, reverseBytes);
                    }
                    else if (datatype == NiftiHeader.DT_RGBA32) 
                    {
                        for (int i = 0; i < data.Length; ++i) dataT[i] = ReadRGBA(_stream, reverseBytes);
                    }
                    else
                    {
                        throw new Exception("Unknown data type");
                    }
                    break;
                
            }

            return data;
        }


        private  Type getDataType()
        {
            var datatype = _header.datatype;

            return typeof(int);
            
            //if (NiftiHeader.DT_BINARY == datatype) return new bool[bytelen * 8];
        }

        private static NiftiHeader ReadHeader(Stream stream)
        {
            bool reverseBytes = false;

            var streamLen = stream.Length;

            NiftiHeader hdr = new NiftiHeader();
            hdr.sizeof_hdr = ReadInt(stream, reverseBytes);

            reverseBytes = hdr.SourceIsBigEndian();

            hdr.data_type = ReadBytes(stream, 10);
            hdr.db_name = ReadBytes(stream, 18);
            hdr.extents = ReadInt(stream, reverseBytes);
            hdr.session_error = ReadShort(stream, reverseBytes);
            hdr.regular = ReadByte(stream);
            hdr.dim_info = ReadByte(stream);

            hdr.dim = ReadMyShorts(stream, 8, reverseBytes);
            hdr.intent_p1 = ReadFloat(stream, reverseBytes);
            hdr.intent_p2 = ReadFloat(stream, reverseBytes);
            hdr.intent_p3 = ReadFloat(stream, reverseBytes);
            hdr.intent_code = ReadShort(stream, reverseBytes);
            hdr.datatype = ReadShort(stream, reverseBytes);
            hdr.bitpix = ReadShort(stream, reverseBytes);
            hdr.slice_start = ReadShort(stream, reverseBytes);
            hdr.pixdim = ReadFloats(stream, 8, reverseBytes);
            hdr.vox_offset = ReadFloat(stream, reverseBytes);
            hdr.scl_slope = ReadFloat(stream, reverseBytes);
            hdr.scl_inter = ReadFloat(stream, reverseBytes);
            hdr.slice_end = ReadShort(stream, reverseBytes);
            hdr.slice_code = ReadByte(stream);
            hdr.xyzt_units = ReadByte(stream);
            hdr.cal_max = ReadFloat(stream, reverseBytes);
            hdr.cal_min = ReadFloat(stream, reverseBytes);
            hdr.slice_duration = ReadFloat(stream, reverseBytes);
            hdr.toffset = ReadFloat(stream, reverseBytes);
            hdr.glmax = ReadInt(stream, reverseBytes);
            hdr.glmin = ReadInt(stream, reverseBytes);

            hdr.descrip = ReadBytes(stream, 80);
            hdr.aux_file = ReadBytes(stream, 24);

            hdr.qform_code = ReadShort(stream, reverseBytes);
            hdr.sform_code = ReadShort(stream, reverseBytes);

            hdr.quatern_b = ReadFloat(stream, reverseBytes);
            hdr.quatern_c = ReadFloat(stream, reverseBytes);
            hdr.quatern_d = ReadFloat(stream, reverseBytes);
            hdr.qoffset_x = ReadFloat(stream, reverseBytes);
            hdr.qoffset_y = ReadFloat(stream, reverseBytes);
            hdr.qoffset_z = ReadFloat(stream, reverseBytes);

            hdr.srow_x = ReadFloats(stream, 4, reverseBytes);
            hdr.srow_y = ReadFloats(stream, 4, reverseBytes);
            hdr.srow_z = ReadFloats(stream, 4, reverseBytes);

            hdr.intent_name = ReadBytes(stream, 16);
            hdr.magic = ReadBytes(stream, 4);

            if (streamLen >= 352)
            {
                hdr.extension = ReadBytes(stream, 4);

                if (hdr.extension[0] == 1) // Extension is present
                {
                    hdr.esize = ReadInt(stream, reverseBytes);
                    hdr.ecode = ReadInt(stream, reverseBytes);
                    hdr.edata = ReadBytes(stream, hdr.esize - 8);
                }
            }

            if (TypeOf(hdr) == FileType.UNKNOWN) throw new InvalidDataException("Not a NIfTI file (no magic bytes)");
            if (hdr.dim[0] > 7) throw new InvalidDataException("NIFTI header is using more than 7 dimensions. I don't really know how to handle that :\\");
            else if (hdr.dim[0] < 0) throw new InvalidDataException("Somethings broken with the dimensions...");

            return hdr;
        }

        private static float[] ReadFloats(Stream stream, int count, bool reverseBytes)
        {
            var result = new float[count];
            for (var i = 0; i < count; ++i) result[i] = ReadFloat(stream, reverseBytes);
            return result;
        }

        private static float ReadFloat(Stream stream, bool reverseBytes)
        {
            return !reverseBytes ?
                BitConverter.ToSingle(ReadBytes(stream, 4), 0)
                : BitConverter.ToSingle(ReadBytesReversed(stream, 4), 0);
        }

        private static sbyte ReadSByte(Stream stream)
        {
            return (sbyte)stream.ReadByte();
        }

        private static byte ReadByte(Stream stream)
        {
            return (byte)stream.ReadByte();
        }

        private static int ReadInt(Stream stream, bool reverseBytes)
        {
            return !reverseBytes ?
                BitConverter.ToInt32(ReadBytes(stream, 4), 0)
                : BitConverter.ToInt32(ReadBytesReversed(stream, 4), 0);
        }

        private static uint ReadUInt(Stream stream, bool reverseBytes)
        {
            return !reverseBytes ?
                BitConverter.ToUInt32(ReadBytes(stream, 4), 0)
                : BitConverter.ToUInt32(ReadBytesReversed(stream, 4), 0);
        }

        private static ushort ReadUShort(Stream stream, bool reverseBytes)
        {
            return !reverseBytes ?
                BitConverter.ToUInt16(ReadBytes(stream, 2), 0)
                : BitConverter.ToUInt16(ReadBytesReversed(stream, 2), 0);
        }
        private static ushort[] ReadUrShorts(Stream stream, int count, bool reverseBytes)
        {
            var result = new ushort[count];
            for (var i = 0; i < count; ++i) result[i] = ReadUShort(stream, reverseBytes);
            return result;
        }

        private static short ReadShort(Stream stream, bool reverseBytes)
        {
            return !reverseBytes ?
                BitConverter.ToInt16(ReadBytes(stream, 2), 0)
                : BitConverter.ToInt16(ReadBytesReversed(stream, 2), 0);
        }

        private static short[] ReadMyShorts(Stream stream, int count, bool reverseBytes)
        {
            var result = new short[count];
            for (var i = 0; i < count; ++i) result[i] = ReadShort(stream, reverseBytes);
            return result;
        }

        private static double ReadDouble(Stream stream, bool reverseBytes)
        {
            return !reverseBytes ?
                BitConverter.ToDouble(ReadBytes(stream, 8), 0)
                : BitConverter.ToDouble(ReadBytesReversed(stream, 8), 0);
        }

        private static long ReadLong(Stream stream, bool reverseBytes)
        {
            return !reverseBytes ?
                BitConverter.ToInt64(ReadBytes(stream, 8), 0)
                : BitConverter.ToInt64(ReadBytesReversed(stream, 8), 0);
        }

        private static ulong ReadULong(Stream stream, bool reverseBytes)
        {
            return !reverseBytes ?
                BitConverter.ToUInt64(ReadBytes(stream, 8), 0)
                : BitConverter.ToUInt64(ReadBytesReversed(stream, 8), 0);
        }

        private static Color ReadRGB(Stream stream, bool reverseBytes)
        {
            byte[] rgb = ReadBytes(stream, 3);
            return Color.FromArgb(rgb[0], rgb[1], rgb[2]);
        }

        private static Color ReadRGBA(Stream stream, bool reverseBytes)
        {
            byte[] rgba = ReadBytes(stream, 4);
            return Color.FromArgb(rgba[3], rgba[0], rgba[1], rgba[2]);

        }

        private static byte[] ReadBytes(Stream stream, int count)
        {
            var result = new byte[count];
            for (var i = 0; i < count; ++i) result[i] = (byte)stream.ReadByte();
            return result;
        }

        private static byte[] ReadBytesReversed(Stream stream, int count)
        {
            var result = new byte[count];
            for (int i = count; i > 0; --i) result[i - 1] = (byte)stream.ReadByte();
            return result;
        }

        private Type GetDataType()
        {
            var datatype = _header.datatype;
            if (NiftiHeader.DT_FLOAT32 == datatype) return typeof(float);
            if (NiftiHeader.DT_INT8 == datatype) return typeof(sbyte);
            if (NiftiHeader.DT_UINT8 == datatype) return typeof(byte);
            if (NiftiHeader.DT_INT16 == datatype) return typeof(short);
            if (NiftiHeader.DT_INT32 == datatype) return typeof(int);
            if (NiftiHeader.DT_INT16 == datatype) return typeof(short);
            if (NiftiHeader.DT_UINT16 == datatype) return typeof(ushort);
            if (NiftiHeader.DT_DOUBLE == datatype) return typeof(double);
            if (NiftiHeader.DT_COMPLEX == datatype) return typeof(long);
            if (NiftiHeader.DT_RGB24 == datatype) return typeof(Color);
            if (NiftiHeader.DT_RGBA32 == datatype) return typeof(Color);
            throw new Exception();
        }
     
        public void ReadFrame<T>(Frame<T> frame)
        {
            var xdimlen = _header.dim[1];
            var ydimlen = _header.dim[2];
            var datatype = _header.datatype;
            var len = xdimlen * ydimlen;
            frame.Data = ReadData<T>(len);
        }

        protected virtual void Dispose(bool disposing)
        {
            if (!_disposedValue)
            {
                if (disposing)
                {
                    _stream.Dispose();
                }

                _disposedValue = true;
            }
        }

        public void Dispose()
        {
            // Neměňte tento kód. Kód pro vyčištění vložte do metody Dispose(bool disposing).
            Dispose(disposing: true);
            GC.SuppressFinalize(this);
        }
    }
}
