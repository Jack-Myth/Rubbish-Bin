Imports System.IO
Imports System.Security.AccessControl
Public Class Form1
    Private Declare Function ShellExecute Lib "shell32.dll" Alias "ShellExecuteA" (ByVal hwnd As Integer, ByVal lpOperation As String, ByVal lpFile As String, ByVal lpParameters As String, ByVal lpDirectory As String, ByVal nShowCmd As Integer) As Integer
    Private Declare Function URLDownloadToFile Lib "urlmon.dll" Alias "URLDownloadToFileA" (ByVal pCaller As Integer, ByVal szURL As String, ByVal szFileName As String, ByVal dwReserved As Integer, ByVal lpfnCB As Integer) As Integer
    Private Declare Function SHFileOperation Lib "shell32.dll" Alias "SHFileOperationA" (ByRef lpFileOp As SHFILEOPSTRUCT) As Integer
    Private Const FO_MOVE = &H1
    Private Const FO_COPY = &H2
    Private Const FO_DELETE = &H3
    Private Const FO_RENAME = &H4
    Private Const FOF_NOCONFIRMATION = &H10
    Private Const FOF_NOCONFIRMMKDIR = &H200
    Private Const FOF_ALLOWUNDO = &H40
    Private Const FOF_MULTIDESTFILES = &H1
    Dim DownloadPath As String()
    Dim HostsCreator As String()
    Dim WebPage As String()
    Public DeleteItems As String()
    Structure SHFILEOPSTRUCT
        Dim hWnd As IntPtr
        Dim wFunc As Integer
        Dim pFrom As String
        Dim pTo As String
        Dim fFlags As Int32
        Dim fAborted As Integer
        Dim hNameMaps As IntPtr
        Dim sProgress As Integer
    End Structure
    Private Sub LinkLabel1_LinkClicked(sender As Object, e As LinkLabelLinkClickedEventArgs) Handles LinkLabel1.LinkClicked
        ShellExecute(Me.Handle, "Open", WebPage(Me.ComboBox1.SelectedIndex), 0, 0, 0)
        Me.LinkLabel1.LinkVisited = True
    End Sub

    Private Sub Button1_Click(sender As Object, e As EventArgs) Handles Button1.Click
        Me.Button1.Text = "正在搭梯子..."
        Me.Button1.Enabled = False
        Me.Refresh()
        If ComboBox1.SelectedItem.ToString.Contains("LaoD") Then
            ApplyHostsForLaoD()
            Return
        End If
        'addpathPower("C:\Windows\System32\drivers\etc", "Administrators", "FullControl")
        'If FileIO.FileSystem.FileExists("C:\Windows\System32\drivers\etc\hosts") Then addpathPower("C:\Windows\System32\drivers\etc\hosts", "Administrators", "FullControl")
        If FileIO.FileSystem.FileExists("C:\Windows\System32\drivers\etc\hosts") Then MainModule.addpathPower("C:\Windows\System32\drivers\etc\hosts", "Administrators", "FullControl")
        If FileIO.FileSystem.FileExists("hosts") Then FileIO.FileSystem.DeleteFile("hosts")
        Dim RunReg As Microsoft.Win32.RegistryKey = My.Computer.Registry.CurrentUser.CreateSubKey("Software\Microsoft\Windows\CurrentVersion\Run")
        Select Case Me.ComboBox2.SelectedIndex
            Case 0
                RunReg.DeleteValue("OneKey_Hosts", False)
            Case 1
                RunReg.SetValue("OneKey_Hosts", Application.ExecutablePath)
            Case 2
                Dim CmdPath As String = Application.ExecutablePath + " -slientApply"
                If Me.CheckBox1.Checked Then CmdPath = CmdPath + " -clear"
                CmdPath = CmdPath + " -dp " + DownloadPath(Me.ComboBox1.SelectedIndex)
                RunReg.SetValue("OneKey_Hosts", CmdPath)
        End Select
        If FileIO.FileSystem.FileExists("C:\Windows\System32\drivers\etc\hosts") And Me.CheckBox1.Checked Then
            Try
                FileIO.FileSystem.DeleteFile("C:\Windows\System32\drivers\etc\hosts")
                System.Threading.Thread.Sleep(1000)
            Catch ex As Exception
                MsgBox("Error:" & ex.Message, MsgBoxStyle.Critical)
                MsgBox("尝试删除原有Hosts失败，终止操作！")
                Return
            End Try
        End If
        URLDownloadToFile(0, DownloadPath(Me.ComboBox1.SelectedIndex), "hosts", 0, 0)
        If FileIO.FileSystem.FileExists("hosts") = False Then
            MsgBox("hosts下载失败，终止！", MsgBoxStyle.Critical)
            If Me.CheckBox1.Checked Then
                MsgBox("请检查网络连接", MsgBoxStyle.Information)
            Else
                MsgBox("请检查网络连接，或勾选完全重新链接再试一次", MsgBoxStyle.Information)
            End If
            Me.Button1.Enabled = True
            Me.Button1.Text = "一键科学上网！"
            Exit Sub
        End If
        '删除Hosts内的指定条目
        If Me.CheckBox2.Checked Then DeleteItemsFunction()

        Try
            If FileIO.FileSystem.FileExists("C:\Windows\System32\drivers\etc\hosts.bak") = False Then
                FileIO.FileSystem.CopyFile("C:\Windows\System32\drivers\etc\hosts", "C:\Windows\System32\drivers\etc\hosts.bak")
                MsgBox("hosts文件已备份，位于C:\Windows\System32\drivers\etc\hosts.bak，如有必要可以使用此文件还原", MsgBoxStyle.Information)
            End If
        Catch
            MsgBox("hosts备份失败", MsgBoxStyle.Exclamation)
        End Try
        Try
            FileIO.FileSystem.CopyFile("hosts", "C:\Windows\System32\drivers\etc\hosts", True)
        Catch ex As Exception
            Dim SHFileOp As New SHFILEOPSTRUCT
            SHFileOp.wFunc = FO_COPY
            SHFileOp.pFrom = "hosts" & vbNullChar & vbNullChar
            SHFileOp.pTo = "C:\Windows\System32\drivers\etc\hosts" & vbNullChar & vbNullChar
            If (SHFileOperation(SHFileOp) = 0) Then Exit Try
            MsgBox(ex.Message, MsgBoxStyle.Critical)
            MsgBox("hosts应用失败，终止！", MsgBoxStyle.Critical)
            MsgBox("请尝试手动将目录下的hosts文件复制到系统相应位置", MsgBoxStyle.Information)
            Me.Button1.Enabled = True
            Me.Button1.Text = "一键科学上网！"
            Exit Sub
        End Try
        MsgBox("科学上网配置完成！", MsgBoxStyle.Information)
        Me.Button1.Text = "一键科学上网！"
        Me.Button1.Enabled = True
        Try
            FileIO.FileSystem.DeleteFile("hosts")
        Catch
        End Try
    End Sub

    Private Sub Form1_Load(sender As Object, e As EventArgs) Handles MyBase.Load
        'MsgBox(New Security.Principal.WindowsPrincipal(Security.Principal.WindowsIdentity.GetCurrent).IsInRole(New Security.Principal.SecurityIdentifier(Security.Principal.WellKnownSidType.BuiltinAdministratorsSid, Nothing)))
        Dim ItemNumber As Integer = 4
        ReDim HostsCreator(ItemNumber)
        ReDim WebPage(ItemNumber)
        ReDim DownloadPath(ItemNumber)
        Me.ComboBox1.Items.Add("hosts From LaoD[测试][推荐]")
        WebPage(0) = "https://laod.cn/hosts/2017-google-hosts.html"
        HostsCreator(0) = "LoadD发布于load.cn"
        DownloadPath(0) = ""
        Me.ComboBox1.Items.Add("hosts From GoogleHost")
        WebPage(1) = "https://github.com/googlehosts/hosts"
        HostsCreator(1) = "GoogleHosts发布于Github"
        DownloadPath(1) = "https://github.com/googlehosts/hosts/raw/master/hosts-files/hosts"
        Me.ComboBox1.Items.Add("hosts-pc From wangchunming")
        WebPage(2) = "https://github.com/wangchunming/2017hosts"
        HostsCreator(2) = "wangchunming发布于Github"
        DownloadPath(2) = "https://github.com/wangchunming/2017hosts/raw/master/hosts-pc"
        Me.ComboBox1.Items.Add("hosts From racaljk[已停止维护]")
        WebPage(3) = "https://github.com/racaljk/hosts"
        HostsCreator(3) = "racaljk发布于Github"
        DownloadPath(3) = "https://github.com/racaljk/hosts/raw/master/hosts"
        '~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        '~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
        ComboBox1.SelectedIndex = 0
        Dim RunReg As Microsoft.Win32.RegistryKey = My.Computer.Registry.CurrentUser.CreateSubKey("Software\Microsoft\Windows\CurrentVersion\Run")
        If RunReg.GetValue("OneKey_Hosts") IsNot Nothing Then
            If RunReg.GetValue("OneKey_Hosts").ToString().Contains("-slientApply") Then
                ComboBox2.SelectedIndex = 2
                If RunReg.GetValue("OneKey_Hosts").ToString().Contains("-clear") Then Me.CheckBox1.Checked = True
                Dim Str As String = RunReg.GetValue("OneKey_Hosts")
                Str = Mid(Str, Str.IndexOf("-dp") + 4)
                Str = Str.Trim()
                Me.ComboBox1.SelectedIndex = Array.IndexOf(DownloadPath, Str)
            Else
                ComboBox2.SelectedIndex = 1
            End If
        Else
            ComboBox2.SelectedIndex = 0
        End If
        DeleteItems = Profile.ReadOneString("Setting", "DeleteItems", "OneKeyHosts.Setting").Split(",")
    End Sub

    Private Sub ComboBox1_SelectedIndexChanged(sender As Object, e As EventArgs) Handles ComboBox1.SelectedIndexChanged
        Me.LinkLabel1.Text = HostsCreator(Me.ComboBox1.SelectedIndex)
        Me.Label3.Location = New Point(Me.LinkLabel1.Location.X + Me.LinkLabel1.Size.Width + 6, Me.Label3.Location.Y)
    End Sub
    Private Sub Button2_Click(sender As Object, e As EventArgs) Handles Button2.Click
        MsgBox("勾选完全重新链接将会在下载Hosts之前删除原有的Hosts来尝试直连下载网站，多数情况下可以避免原有Hosts对下载的影响，如果遇见长时间无法下载完成的情况，请尝试此选项")
    End Sub
    'Public Sub addpathPower(ByVal pathname As String, ByVal username As String, ByVal power As String)

    '    Dim dirinfo As DirectoryInfo = New DirectoryInfo(pathname)

    '    If (dirinfo.Attributes & FileAttributes.ReadOnly) <> 0 Then
    '        dirinfo.Attributes = FileAttributes.Normal
    '    End If

    '    '取得访问控制列表
    '    Dim dirsecurity As DirectorySecurity = dirinfo.GetAccessControl()

    '    Select Case power
    '        Case "FullControl"
    '            dirsecurity.AddAccessRule(New FileSystemAccessRule(username, FileSystemRights.FullControl, InheritanceFlags.ContainerInherit, PropagationFlags.InheritOnly, AccessControlType.Allow))
    '        Case "ReadOnly"
    '            dirsecurity.AddAccessRule(New FileSystemAccessRule(username, FileSystemRights.Read, AccessControlType.Allow))
    '        Case "Write"
    '            dirsecurity.AddAccessRule(New FileSystemAccessRule(username, FileSystemRights.Write, AccessControlType.Allow))
    '        Case "Modify"
    '            dirsecurity.AddAccessRule(New FileSystemAccessRule(username, FileSystemRights.Modify, AccessControlType.Allow))
    '    End Select
    '    dirinfo.SetAccessControl(dirsecurity)
    'End Sub

    Private Sub DeleteItemsFunction()
        Dim TmPFPath = My.Computer.FileSystem.GetTempFileName()
        Dim AnR = My.Computer.FileSystem.OpenTextFileReader("hosts")
        Dim AnW = My.Computer.FileSystem.OpenTextFileWriter(TmPFPath, False)
        Dim ATmp, ATmp2 As String
        Dim A As Boolean
        Do While (AnR.EndOfStream = False)
            ATmp = AnR.ReadLine()
            ATmp.Trim()
            ATmp2 = LCase(ATmp)
            If ATmp.Length > 0 AndAlso ATmp.Substring(0, 1).Equals("#") Then
                A = False
                Dim AEnd As String
                For Each S As String In DeleteItems
                    If ATmp2.Contains(LCase(S)) AndAlso ATmp2.Contains("start") Then
                        A = True
                        AEnd = S
                        Exit For
                    End If
                Next
                If A Then
                    Do While (AnR.EndOfStream = False)
                        ATmp = AnR.ReadLine()
                        ATmp.Trim()
                        ATmp2 = LCase(ATmp)
                        If ATmp.Length > 0 AndAlso ATmp.Substring(0, 1).Equals("#") Then
                            If ATmp2.Contains(LCase(AEnd)) AndAlso ATmp2.Contains("end") Then Exit Do
                        End If
                    Loop
                Else
                    AnW.WriteLine(ATmp)
                End If
            Else
                AnW.WriteLine(ATmp)
            End If
        Loop
        AnW.Close()
        AnR.Close()
        My.Computer.FileSystem.CopyFile(TmPFPath, "hosts", True)
    End Sub

    Private Sub ApplyHostsForLaoD()
        Me.Button1.Text = "正在分析网页内容..."
        Me.Button1.Refresh()
        Dim WB As New WebBrowser
        Dim TT As New Net.WebClient
        AddHandler WB.DocumentCompleted, AddressOf ApplyHostsFromLaoDState1
        WB.Url = New Uri("https://iiio.io/download/")
    End Sub

    Private Sub ApplyHostsFromLaoDState1(ByVal sender As System.Object, ByVal ei As System.Windows.Forms.WebBrowserDocumentCompletedEventArgs)
        Dim broswer As WebBrowser = CType(sender, WebBrowser)
        RemoveHandler broswer.DocumentCompleted, AddressOf ApplyHostsFromLaoDState1
        Dim HyperLinks = broswer.Document.GetElementsByTagName("A")
        Dim theBiggest As Long = 0
        For i = 0 To HyperLinks.Count - 1
            Dim TmpString As String = HyperLinks(i).InnerText
            TmpString = Microsoft.VisualBasic.Left(TmpString, TmpString.Count - 1)
            If IsNumeric(TmpString) Then
                If theBiggest < CLng(TmpString) Then theBiggest = CLng(TmpString)
            End If
        Next
        Me.Button1.Text = "得到日期为" + CStr(theBiggest) + "的URL,Downloading"
        Me.Button1.Refresh()
        URLDownloadToFile(0, "https://iiio.io/download/" + CStr(theBiggest) + "/windows%E7%94%A8%E6%89%B9%E5%A4%84%E7%90%86.zip", "hostsFromLoad.zip", 0, 0)
        If FileIO.FileSystem.FileExists("hostsFromLoad.zip") = False Then
            MsgBox("hosts下载失败，终止！", MsgBoxStyle.Critical)
            Return
        End If
        Dim TT As New Net.WebClient
        TT.Encoding = System.Text.Encoding.UTF8
        Dim WebPage As String = TT.DownloadString("https://laod.cn/hosts/2017-google-hosts.html")
        Dim Target = WebPage.IndexOf(" 解压密码：")
        Dim Password As String
        If Target < 0 Then
            If MsgBox("没有找到解压密码，手动查找？", MsgBoxStyle.YesNo) = MsgBoxResult.No Then
                MsgBox("应用Hosts失败")
                Return
            End If
            System.Diagnostics.Process.Start("https://laod.cn/hosts/2017-google-hosts.html")
            Password = InputBox("输入你找到的密码")
            If Password = "" Then
                MsgBox("应用Hosts失败")
                Return
            End If
        Else
            Password = WebPage.Substring(Target + " 解压密码：".Length, WebPage.IndexOf("</span>", Target) - Target - " 解压密码：".Length).Trim()
        End If
        My.Computer.FileSystem.WriteAllBytes("7za.exe", My.Resources.RESource._7za, False)
        Shell("7za.exe x hostsFromLoad.zip -p" + Password + " -o" + """" + Application.StartupPath + """" + " *\hosts", AppWinStyle.NormalFocus, True)
        If FileIO.FileSystem.FileExists("windows用批处理\hosts") = False Then
            If MsgBox("自动分析的解压密码好像是错误的:" + Password + "，手动查找？", MsgBoxStyle.YesNo) = MsgBoxResult.No Then
                MsgBox("应用Hosts失败")
                Return
            End If
            System.Diagnostics.Process.Start("https://laod.cn/hosts/2017-google-hosts.html")
            Password = InputBox("输入你找到的密码")
            If Password = "" Then
                MsgBox("应用Hosts失败")
                Return
            End If
            Shell("7za.exe x hostsFromLoad.zip -p" + Password + " -o" + """" + Application.StartupPath + """" + " *\hosts", AppWinStyle.Hide, True)
            If FileIO.FileSystem.FileExists("windows用批处理\hosts") = False Then
                MsgBox("解压错误，应用Hosts失败")
                Return
            End If
        End If
        FileIO.FileSystem.CopyFile("windows用批处理\hosts", "hosts", True)
        FileIO.FileSystem.DeleteDirectory("windows用批处理", FileIO.DeleteDirectoryOption.DeleteAllContents)
        Try
            If FileIO.FileSystem.FileExists("C:\Windows\System32\drivers\etc\hosts.bak") = False Then
                FileIO.FileSystem.CopyFile("C:\Windows\System32\drivers\etc\hosts", "C:\Windows\System32\drivers\etc\hosts.bak")
                MsgBox("hosts文件已备份，位于C:\Windows\System32\drivers\etc\hosts.bak，如有必要可以使用此文件还原", MsgBoxStyle.Information)
            End If
        Catch
            MsgBox("hosts备份失败", MsgBoxStyle.Exclamation)
        End Try
        Try
            FileIO.FileSystem.CopyFile("hosts", "C:\Windows\System32\drivers\etc\hosts", True)
        Catch ex As Exception
            Dim SHFileOp As New SHFILEOPSTRUCT
            SHFileOp.wFunc = FO_COPY
            SHFileOp.pFrom = "hosts" & vbNullChar & vbNullChar
            SHFileOp.pTo = "C:\Windows\System32\drivers\etc\hosts" & vbNullChar & vbNullChar
            If (SHFileOperation(SHFileOp) = 0) Then Exit Try
            MsgBox(ex.Message, MsgBoxStyle.Critical)
            MsgBox("hosts应用失败，终止！", MsgBoxStyle.Critical)
            MsgBox("请尝试手动将目录下的hosts文件复制到系统相应位置", MsgBoxStyle.Information)
            Me.Button1.Enabled = True
            Me.Button1.Text = "一键科学上网！"
            Return
        End Try
        MsgBox("科学上网配置完成！", MsgBoxStyle.Information)
        Me.Button1.Text = "一键科学上网！"
        Me.Button1.Enabled = True
        Try
            FileIO.FileSystem.DeleteFile("hostsFromLoad.zip")
            FileIO.FileSystem.DeleteFile("hosts")
            FileIO.FileSystem.DeleteFile("7za.exe")
        Catch
        End Try
        'AddHandler broswer.DocumentCompleted, AddressOf ApplyHostsFromLaoDState2
        'broswer.Url = New Uri("https://laod.cn/hosts/2017-google-hosts.html")
        Return
    End Sub

    Private Sub Button3_Click(sender As Object, e As EventArgs) Handles Button3.Click
        ConfigDeleteItem.ShowDialog()
    End Sub
End Class