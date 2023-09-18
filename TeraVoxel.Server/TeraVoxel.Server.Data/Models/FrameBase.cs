/*
 * Author: Jan Svoboda 
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY 
 */

namespace TeraVoxel.Server.Data.Models
{
    public abstract class FrameBase
    {
        public int SizeX { get; set; }
        public int SizeY { get; set; }
        public FrameBase(int sizeX, int sizeY)
        {
            SizeX = sizeX;
            SizeY = sizeY;
        }
        public abstract FrameBase[] Split(int sizex, int sizey);
    }
}
