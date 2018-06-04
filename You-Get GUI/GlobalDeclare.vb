Imports System.Security.Permissions

Module GlobalDeclare
    '读ini API函数
    Private Declare Function GetPrivateProfileString Lib "kernel32" Alias "GetPrivateProfileStringA" (ByVal lpApplicationName As String, ByVal lpKeyName As String, ByVal lpDefault As String, ByVal lpReturnedString As String, ByVal nSize As Int32, ByVal lpFileName As String) As Int32
    '写ini API函数
    Private Declare Function WritePrivateProfileString Lib "kernel32" Alias "WritePrivateProfileStringA" (ByVal lpApplicationName As String, ByVal lpKeyName As String, ByVal lpString As String, ByVal lpFileName As String) As Int32
    Declare Function GetLastError Lib "kernel32" Alias "GetLastError" () As Integer
    Public Class YouGet
        Public InstanceName As String
        Public ProcessInstance As Process
        Public Progress As Integer
        Public Usage As Int16
        Public IsRunning As Boolean
        Public URL As String
        Public IsCurrent As Boolean
        Public DownloadPath As String
        Public Size As String
        Public Timer As TimerForYouGet
        Public CMDLine As String
    End Class
    Public YouGetInstances As New ArrayList
    Public CurrentYGIntance As YouGet
    Public Function GetINI(ByVal Section As String, ByVal AppName As String, ByVal lpDefault As String, ByVal FileName As String) As String
        Dim Str As String = LSet(Str, 256)
        GetPrivateProfileString(Section, AppName, lpDefault, Str, Len(Str), FileName)
        Return Microsoft.VisualBasic.Left(Str, InStr(Str, Chr(0)) - 1)
    End Function
    '写ini文件操作
    Public Function WriteINI(ByVal Section As String, ByVal AppName As String, ByVal lpDefault As String, ByVal FileName As String) As Long
        WriteINI = WritePrivateProfileString(Section, AppName, lpDefault, FileName)
    End Function
    Public Class TimerForYouGet
        Public YGIntance As YouGet
        Dim TimerRef As Timers.Timer
        Dim ReadyToFinish As Boolean = False
        Dim P As Single
        Dim SpeedToShow As String
        Dim Messages As String
        Public UI As MainUI
        Delegate Sub My_Delegate()
        Public Sub New(ByRef YGI)
            YGIntance = YGI
            TimerRef = New Timers.Timer
            AddHandler TimerRef.Elapsed, AddressOf Notify
            TimerRef.Interval = 1000
        End Sub

        Public Sub BeginCheck(U As MainUI)
            On Error Resume Next
            UI = U
            AddHandler YGIntance.ProcessInstance.OutputDataReceived, AddressOf NotifyHandler
            AddHandler YGIntance.ProcessInstance.Exited, AddressOf ExitEvent
            'TimerRef.Start()
            YGIntance.ProcessInstance.BeginOutputReadLine()
        End Sub
        Public Sub StopCheck()
            On Error Resume Next
            'TimerRef.Stop()
            YGIntance.ProcessInstance.CancelOutputRead()
            RemoveHandler YGIntance.ProcessInstance.OutputDataReceived, AddressOf NotifyHandler
            RemoveHandler YGIntance.ProcessInstance.Exited, AddressOf ExitEvent
        End Sub
        Public Sub NotifyHandler(sendingProcess As Object, outLine As DataReceivedEventArgs)
            On Error GoTo Err
            Dim Data As String
            Data = outLine.Data
            If Data.Contains("you-get:") Then
                YGIntance.IsRunning = False
                MsgBox("You-Get 汇报了一个错误!", MsgBoxStyle.Exclamation)
                MsgBox(Data)
                Exit Sub
            End If
            If Data.Contains("merge") Then
                SpeedToShow = "正在融合"
                GoTo Err
            End If
            Messages = Messages & vbCrLf & Data
            Messages = Right(Messages, 1024)
            P = Convert.ToSingle(Split(Trim(Data), "%")(0))
            YGIntance.Progress = P
            SpeedToShow = Split(Data, "]")(1)
