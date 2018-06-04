<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()>
Partial Class MainUI
    Inherits System.Windows.Forms.Form

    'Form 重写 Dispose，以清理组件列表。
    <System.Diagnostics.DebuggerNonUserCode()>
    Protected Overrides Sub Dispose(ByVal disposing As Boolean)
        Try
            If disposing AndAlso components IsNot Nothing Then
                components.Dispose()
            End If
        Finally
            MyBase.Dispose(disposing)
        End Try
    End Sub

    'Windows 窗体设计器所必需的
    Private components As System.ComponentModel.IContainer

    '注意: 以下过程是 Windows 窗体设计器所必需的
    '可以使用 Windows 窗体设计器修改它。  
    '不要使用代码编辑器修改它。
    <System.Diagnostics.DebuggerStepThrough()>
    Private Sub InitializeComponent()
        Me.components = New System.ComponentModel.Container()
        Me.Button1 = New System.Windows.Forms.Button()
        Me.ContextMenuStrip1 = New System.Windows.Forms.ContextMenuStrip(Me.components)
        Me.控制台ToolStripMenuItem = New System.Windows.Forms.ToolStripMenuItem()
        Me.新建下载实例ToolStripMenuItem = New System.Windows.Forms.ToolStripMenuItem()
        Me.ToolStripMenuItem1 = New System.Windows.Forms.ToolStripSeparator()
        Me.ProgressBar1 = New System.Windows.Forms.ProgressBar()
        Me.TextBox_Name = New System.Windows.Forms.TextBox()
        Me.Label1 = New System.Windows.Forms.Label()
        Me.Label2 = New System.Windows.Forms.Label()
        Me.TextBox_Size = New System.Windows.Forms.TextBox()
        Me.TextBox_Speed = New System.Windows.Forms.TextBox()
        Me.Label3 = New System.Windows.Forms.Label()
        Me.Label4 = New System.Windows.Forms.Label()
        Me.TextBox_Progress = New System.Windows.Forms.TextBox()
        Me.TextBox_URL = New System.Windows.Forms.TextBox()
        Me.Label5 = New System.Windows.Forms.Label()
        Me.Button2 = New System.Windows.Forms.Button()
        Me.Button3 = New System.Windows.Forms.Button()
        Me.TextBox_Path = New System.Windows.Forms.TextBox()
        Me.Label6 = New System.Windows.Forms.Label()
        Me.ContextMenuStrip1.SuspendLayout()
        Me.SuspendLayout()
        '
        'Button1
        '
        Me.Button1.Location = New System.Drawing.Point(464, 12)
        Me.Button1.Name = "Button1"
        Me.Button1.Size = New System.Drawing.Size(62, 23)
        Me.Button1.TabIndex = 1
        Me.Button1.Text = "菜单"
        Me.Button1.UseVisualStyleBackColor = True
        '
        'ContextMenuStrip1
        '
        Me.ContextMenuStrip1.Items.AddRange(New System.Windows.Forms.ToolStripItem() {Me.控制台ToolStripMenuItem, Me.新建下载实例ToolStripMenuItem, Me.ToolStripMenuItem1})
        Me.ContextMenuStrip1.Name = "ContextMenuStrip1"
        Me.ContextMenuStrip1.Size = New System.Drawing.Size(149, 54)
        Me.ContextMenuStrip1.Text = "Menu"
        '
        '控制台ToolStripMenuItem
        '
        Me.控制台ToolStripMenuItem.Name = "控制台ToolStripMenuItem"
        Me.控制台ToolStripMenuItem.Size = New System.Drawing.Size(148, 22)
        Me.控制台ToolStripMenuItem.Text = "控制台"
        '
        '新建下载实例ToolStripMenuItem
        '
        Me.新建下载实例ToolStripMenuItem.Name = "新建下载实例ToolStripMenuItem"
        Me.新建下载实例ToolStripMenuItem.Size = New System.Drawing.Size(148, 22)
        Me.新建下载实例ToolStripMenuItem.Text = "新建下载实例"
        '
        'ToolStripMenuItem1
        '
        Me.ToolStripMenuItem1.Name = "ToolStripMenuItem1"
        Me.ToolStripMenuItem1.Size = New System.Drawing.Size(145, 6)
        '
        'ProgressBar1
        '
        Me.ProgressBar1.Anchor = CType(((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left) _
            Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.ProgressBar1.Location = New System.Drawing.Point(13, 131)
        Me.ProgressBar1.Name = "ProgressBar1"
        Me.ProgressBar1.Size = New System.Drawing.Size(513, 23)
        Me.ProgressBar1.TabIndex = 3
        '
        'TextBox_Name
        '
        Me.TextBox_Name.Location = New System.Drawing.Point(68, 14)
        Me.TextBox_Name.Name = "TextBox_Name"
        Me.TextBox_Name.Size = New System.Drawing.Size(202, 21)
        Me.TextBox_Name.TabIndex = 4
        '
        'Label1
        '
        Me.Label1.AutoSize = True
        Me.Label1.Location = New System.Drawing.Point(9, 17)
        Me.Label1.Name = "Label1"
        Me.Label1.Size = New System.Drawing.Size(53, 12)
        Me.Label1.TabIndex = 5
        Me.Label1.Text = "下载名称"
        '
        'Label2
        '
        Me.Label2.AutoSize = True
        Me.Label2.Location = New System.Drawing.Point(9, 71)
        Me.Label2.Name = "Label2"
        Me.Label2.Size = New System.Drawing.Size(53, 12)
        Me.Label2.TabIndex = 6
        Me.Label2.Text = "文件大小"
        '
        'TextBox_Size
        '
        Me.TextBox_Size.Location = New System.Drawing.Point(68, 68)
        Me.TextBox_Size.Name = "TextBox_Size"
        Me.TextBox_Size.ReadOnly = True
        Me.TextBox_Size.Size = New System.Drawing.Size(187, 21)
        Me.TextBox_Size.TabIndex = 7
        '
        'TextBox_Speed
        '
        Me.TextBox_Speed.Location = New System.Drawing.Point(321, 71)
        Me.TextBox_Speed.Name = "TextBox_Speed"
        Me.TextBox_Speed.ReadOnly = True
        Me.TextBox_Speed.Size = New System.Drawing.Size(73, 21)
        Me.TextBox_Speed.TabIndex = 8
        '
        'Label3
        '
        Me.Label3.AutoSize = True
        Me.Label3.Location = New System.Drawing.Point(261, 74)
        Me.Label3.Name = "Label3"
        Me.Label3.Size = New System.Drawing.Size(53, 12)
        Me.Label3.TabIndex = 9
        Me.Label3.Text = "下载速度"
        '
        'Label4
        '
        Me.Label4.AutoSize = True
        Me.Label4.Location = New System.Drawing.Point(400, 74)
        Me.Label4.Name = "Label4"
        Me.Label4.Size = New System.Drawing.Size(53, 12)
        Me.Label4.TabIndex = 11
        Me.Label4.Text = "下载进度"
        '
        'TextBox_Progress
        '
        Me.TextBox_Progress.Location = New System.Drawing.Point(464, 71)
        Me.TextBox_Progress.Name = "TextBox_Progress"
        Me.TextBox_Progress.ReadOnly = True
        Me.TextBox_Progress.Size = New System.Drawing.Size(59, 21)
        Me.TextBox_Progress.TabIndex = 10
        '
        'TextBox_URL
        '
        Me.TextBox_URL.Location = New System.Drawing.Point(68, 41)
        Me.TextBox_URL.Name = "TextBox_URL"
        Me.TextBox_URL.ReadOnly = True
        Me.TextBox_URL.Size = New System.Drawing.Size(455, 21)
        Me.TextBox_URL.TabIndex = 15
        '
        'Label5
        '
        Me.Label5.AutoSize = True
        Me.Label5.Location = New System.Drawing.Point(9, 44)
        Me.Label5.Name = "Label5"
        Me.Label5.Size = New System.Drawing.Size(53, 12)
        Me.Label5.TabIndex = 14
        Me.Label5.Text = "视频地址"
        '
        'Button2
        '
        Me.Button2.Location = New System.Drawing.Point(276, 12)
        Me.Button2.Name = "Button2"
        Me.Button2.Size = New System.Drawing.Size(75, 23)
        Me.Button2.TabIndex = 16
        Me.Button2.Text = "开始下载"
        Me.Button2.UseVisualStyleBackColor = True
        '
        'Button3
        '
        Me.Button3.Location = New System.Drawing.Point(357, 12)
        Me.Button3.Name = "Button3"
        Me.Button3.Size = New System.Drawing.Size(75, 23)
        Me.Button3.TabIndex = 17
        Me.Button3.Text = "停止下载"
        Me.Button3.UseVisualStyleBackColor = True
        '
        'TextBox_Path
        '
        Me.TextBox_Path.Location = New System.Drawing.Point(68, 98)
        Me.TextBox_Path.Name = "TextBox_Path"
        Me.TextBox_Path.ReadOnly = True
        Me.TextBox_Path.Size = New System.Drawing.Size(455, 21)
        Me.TextBox_Path.TabIndex = 19
        '
        'Label6
        '
        Me.Label6.AutoSize = True
        Me.Label6.Location = New System.Drawing.Point(9, 101)
        Me.Label6.Name = "Label6"
        Me.Label6.Size = New System.Drawing.Size(53, 12)
        Me.Label6.TabIndex = 18
        Me.Label6.Text = "下载路径"
        '
        'MainUI
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 12.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.ClientSize = New System.Drawing.Size(538, 166)
        Me.Controls.Add(Me.TextBox_Path)
        Me.Controls.Add(Me.Label6)
        Me.Controls.Add(Me.Button3)
        Me.Controls.Add(Me.Button2)
        Me.Controls.Add(Me.TextBox_URL)
        Me.Controls.Add(Me.Label5)
        Me.Controls.Add(Me.Label4)
        Me.Controls.Add(Me.TextBox_Progress)
        Me.Controls.Add(Me.Label3)
        Me.Controls.Add(Me.TextBox_Speed)
        Me.Controls.Add(Me.TextBox_Size)
        Me.Controls.Add(Me.Label2)
        Me.Controls.Add(Me.Label1)
        Me.Controls.Add(Me.TextBox_Name)
        Me.Controls.Add(Me.ProgressBar1)
        Me.Controls.Add(Me.Button1)
        Me.FormBorderStyle = System.Windows.Forms.FormBorderStyle.Fixed3D
        Me.MaximizeBox = False
        Me.Name = "MainUI"
        Me.Text = "You-Get GUI"
        Me.ContextMenuStrip1.ResumeLayout(False)
        Me.ResumeLayout(False)
        Me.PerformLayout()

    End Sub
    Friend WithEvents Button1 As Button
    Friend WithEvents ContextMenuStrip1 As ContextMenuStrip
    Friend WithEvents 控制台ToolStripMenuItem As ToolStripMenuItem
    Friend WithEvents 新建下载实例ToolStripMenuItem As ToolStripMenuItem
    Friend WithEvents ToolStripMenuItem1 As ToolStripSeparator
    Friend WithEvents ProgressBar1 As ProgressBar
    Friend WithEvents TextBox_Name As TextBox
    Friend WithEvents Label1 As Label
    Friend WithEvents Label2 As Label
    Friend WithEvents TextBox_Size As TextBox
    Friend WithEvents TextBox_Speed As TextBox
    Friend WithEvents Label3 As Label
    Friend WithEvents Label4 As Label
    Friend WithEvents TextBox_Progress As TextBox
    Friend WithEvents TextBox_URL As TextBox
    Friend WithEvents Label5 As Label
    Friend WithEvents Button2 As Button
    Friend WithEvents Button3 As Button
    Friend WithEvents TextBox_Path As TextBox
    Friend WithEvents Label6 As Label
End Class
