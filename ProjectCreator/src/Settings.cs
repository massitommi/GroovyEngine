using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;

namespace ProjectCreator
{
    static class Settings
    {
        static readonly string SETTINGS_FILE = "settings";
        static string sEngineLocation = "";

        public static void Load()
        {
            if(File.Exists(SETTINGS_FILE))
            {
                StreamReader r = new StreamReader(SETTINGS_FILE);
                string engineLocation = r.ReadToEnd();
                r.Close();

                if (IsValidEngineLocation(engineLocation))
                    sEngineLocation = engineLocation;
            }
        }

        public static void Save()
        {
            StreamWriter w = new StreamWriter(SETTINGS_FILE, false);
            w.Write(sEngineLocation);
            w.Close();
        }

        public static bool IsValidEngineLocation(string engineLocation)
        {
            return 
                engineLocation.Length > 0 &&
                Directory.Exists(engineLocation) &&
                engineLocation.Contains("GroovyEngine");
        }

        public static string GetEngineLocation() => sEngineLocation;

        public static bool IsEngineLocationSet() => sEngineLocation.Length > 0;

        public static void SetEngineLocation(string engineLocation)
        {
            if (IsValidEngineLocation(engineLocation))
                sEngineLocation = engineLocation;
        }
    }
}
