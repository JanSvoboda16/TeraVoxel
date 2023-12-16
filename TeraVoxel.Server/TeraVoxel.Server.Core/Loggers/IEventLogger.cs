/*
 * Author: Jan Svoboda 
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY 
 */
namespace TeraVoxel.Server.Core
{
    public interface IEventLogger
    {
        void Log(string component, string action, string context = "", string value = "");
    }
}