Err:
            If YGIntance.IsCurrent Then
                Dim handler As New My_Delegate(AddressOf Invoke)
                UI.Invoke(handler)
            End If
        End Sub
        Public Sub Notify()
            If YGIntance.ProcessInstance.HasExited And ReadyToFinish Then
                StopCheck()
                MsgBox("实例：" & YGIntance.InstanceName & "任务已完成")
            End If
            If YGIntance.IsCurrent Then
                If ReadyToFinish Then
                    MainUI.TextBox_Speed.Text = "正在链接视频"
                    MainUI.TextBox_Progress.Text = "100%"
                Else
                End If
            End If
        End Sub
        Public Sub ExitEvent(ByVal sender As Object,
            ByVal e As System.EventArgs)
            If YGIntance.IsRunning Then
                StopCheck()
                MsgBox("实例：" & YGIntance.InstanceName & "任务已完成")
            End If
            YGIntance.IsRunning = False
        End Sub
        Public Sub Invoke()
            UI.TextBox_Progress.Text = P & "%"
            UI.ProgressBar1.Value = CInt(P)
            UI.TextBox_Speed.Text = SpeedToShow
            Console.TextBox_View.Text = Messages
        End Sub
    End Class



    Public Function IsNameUsed(Name As String) As Boolean
        For Each YGI In YouGetInstances
            If CType(YGI, YouGet).InstanceName = Name Then Return True
        Next
        Return False
    End Function

    Public Sub SetCurrentYGInstance(ByRef YGI As YouGet)
        If CurrentYGIntance IsNot Nothing Then
            CurrentYGIntance.IsCurrent = False
        End If
        CurrentYGIntance = YGI
        MainUI.TextBox_Name.Text = YGI.InstanceName
        MainUI.TextBox_URL.Text = YGI.URL
        MainUI.TextBox_Size.Text = YGI.Size
        MainUI.TextBox_Path.Text = YGI.DownloadPath
        MainUI.ProgressBar1.Value = YGI.Progress
        MainUI.TextBox_Speed.Text = ""
        MainUI.TextBox_Progress.Text = ""
        YGI.IsCurrent = True
    End Sub

    Public Sub DeleteYGInstance(Name As String, Menus As ToolStripItem)
        Dim YG As YouGet = FindYGInstance(Name)
        If YG.IsCurrent Then
            MsgBox("此实例已被设置为当前，请切换到其他实例，然后删除此实例")
            Exit Sub
        End If
        If YG.IsRunning Then
            If (MsgBox("警告：此实例当前正在运行，如果删除此实例，你将无法通过GUI重新开始下载，如果只想暂停，点击停止下载即可" & vbCrLf & "无论如何也要删除这个实例吗？", MsgBoxStyle.YesNo) = MsgBoxResult.Yes) Then
                YG.ProcessInstance.Kill()
            Else
                Exit Sub
            End If
        End If
        YouGetInstances.Remove(YG)
        Menus.Dispose()
    End Sub

    Public Function FindYGInstance(Name As String) As YouGet
        For i = 0 To YouGetInstances.Count - 1
            If CType(YouGetInstances(i), YouGet).InstanceName = Name Then
                Return YouGetInstances(i)
            End If
        Next
        Return Nothing
    End Function

    Sub Main()
        Dim YGI As YouGet
        Dim p As Process
        Dim PSI As ProcessStartInfo
        Dim MenuI As ToolStripItem
        If (My.Computer.FileSystem.FileExists(Environment.CurrentDirectory & "\Info.ini")) Then
            Dim MaxRec As Integer = GetINI("Main", "MaxRec", "0", Environment.CurrentDirectory & "\Info.ini")
            If MaxRec > 0 Then
                For i = 0 To MaxRec - 1
                    YGI = New YouGet
                    Dim Data As String
                    Data = GetINI("YGI" & i, "Name", "Name", Environment.CurrentDirectory & "\Info.ini")
                    While (IsNameUsed(Data))
                        Data = "_" & Data
                    End While
                    YGI.InstanceName = Data
                    YGI.DownloadPath = GetINI("YGI" & i, "DownloadPath", "", Environment.CurrentDirectory & "\Info.ini")
                    YGI.Size = GetINI("YGI" & i, "Size", "NULL", Environment.CurrentDirectory & "\Info.ini")
                    YGI.URL = GetINI("YGI" & i, "URL", "地址无效，下载将无法启动", Environment.CurrentDirectory & "\Info.ini")
                    YGI.CMDLine = GetINI("YGI" & i, "CMDLine", "", Environment.CurrentDirectory & "\Info.ini")
                    p = New Process()
                    PSI = New ProcessStartInfo("you-get.exe")
                    PSI.RedirectStandardInput = True
                    PSI.RedirectStandardOutput = True
                    PSI.UseShellExecute = False
                    PSI.CreateNoWindow = True
                    p.StartInfo = PSI
                    p.StartInfo.Arguments = YGI.CMDLine
                    YGI.ProcessInstance = p
                    YGI.Timer = New TimerForYouGet(YGI)
                    YouGetInstances.Add(YGI)
                    MenuI = MainUI.ContextMenuStrip1.Items.Add(YGI.InstanceName)
                    AddHandler CType(MenuI, ToolStripDropDownItem).DropDownItems.Add("设为当前").Click, AddressOf MainUI.MenuItem_Click
                    AddHandler CType(MenuI, ToolStripDropDownItem).DropDownItems.Add("删除实例").Click, AddressOf MainUI.MenuItem_Delete
                Next
                SetCurrentYGInstance(YGI)
            Else
                YGI = New YouGet
                YGI.InstanceName = "空实例"
                YGI.Timer = New TimerForYouGet(YGI)
                p = New Process()
                PSI = New ProcessStartInfo("you-get.exe")
                PSI.RedirectStandardInput = True
                PSI.RedirectStandardOutput = True
                PSI.UseShellExecute = False
                PSI.CreateNoWindow = True
                p.StartInfo = PSI
                YGI.ProcessInstance = p
                YouGetInstances.Add(YGI)
                SetCurrentYGInstance(YGI)
                MenuI = MainUI.ContextMenuStrip1.Items.Add(YGI.InstanceName)
                AddHandler CType(MenuI, ToolStripDropDownItem).DropDownItems.Add("设为当前").Click, AddressOf MainUI.MenuItem_Click
                AddHandler CType(MenuI, ToolStripDropDownItem).DropDownItems.Add("删除实例").Click, AddressOf MainUI.MenuItem_Delete
            End If
        Else
            YGI = New YouGet
            YGI.InstanceName = "空实例"
            YGI.Timer = New TimerForYouGet(YGI)
            p = New Process()
            PSI = New ProcessStartInfo("you-get.exe")
            PSI.RedirectStandardInput = True
            PSI.RedirectStandardOutput = True
            PSI.UseShellExecute = False
            PSI.CreateNoWindow = True
            p.StartInfo = PSI
            YGI.ProcessInstance = p
            YouGetInstances.Add(YGI)
            SetCurrentYGInstance(YGI)
            MenuI = MainUI.ContextMenuStrip1.Items.Add(YGI.InstanceName)
            AddHandler CType(MenuI, ToolStripDropDownItem).DropDownItems.Add("设为当前").Click, AddressOf MainUI.MenuItem_Click
            AddHandler CType(MenuI, ToolStripDropDownItem).DropDownItems.Add("删除实例").Click, AddressOf MainUI.MenuItem_Delete
        End If
        Application.EnableVisualStyles()
        MainUI.ShowDialog()
    End Sub
End Module