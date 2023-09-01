using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ProjectCreator.src.forms
{
    public partial class SettingsPage : Form
    {
        public SettingsPage()
        {
            InitializeComponent();
        }

        private void Settings_Load(object sender, EventArgs e)
        {
            EngineLocationTxt.Text = Settings.GetEngineLocation();
        }

        private void BrowseBtn_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog dialog = new FolderBrowserDialog();
            dialog.RootFolder = Environment.SpecialFolder.MyComputer;
            dialog.Description = "Select the engine workspace directory";

            if (dialog.ShowDialog() == DialogResult.OK)
                EngineLocationTxt.Text = dialog.SelectedPath;
        }

        private void SetEngineLocationBtn_Click(object sender, EventArgs e)
        {
            if (!Settings.IsValidEngineLocation(EngineLocationTxt.Text))
            {
                MessageBox.Show("Invalid engine path!", "Invalid engine path!", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            Settings.SetEngineLocation(EngineLocationTxt.Text);
            Close();
        }
    }
}
