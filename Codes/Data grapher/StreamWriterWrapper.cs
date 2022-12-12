using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace DataGrapher
{
    public sealed class StreamWriterWrapper
    {
        private static readonly StreamWriterWrapper instance = new StreamWriterWrapper();

        // https://csharpindepth.com/Articles/Singleton#lazy
        // Fourth version
        static StreamWriterWrapper()
        {
        }

        public static StreamWriterWrapper Instance
        {
            get
            {
                return instance;
            }
        }

        Dictionary<string, StringBuilder> lines = new Dictionary<string, StringBuilder>();

        public void WriteLine(string FilePath, string Line)
        {
            if (!lines.ContainsKey(FilePath))
            {
                lines[FilePath] = new StringBuilder();
            }

            lines[FilePath].AppendLine(Line);
        }

        // must get called by SleeveResistanceGrapher.form_closing()
        public void Dispose()
        {
            foreach (KeyValuePair<string, StringBuilder> kvp in lines)
            {
                using (StreamWriter sw = File.AppendText(kvp.Key))
                {
                    sw.Write(kvp.Value.ToString());
                }
            }
        }
    }
}