using System;
using System.Collections.Generic;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace TeraVoxel.Server.Core
{
    public class EmptyEventLogger : IEventLogger
    {
        public void Log(string component, string action, string context = "", string value = "")
        {
            
        }
    }
}
