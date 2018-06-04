Imports System.ComponentModel

Public Class MainUI
    Private Sub Button1_Click(sender As Object, e As EventArgs) Handles Button1.Click
        ContextMenuStrip1.Show(System.Windows.Forms.Control.MousePosition)
    End Sub

    Private Sub 控制台ToolStripMenuItem_Click(sender As Object, e As EventArgs) Handles 控制台ToolStripMenuItem.Click
        Console.Show()
    End Sub

    Private Sub 新建下载实例ToolStripMenuItem_Click(sender As Object, e As EventArgs) Handles 新建下载实例ToolStripMenuItem.Click
        NewInstance.ShowDialog()
    End Sub

    Public Sub MenuItem_Click(sender As Object, e As EventArgs)
        SetCurrentYGInstance(FindYGInstance(CType(sender, ToolStripItem).OwnerItem.Text))
    End Sub
    Public Sub MenuItem_Delete(sender As Object, e As EventArgs)
        DeleteYGInstance(CType(sender, ToolStripItem).OwnerItem.Text, CType(sender, ToolStripItem).OwnerItem)
    End Sub

    Private Sub TextBox1_LostFocus(sender As Object, e As EventArgs) Handles TextBox_Name.LostFocus
        If TextBox_Name.Text = CurrentYGIntance.InstanceName Then Exit Sub
        If IsNameUsed(TextBox_Name.Text) Then
            MsgBox("存在一个相同名称的实例", MsgBoxStyle.Information, "名称冲突")
            TextBox_Name.Text = CurrentYGIntance.InstanceName
            TextBox_Name.Focus()
            Exit Sub
        End If
        For Each A As ToolStripItem In ContextMenuStrip1.Items
            If A.Text = CurrentYGIntance.InstanceName Then
                A.Text = TextBox_Name.Text
                Exit For
            End If
        Next
        CurrentYGIntance.InstanceName = TextBox_Name.Text
    End Sub

    Public Sub AddNewInstance(Name As String, CMDLine As String)
        Dim MenuI As ToolStripItem
        Dim YGI As New YouGet
        Dim IA As Integer = ContextMenuStrip1.Items.Count - 2
        If IsNameUsed(Name) = False Then
            YGI.InstanceName = Name
        Else
            MsgBox("名称冲突，取消添加")
            Exit Sub
        End If
        Me.TextBox_Name.Text = YGI.InstanceName
        Dim p As New Process()
        Dim PSI As New ProcessStartInfo("you-get.exe")
        PSI.RedirectStandardInput = True
        PSI.RedirectStandardOutput = True
        PSI.UseShellExecute = False
        PSI.CreateNoWindow = True
        p.StartInfo = PSI
        YGI.ProcessInstance = p
        YGI.URL = NewInstance.TextBox1.Text
        Me.TextBox_URL.Text = YGI.URL
        YGI.ProcessInstance.StartInfo.Arguments = CMDLine
        YGI.CMDLine = CMDLine
        If NewInstance.TextBox3.Text <> "" Then
            Me.TextBox_Path.Text = NewInstance.TextBox3.Text
            YGI.DownloadPath = NewInstance.TextBox3.Text
        Else
            YGI.DownloadPath = Environment.CurrentDirectory
            Me.TextBox_Path.Text = Environment.CurrentDirectory
        End If
        YGI.Size = NewInstance.TextBox2.Text
        Me.TextBox_Size.Text = YGI.Size
        YGI.Timer = New TimerForYouGet(YGI)
        'YGI.Timer.UI = Me
        YouGetInstances.Add(YGI)
        SetCurrentYGInstance(YGI)
        MenuI = ContextMenuStrip1.Items.Add(YGI.InstanceName)
        AddHandler CType(MenuI, ToolStripDropDownItem).DropDownItems.Add("设为当前").Click, AddressOf MenuItem_Click
        AddHandler CType(MenuI, ToolStripDropDownItem).DropDownItems.Add("删除实例").Click, AddressOf MenuItem_Delete
    End Sub

    Private Sub Button2_Click(sender As Object, e As EventArgs) Handles Button2.Click
        'MsgBox(CurrentYGIntance.ProcessInstance.StartInfo.Arguments)
        CurrentYGIntance.ProcessInstance.Start()
        CurrentYGIntance.IsRunning = True
        CurrentYGIntance.Timer.BeginCheck(Me)
    End Sub

    Private Sub Button3_Click(sender As Object, e As EventArgs) Handles Button3.Click
        On Error Resume Next
        CurrentYGIntance.Timer.StopCheck()
        CurrentYGIntance.ProcessInstance.Kill()
        CurrentYGIntance.IsRunning = False
    End Sub


    Private Sub MainUI_Closed(sender As Object, e As EventArgs) Handles Me.Closed
        If My.Computer.FileSystem.FileExists(Environment.CurrentDirectory & "\Info.ini") Then My.Computer.FileSystem.DeleteFile(Environment.CurrentDirectory & "\Info.ini")
        WriteINI("Main", "MaxRec", YouGetInstances.Count, Environment.CurrentDirectory & "\Info.ini")
        For i = 0 To YouGetInstances.Count - 1
            WriteINI("YGI" & i, "Name", CType(YouGetInstances(i), YouGet).InstanceName, Environment.CurrentDirectory & "\Info.ini")
            WriteINI("YGI" & i, "DownloadPath", CType(YouGetInstances(i), YouGet).DownloadPath, Environment.CurrentDirectory & "\Info.ini")
            WriteINI("YGI" & i, "Size", CType(YouGetInstances(i), YouGet).Size, Environment.CurrentDirectory & "\Info.ini")
            WriteINI("YGI" & i, "URL", CType(YouGetInstances(i), YouGet).URL, Environment.CurrentDirectory & "\Info.ini")
            WriteINI("YGI" & i, "CMDLine", CType(YouGetInstances(i), YouGet).CMDLine, Environment.CurrentDirectory & "\Info.ini")
        Next
    End Sub

    Private Sub MainUI_Closing(sender As Object, e As CancelEventArgs) Handles Me.Closing
        Dim DT As String = ""
        For Each YGI As YouGet In YouGetInstances
            If YGI.IsRunning = True Then
                DT = DT & vbCrLf & YGI.InstanceName
            End If
        Next
        If DT <> "" Then
            If MsgBox("有下载正在进行，仍然关闭？" & vbCrLf & "正在进行的下载：" & vbCrLf & DT, MsgBoxStyle.YesNo, "有下载正在进行") <> vbYes Then e.Cancel = True
        End If
    End Sub
End Class
