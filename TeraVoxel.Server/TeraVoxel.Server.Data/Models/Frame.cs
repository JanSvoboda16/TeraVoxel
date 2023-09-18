/*
 * Author: Jan Svoboda 
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY 
 */

namespace TeraVoxel.Server.Data.Models{

    public class Frame<T>: FrameBase
    {
        public T[]? Data { get; set; }
        public Frame(int sizeX, int sizeY) : base(sizeX, sizeY) { }

        protected Frame<T> GetSubFrame(int xIndex, int yIndex, int maxSizeX, int maxSizeY)
        {
            var subFrameSizeX = SizeX - xIndex * maxSizeX < maxSizeX ? SizeX - xIndex * maxSizeX : maxSizeX;
            var subFrameSizeY = SizeY - yIndex * maxSizeY < maxSizeY ? SizeY - yIndex * maxSizeY : maxSizeY;

            var subFrameStartX = xIndex * maxSizeX;
            var subFrameStartY = yIndex * maxSizeY;

            var data = new T[subFrameSizeX * subFrameSizeY];
            var dataIndex = 0;

            for (int y = subFrameStartY; y < subFrameStartY+subFrameSizeY; y++)
            {
                for (int x = subFrameStartX; x < subFrameStartX + subFrameSizeX; x++)
                {
                    data[dataIndex++] = (Data ?? throw new Exception())[y * SizeX + x];
                }
            }

            return new Frame<T>(subFrameSizeX, subFrameSizeY) { Data = data};
        }

        public override Frame<T>[] Split(int sizex, int sizey)
        {
            // Count of subframes in x axis            
            int xCount = (int)Math.Round(SizeX / (decimal)sizex, 0, MidpointRounding.ToPositiveInfinity);
            // Count of subframes in y axis
            int yCount = (int)Math.Round(SizeY / (decimal)sizey, 0, MidpointRounding.ToPositiveInfinity);
           
            var subFrames = new Frame<T>[xCount*yCount];

            for (int y = 0; y < yCount; y++)
            {
                for(int x = 0; x < xCount; x++)
                {
                    subFrames[y * xCount + x] = GetSubFrame(x, y, sizex, sizey);
                }
            } 

            return subFrames;
        }
    }
}
