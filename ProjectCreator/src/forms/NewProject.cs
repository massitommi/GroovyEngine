using ProjectCreator.src.forms;
using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.IO;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace ProjectCreator.forms
{
    public partial class NewProject : Form
    {
        public NewProject()
        {
            InitializeComponent();
        }

        bool mValidFolder = false;
        bool mValidName = false;

        void OnEngineLocationUpdated()
        {
            if (!Settings.IsEngineLocationSet())
            {
                SettingsAlertLbl.Text = "Engine location is not set!";
            }
            else
            {
                SettingsAlertLbl.Text = "";
            }
        }

        void UpdateCreateBtn()
        {
            CreateProjectBtn.Enabled = Settings.IsEngineLocationSet() && mValidFolder && mValidName;
        }

        private void NewProject_Load(object sender, EventArgs e)
        {
            NewProjectFolderTxt.Text = Environment.GetFolderPath(Environment.SpecialFolder.MyDocuments);
            NewProjectNameTxt.Text = "MyNewProject";
            SettingsBtn.Image = Image.FromFile(Path.Combine("res", "settings_icon.png"));

            OnEngineLocationUpdated();
        }

        private void SettingsBtn_Click(object sender, EventArgs e)
        {
            SettingsPage settingsWnd = new SettingsPage();
            settingsWnd.StartPosition = FormStartPosition.CenterParent;
            settingsWnd.ShowDialog();

            OnEngineLocationUpdated();
            UpdateCreateBtn();
        }

        private void CreateProjectBtn_Click(object sender, EventArgs e)
        {
            string fullPath = Path.Combine(NewProjectFolderTxt.Text, NewProjectNameTxt.Text);
            
            if(Directory.Exists(fullPath))
            {
                MessageBox.Show("Can't create project, path is already used", "Couldn't create project",MessageBoxButtons.OK, MessageBoxIcon.Error);
                return;
            }

            string templatePath = Path.Combine(Settings.GetEngineLocation(), "Templates", "empty_project");

            // copy template files
            Process copyCmd = new Process();
            copyCmd.StartInfo.UseShellExecute = true;
            copyCmd.StartInfo.FileName = Path.Combine(Environment.SystemDirectory, "xcopy.exe");
            copyCmd.StartInfo.Arguments = templatePath + " " + fullPath + " /Q /E /Y /I";
            copyCmd.Start();
            copyCmd.WaitForExit();

            if(copyCmd.ExitCode != 0)
            {
                MessageBox.Show(string.Format("xcopy error, cmd: {0}, error: {1}", copyCmd.StartInfo.Arguments, copyCmd.ExitCode));
                return;
            }

            // rename files
            File.Move(Path.Combine(fullPath, "PROJECT_NAME.groovyproj"), Path.Combine(fullPath, NewProjectNameTxt.Text + ".groovyproj"));
            Directory.Move(Path.Combine(fullPath, "PROJECT_NAME"), Path.Combine(fullPath, NewProjectNameTxt.Text));

            // fix lua script
            string premakeConfig = "";
            StreamReader preamakeReader = new StreamReader(Path.Combine(fullPath, "premake_config.lua"));
            premakeConfig = preamakeReader.ReadToEnd();
            preamakeReader.Close();

            premakeConfig = premakeConfig.Replace("$PROJECT_NAME", NewProjectNameTxt.Text);
            premakeConfig = premakeConfig.Replace("$ENGINE_LOCATION", Settings.GetEngineLocation().Replace('\\', '/'));

            StreamWriter premakeWriter = new StreamWriter(Path.Combine(fullPath, "premake_config.lua"));
            premakeWriter.Write(premakeConfig);
            premakeWriter.Close();

            // quick launch bat
            string launchBatTemplate = """
                @echo off
                chdir {0}
                start {1} {2}
                """;
            // {0} -> Working directory
            // {1} -> Exe to run
            // {2} -> Arguments

            {
                StreamWriter quickLaunchBat = new StreamWriter(Path.Combine(fullPath, "LaunchEditor.bat"), false);
                quickLaunchBat.Write(string.Format(launchBatTemplate,
                    Path.Combine(Settings.GetEngineLocation(), "Editor"),                                                       // working dir
                    Path.Combine(Settings.GetEngineLocation(), "bin", "Debug_Editor-windows-x86_64", "Editor", "Editor.exe"),   // exe
                    Path.Combine(fullPath, NewProjectNameTxt.Text + ".groovyproj")                                              // args
                    ));
                quickLaunchBat.Close();
            }

            {
                StreamWriter quickLaunchBat = new StreamWriter(Path.Combine(fullPath, "LaunchSandxbox.bat"), false);
                quickLaunchBat.Write(string.Format(launchBatTemplate,
                    Path.Combine(Settings.GetEngineLocation(), "Sandbox"),                                                      // working dir
                    Path.Combine(Settings.GetEngineLocation(), "bin", "Debug_Game-windows-x86_64", "Sandbox", "Sandbox.exe"),   // exe
                    Path.Combine(fullPath, NewProjectNameTxt.Text + ".groovyproj")                                              // args
                    ));
                quickLaunchBat.Close();
            }

            MessageBox.Show("ProjectCreator created successfully!");
        }

        private void BrowseBtn_Click(object sender, EventArgs e)
        {
            FolderBrowserDialog dialog = new FolderBrowserDialog();
            dialog.RootFolder = Environment.SpecialFolder.MyComputer;
            if(dialog.ShowDialog() == DialogResult.OK)
            {
                NewProjectFolderTxt.Text = dialog.SelectedPath;
                UpdateCreateBtn();
            }
        }

        private void NewProjectFolderTxt_TextChanged(object sender, EventArgs e)
        {
            if(!Directory.Exists(NewProjectFolderTxt.Text))
            {
                FolderAlertLbl.Text = "Invalid path";
                mValidFolder = false;
            }
            else
            {
                FolderAlertLbl.Text = "";
                mValidFolder = true;
            }

            UpdateCreateBtn();
        }

        private void NewProjectNameTxt_TextChanged(object sender, EventArgs e)
        {
            if (NewProjectNameTxt.Text.Length == 0)
            {
                NameAlertLbl.Text = "Empty name";
                mValidName = false;
            }
            else if (NewProjectNameTxt.Text.Contains(' '))
            {
                NameAlertLbl.Text = "Whitespaces aren't allowed here";
                mValidName = false;
            }
            else
            {
                NameAlertLbl.Text = "";
                mValidName = true;
            }

            UpdateCreateBtn();
        }
    }
}
