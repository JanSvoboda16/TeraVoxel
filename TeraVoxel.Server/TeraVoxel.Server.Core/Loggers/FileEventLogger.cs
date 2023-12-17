/*
 * Author: Jan Svoboda 
 * University: BRNO UNIVERSITY OF TECHNOLOGY, FACULTY OF INFORMATION TECHNOLOGY 
 */
using System;
using System.Diagnostics;

namespace TeraVoxel.Server.Core
{
    public class FileEventLogger : IEventLogger
    {
        private readonly Stopwatch _stopwatch;
        private readonly StreamWriter _streamWriter;
        private object _lock = new object();
        public FileEventLogger(string filePath, bool append = false)
        {
            _streamWriter = new StreamWriter(filePath, append);
            _stopwatch = Stopwatch.StartNew();
        }

        public void Log(string component, string action, string context, string value)
        {
            lock (_lock)
            {
                _streamWriter.WriteLine($"{component};{action};{context};{value};{_stopwatch.Elapsed}");
                _streamWriter.Flush();
            }
        }

        ~FileEventLogger()
        {
           _streamWriter.Close();
        }
    }
}
