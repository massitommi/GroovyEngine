namespace ProjectCreator.src.forms
{
    partial class SettingsPage
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
            this.SetEngineLocationBtn = new System.Windows.Forms.Button();
            this.EngineLocationTxt = new System.Windows.Forms.TextBox();
            this.label2 = new System.Windows.Forms.Label();
            this.BrowseBtn = new System.Windows.Forms.Button();
            this.SuspendLayout();
            // 
            // SetEngineLocationBtn
            // 
            this.SetEngineLocationBtn.Font = new System.Drawing.Font("Segoe UI", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.SetEngineLocationBtn.Location = new System.Drawing.Point(72, 164);
            this.SetEngineLocationBtn.Name = "SetEngineLocationBtn";
            this.SetEngineLocationBtn.Size = new System.Drawing.Size(128, 47);
            this.SetEngineLocationBtn.TabIndex = 25;
            this.SetEngineLocationBtn.Text = "Create";
            this.SetEngineLocationBtn.UseVisualStyleBackColor = true;
            this.SetEngineLocationBtn.Click += new System.EventHandler(this.SetEngineLocationBtn_Click);
            // 
            // EngineLocationTxt
            // 
            this.EngineLocationTxt.Font = new System.Drawing.Font("Segoe UI", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.EngineLocationTxt.Location = new System.Drawing.Point(72, 114);
            this.EngineLocationTxt.Name = "EngineLocationTxt";
            this.EngineLocationTxt.Size = new System.Drawing.Size(411, 25);
            this.EngineLocationTxt.TabIndex = 22;
            this.EngineLocationTxt.Text = "C:/Stuff/";
            // 
            // label2
            // 
            this.label2.AutoSize = true;
            this.label2.Font = new System.Drawing.Font("Segoe UI", 12F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.label2.Location = new System.Drawing.Point(68, 90);
            this.label2.Name = "label2";
            this.label2.Size = new System.Drawing.Size(116, 21);
            this.label2.TabIndex = 23;
            this.label2.Text = "Engine location";
            // 
            // BrowseBtn
            // 
            this.BrowseBtn.Font = new System.Drawing.Font("Segoe UI", 9.75F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
            this.BrowseBtn.Location = new System.Drawing.Point(489, 114);
            this.BrowseBtn.Name = "BrowseBtn";
            this.BrowseBtn.Size = new System.Drawing.Size(36, 25);
            this.BrowseBtn.TabIndex = 24;
            this.BrowseBtn.Text = "...";
            this.BrowseBtn.UseVisualStyleBackColor = true;
            this.BrowseBtn.Click += new System.EventHandler(this.BrowseBtn_Click);
            // 
            // SettingsPage
            // 
            this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
            this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
            this.ClientSize = new System.Drawing.Size(592, 324);
            this.Controls.Add(this.SetEngineLocationBtn);
            this.Controls.Add(this.EngineLocationTxt);
            this.Controls.Add(this.label2);
            this.Controls.Add(this.BrowseBtn);
            this.Name = "SettingsPage";
            this.Text = "Settings";
            this.Load += new System.EventHandler(this.Settings_Load);
            this.ResumeLayout(false);
            this.PerformLayout();

        }

        #endregion

        private System.Windows.Forms.Button SetEngineLocationBtn;
        private System.Windows.Forms.TextBox EngineLocationTxt;
        private System.Windows.Forms.Label label2;
        private System.Windows.Forms.Button BrowseBtn;
    }
}