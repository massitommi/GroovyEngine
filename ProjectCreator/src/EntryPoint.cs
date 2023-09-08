using ProjectCreator.forms;
using System;
using System.Collections.Generic;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ProjectCreator
{
    internal class EntryPoint
    {
        [STAThread]
        static void Main(string[] args)
        {
            Settings.Load();

            Application.EnableVisualStyles();
            Application.Run(new NewProject()); // or whatever

            Settings.Save();
        }
    }
}