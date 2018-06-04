Public Class NewInstance
    'Implements System.Text.RegularExpressions
    Dim InstanceName As String
    Dim SizeSet As New ArrayList
    Dim CMDSet As New ArrayList
    Dim CanOpen As Boolean
    Private Sub Button2_Click(sender As Object, e As EventArgs) Handles Button2.Click
        Dim CMDLine As String = ""
        If (Me.TextBox3.Text <> "") Then CMDLine = CMDLine & "-o " & Me.TextBox3.Text
        CMDLine = CMDLine & " " & Replace(CMDSet(ComboBox1.SelectedIndex), "[URL]", Me.TextBox1.Text)
        MainUI.AddNewInstance(InstanceName, CMDLine)
        Me.Close()
    End Sub

    Private Sub Button1_Click(sender As Object, e As EventArgs) Handles Button1.Click
        On Error GoTo ErR
        Dim p As New Process()
        Dim PSI As New ProcessStartInfo("you-get.exe")
        PSI.RedirectStandardInput = True
        PSI.RedirectStandardOutput = True
        PSI.UseShellExecute = False
        PSI.CreateNoWindow = True
        p.StartInfo = PSI
        p.StartInfo.Arguments = "-i " & Me.TextBox1.Text
        If p.Start() = False Then Exit Sub
        '******************************************************************************************************
        Dim DataStr As String
        Dim DataStr2 As String
        Dim SelectDef As Int16
        p.WaitForExit()
        DataStr = p.StandardOutput.ReadLine()
        If DataStr.Contains("error") Then
            p.StandardOutput.BaseStream.Seek(0, IO.SeekOrigin.Begin)
            MsgBox("获取数据时发生错误")
            MsgBox(p.StandardOutput.ReadToEnd, MsgBoxStyle.Critical)
            Exit Sub
        End If
        DataStr = p.StandardOutput.ReadLine()
        InstanceName = Trim(Replace(DataStr, "title:", ""))
        p.StandardOutput.ReadLine()
        SizeSet.Clear()
        CMDSet.Clear()
        ComboBox1.Items.Clear()
        While (Not p.StandardOutput.EndOfStream)
            DataStr = p.StandardOutput.ReadLine()
            If DataStr.Contains("[ DEFAULT ] _____") Then
                SelectDef = SizeSet.Count
            ElseIf DataStr.Contains("- itag:") Then
                DataStr = p.StandardOutput.ReadLine()
                DataStr2 = Trim(Replace(DataStr, "container:", ""))
                DataStr = p.StandardOutput.ReadLine()
                ComboBox1.Items.Add(DataStr2 & "     " & Trim(Replace(DataStr, "quality:", "")))
                DataStr = p.StandardOutput.ReadLine()
                If (DataStr.Contains("size:")) Then
                    SizeSet.Add(Trim(Replace(DataStr, "size:", "")))
                Else
                    SizeSet.Add("未知")
                End If
                DataStr = p.StandardOutput.ReadLine()
                CMDSet.Add(Trim(Replace(Replace(DataStr, "# download-with:", ""), "you-get", "")))
            Else
            End If
        End While
        ComboBox1.SelectedIndex = SelectDef
        Exit Sub
Err:
    End Sub

    Private Sub Button3_Click(sender As Object, e As EventArgs) Handles Button3.Click
        FolderBrowserDialog1.SelectedPath = ""
        FolderBrowserDialog1.ShowDialog()
        If (FolderBrowserDialog1.SelectedPath <> "") Then
            Me.TextBox3.Text = FolderBrowserDialog1.SelectedPath
        End If
    End Sub

    Private Sub ComboBox1_SelectedIndexChanged(sender As Object, e As EventArgs) Handles ComboBox1.SelectedIndexChanged
        Me.TextBox2.Text = SizeSet(ComboBox1.SelectedIndex)
        Me.Button2.Enabled = True
    End Sub

    Private Sub NewInstance_Closed(sender As Object, e As EventArgs) Handles Me.Closed
        Me.Dispose()
    End Sub
End Class