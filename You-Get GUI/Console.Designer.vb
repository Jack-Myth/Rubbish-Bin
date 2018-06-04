<Global.Microsoft.VisualBasic.CompilerServices.DesignerGenerated()> _
Partial Class Console
    Inherits System.Windows.Forms.Form

    'Form 重写 Dispose，以清理组件列表。
    <System.Diagnostics.DebuggerNonUserCode()> _
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
    <System.Diagnostics.DebuggerStepThrough()> _
    Private Sub InitializeComponent()
        Me.TextBox_View = New System.Windows.Forms.TextBox()
        Me.TextBox2 = New System.Windows.Forms.TextBox()
        Me.TextBox_CMDLine = New System.Windows.Forms.TextBox()
        Me.Button1 = New System.Windows.Forms.Button()
        Me.SuspendLayout()
        '
        'TextBox_View
        '
        Me.TextBox_View.Anchor = CType((((System.Windows.Forms.AnchorStyles.Top Or System.Windows.Forms.AnchorStyles.Bottom) _
            Or System.Windows.Forms.AnchorStyles.Left) _
            Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.TextBox_View.BackColor = System.Drawing.Color.Black
        Me.TextBox_View.ForeColor = System.Drawing.Color.White
        Me.TextBox_View.Location = New System.Drawing.Point(12, 12)
        Me.TextBox_View.Multiline = True
        Me.TextBox_View.Name = "TextBox_View"
        Me.TextBox_View.ReadOnly = True
        Me.TextBox_View.Size = New System.Drawing.Size(520, 339)
        Me.TextBox_View.TabIndex = 0
        '
        'TextBox2
        '
        Me.TextBox2.BackColor = System.Drawing.Color.Black
        Me.TextBox2.BorderStyle = System.Windows.Forms.BorderStyle.None
        Me.TextBox2.ForeColor = System.Drawing.Color.White
        Me.TextBox2.Location = New System.Drawing.Point(12, 360)
        Me.TextBox2.Name = "TextBox2"
        Me.TextBox2.Size = New System.Drawing.Size(64, 14)
        Me.TextBox2.TabIndex = 1
        Me.TextBox2.Text = "You-Get"
        Me.TextBox2.TextAlign = System.Windows.Forms.HorizontalAlignment.Center
        '
        'TextBox_CMDLine
        '
        Me.TextBox_CMDLine.Anchor = CType(((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Left) _
            Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.TextBox_CMDLine.BackColor = System.Drawing.Color.Black
        Me.TextBox_CMDLine.ForeColor = System.Drawing.Color.White
        Me.TextBox_CMDLine.Location = New System.Drawing.Point(82, 357)
        Me.TextBox_CMDLine.Name = "TextBox_CMDLine"
        Me.TextBox_CMDLine.Size = New System.Drawing.Size(403, 21)
        Me.TextBox_CMDLine.TabIndex = 2
        '
        'Button1
        '
        Me.Button1.Anchor = CType((System.Windows.Forms.AnchorStyles.Bottom Or System.Windows.Forms.AnchorStyles.Right), System.Windows.Forms.AnchorStyles)
        Me.Button1.Location = New System.Drawing.Point(491, 357)
        Me.Button1.Name = "Button1"
        Me.Button1.Size = New System.Drawing.Size(41, 23)
        Me.Button1.TabIndex = 3
        Me.Button1.Text = "Run"
        Me.Button1.UseVisualStyleBackColor = True
        '
        'Console
        '
        Me.AutoScaleDimensions = New System.Drawing.SizeF(6.0!, 12.0!)
        Me.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font
        Me.BackColor = System.Drawing.Color.Black
        Me.ClientSize = New System.Drawing.Size(542, 386)
        Me.Controls.Add(Me.Button1)
        Me.Controls.Add(Me.TextBox_CMDLine)
        Me.Controls.Add(Me.TextBox2)
        Me.Controls.Add(Me.TextBox_View)
        Me.Name = "Console"
        Me.Text = "Console"
        Me.ResumeLayout(False)
        Me.PerformLayout()

    End Sub

    Friend WithEvents TextBox_View As TextBox
    Friend WithEvents TextBox2 As TextBox
    Friend WithEvents TextBox_CMDLine As TextBox
    Friend WithEvents Button1 As Button
End Class
