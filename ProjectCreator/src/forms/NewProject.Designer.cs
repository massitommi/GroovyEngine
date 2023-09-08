namespace ProjectCreator.forms
{
    partial class NewProject
    {
        /// <summary>
        /// Required designer variable.
        /// </summary>
        private System.ComponentModel.IContainer components = null;

        /// <summary>
        /// Clean up any resources being used.
        /// </summary>
        /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
        protected override void Dispose(bool disposing)
        {
            if (disposing && (components != null))
            {
                components.Dispose();
            }
            base.Dispose(disposing);
        }

        #region Windows Form Designer generated code

        /// <summary>
        /// Required method for Designer support - do not modify
        /// the contents of this method with the code editor.
        /// </summary>
        private void InitializeComponent()
        {
            System.ComponentModel.ComponentResourceManager resources = new System.ComponentModel.ComponentResourceManager(typeof(NewProject));
            this.label2 = new System.Windows.Forms.Label();
            this.NewProjectFolderTxt = new System.Windows.Forms.TextBox();
            this.BrowseBtn = new System.Windows.Forms.Button();
            this.SettingsBtn = new System.Windows.Forms.PictureBox();
            this.NewProjectNameTxt = new System.Windows.Forms.TextBox();
            this.label1 = new System.Windows.Forms.Label();
            this.CreateProjectBtn = new System.Windows.Forms.Button();
            this.SettingsAlertLbl = new System.Windows.Forms.Label();
            this.FolderAlertLbl = new System.Windows.Forms.Label();
            this.NameAlertLbl = new System.Windows.Forms.Label();
            ((System.ComponentModel.ISupportInitialize)(this.SettingsBtn)).BeginInit();
            this.SuspendLayout();
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Segoe UI", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label2.Location = new System.Drawing.Point(9, 123);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(69, 21);
            this.label2.TabIndex = 6;
            this.label2.Text = "Location";
            // 
            // NewProjectFolderTxt
            // 
            this.NewProjectFolderTxt.Font = new System.Drawing.Font("Segoe UI", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.NewProjectFolderTxt.Location = new System.Drawing.Point(13, 147);
            this.NewProjectFolderTxt.Name = "NewProjectFolderTxt";
            this.NewProjectFolderTxt.Size = new System.Drawing.Size(411, 25);
            this.NewProjectFolderTxt.TabIndex = 5;
            this.NewProjectFolderTxt.Text = "C:/Stuff/";
            this.NewProjectFolderTxt.TextChanged += new System.EventHandler(this.NewProjectFolderTxt_TextChanged);
            // 
            // BrowseBtn
            // 
            this.BrowseBtn.Font = new System.Drawing.Font("Segoe UI", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.BrowseBtn.Location = new System.Drawing.Point(430, 147);
            this.BrowseBtn.Name = "BrowseBtn";
            this.BrowseBtn.Size = new System.Drawing.Size(36, 25);
            this.BrowseBtn.TabIndex = 9;
            this.BrowseBtn.Text = "...";
            this.BrowseBtn.UseVisualStyleBackColor = true;
            this.BrowseBtn.Click += new System.EventHandler(this.BrowseBtn_Click);
            // 
            // SettingsBtn
            // 
            this.SettingsBtn.Cursor = System.Windows.Forms.Cursors.Hand;
            this.SettingsBtn.Location = new System.Drawing.Point(12, 12);
            this.SettingsBtn.Name = "SettingsBtn";
            this.SettingsBtn.Size = new System.Drawing.Size(30, 30);
            this.SettingsBtn.SizeMode = System.Windows.Forms.PictureBoxSizeMode.Zoom;
            this.SettingsBtn.TabIndex = 16;
            this.SettingsBtn.TabStop = false;
            this.SettingsBtn.Click += new System.EventHandler(this.SettingsBtn_Click);
            // 
            // NewProjectNameTxt
            // 
            this.NewProjectNameTxt.Font = new System.Drawing.Font("Segoe UI", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.NewProjectNameTxt.Location = new System.Drawing.Point(507, 147);
            this.NewProjectNameTxt.Name = "NewProjectNameTxt";
            this.NewProjectNameTxt.Size = new System.Drawing.Size(184, 25);
            this.NewProjectNameTxt.TabIndex = 17;
            this.NewProjectNameTxt.Text = "GTA6";
            this.NewProjectNameTxt.TextChanged += new System.EventHandler(this.NewProjectNameTxt_TextChanged);
            // 
            // label1
            // 
            this.label1.AutoSize = true;
            this.label1.Font = new System.Drawing.Font("Segoe UI", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label1.Location = new System.Drawing.Point(503, 123);
            this.label1.Name = "label1";
            this.label1.Size = new System.Drawing.Size(52, 21);
            this.label1.TabIndex = 18;
            this.label1.Text = "Name";
            // 
            // CreateProjectBtn
            // 
            this.CreateProjectBtn.Font = new System.Drawing.Font("Segoe UI", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.CreateProjectBtn.Location = new System.Drawing.Point(13, 220);
            this.CreateProjectBtn.Name = "CreateProjectBtn";
            this.CreateProjectBtn.Size = new System.Drawing.Size(128, 47);
            this.CreateProjectBtn.TabIndex = 19;
            this.CreateProjectBtn.Text = "Create";
            this.CreateProjectBtn.UseVisualStyleBackColor = true;
            this.CreateProjectBtn.Click += new System.EventHandler(this.CreateProjectBtn_Click);
            // 
            // SettingsAlertLbl
            // 
            this.SettingsAlertLbl.AutoSize = true;
            this.SettingsAlertLbl.Font = new System.Drawing.Font("Segoe UI", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.SettingsAlertLbl.ForeColor = System.Drawing.Color.Crimson;
            this.SettingsAlertLbl.Location = new System.Drawing.Point(63, 12);
            this.SettingsAlertLbl.Name = "SettingsAlertLbl";
            this.SettingsAlertLbl.Size = new System.Drawing.Size(113, 17);
            this.SettingsAlertLbl.TabIndex = 20;
            this.SettingsAlertLbl.Text = "Settings alert msg";
            // 
            // FolderAlertLbl
            // 
            this.FolderAlertLbl.AutoSize = true;
            this.FolderAlertLbl.Font = new System.Drawing.Font("Segoe UI", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.FolderAlertLbl.ForeColor = System.Drawing.Color.Crimson;
            this.FolderAlertLbl.Location = new System.Drawing.Point(12, 175);
            this.FolderAlertLbl.Name = "FolderAlertLbl";
            this.FolderAlertLbl.Size = new System.Drawing.Size(161, 17);
            this.FolderAlertLbl.TabIndex = 21;
            this.FolderAlertLbl.Text = "ProjectCreator folder alert";
            // 
            // NameAlertLbl
            // 
            this.NameAlertLbl.AutoSize = true;
            this.NameAlertLbl.Font = new System.Drawing.Font("Segoe UI", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.NameAlertLbl.ForeColor = System.Drawing.Color.Crimson;
            this.NameAlertLbl.Location = new System.Drawing.Point(504, 175);
            this.NameAlertLbl.Name = "NameAlertLbl";
            this.NameAlertLbl.Size = new System.Drawing.Size(113, 17);
            this.NameAlertLbl.TabIndex = 22;
            this.NameAlertLbl.Text = "Settings alert msg";
            // 
            // NewProject
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(800, 450);
            this.Controls.Add(this.NameAlertLbl);
            this.Controls.Add(this.FolderAlertLbl);
            this.Controls.Add(this.SettingsAlertLbl);
            this.Controls.Add(this.CreateProjectBtn);
            this.Controls.Add(this.NewProjectNameTxt);
            this.Controls.Add(this.label1);
            this.Controls.Add(this.NewProjectFolderTxt);
            this.Controls.Add(this.SettingsBtn);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.BrowseBtn);
            this.Name = "NewProject";
            this.Text = "NewProject";
            this.Load += new System.EventHandler(this.NewProject_Load);
            ((System.ComponentModel.ISupportInitialize)(this.SettingsBtn)).EndInit();
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.TextBox NewProjectFolderTxt;
        private System.Windows.Forms.Button BrowseBtn;
        private System.Windows.Forms.PictureBox SettingsBtn;
        private System.Windows.Forms.TextBox NewProjectNameTxt;
        private System.Windows.Forms.Label label1;
        private System.Windows.Forms.Button CreateProjectBtn;
        private System.Windows.Forms.Label SettingsAlertLbl;
        private System.Windows.Forms.Label FolderAlertLbl;
        private System.Windows.Forms.Label NameAlertLbl;
    }
}