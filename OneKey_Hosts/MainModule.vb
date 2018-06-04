Imports System.IO
Imports System.Security.AccessControl

Public Class MainModule
    Public Declare Function URLDownloadToFile Lib "urlmon.dll" Alias "URLDownloadToFileA" (ByVal pCaller As Integer, ByVal szURL As String, ByVal szFileName As String, ByVal dwReserved As Integer, ByVal lpfnCB As Integer) As Integer

    Public Shared Sub Main()
        Application.EnableVisualStyles()
        If Environment.GetCommandLineArgs().Contains("-slientApply") Then
            If FileIO.FileSystem.FileExists("C:\Windows\System32\drivers\etc\hosts") Then addpathPower("C:\Windows\System32\drivers\etc\hosts", "Administrators", "FullControl")
            If FileIO.FileSystem.FileExists("hosts") Then FileIO.FileSystem.DeleteFile("hosts")
            If FileIO.FileSystem.FileExists("C:\Windows\System32\drivers\etc\hosts") And Environment.GetCommandLineArgs().Contains("-clear") Then
                Try
                    FileIO.FileSystem.DeleteFile("C:\Windows\System32\drivers\etc\hosts")
                    System.Threading.Thread.Sleep(1000)
                Catch ex As Exception
                    End
                End Try
            End If
            URLDownloadToFile(0, Environment.GetCommandLineArgs()(Array.IndexOf(Environment.GetCommandLineArgs(), "-dp") + 1), "hosts", 0, 0)
            If FileIO.FileSystem.FileExists("hosts") = False Then
                End
            End If
            Try
                If FileIO.FileSystem.FileExists("C:\Windows\System32\drivers\etc\hosts.bak") = False Then
                    FileIO.FileSystem.CopyFile("C:\Windows\System32\drivers\etc\hosts", "C:\Windows\System32\drivers\etc\hosts.bak")
                    'MsgBox("hosts文件已备份，位于C:\Windows\System32\drivers\etc\hosts.bak，如有必要可以使用此文件还原", MsgBoxStyle.Information)
                End If
            Catch
            End Try
            Try
                FileIO.FileSystem.CopyFile("hosts", "C:\Windows\System32\drivers\etc\hosts", True)
            Catch ex As Exception
            End Try
            Try
                FileIO.FileSystem.DeleteFile("hosts")
            Catch
            End Try
            End
        Else
            Application.Run(Form1)
        End If
    End Sub
    Public Shared Sub addpathPower(ByVal pathname As String, ByVal username As String, ByVal power As String)

        Dim dirinfo As FileInfo = New FileInfo(pathname)

        If (dirinfo.Attributes & FileAttributes.ReadOnly) <> 0 Then
            dirinfo.Attributes = FileAttributes.Normal
        End If

        '取得访问控制列表
        Dim dirsecurity As FileSecurity = dirinfo.GetAccessControl()
        Select Case power
            Case "FullControl"
                dirsecurity.AddAccessRule(New FileSystemAccessRule(username, FileSystemRights.FullControl, AccessControlType.Allow))
            Case "ReadOnly"
                dirsecurity.AddAccessRule(New FileSystemAccessRule(username, FileSystemRights.Read, AccessControlType.Allow))
            Case "Write"
                dirsecurity.AddAccessRule(New FileSystemAccessRule(username, FileSystemRights.Write, AccessControlType.Allow))
            Case "Modify"
                dirsecurity.AddAccessRule(New FileSystemAccessRule(username, FileSystemRights.Modify, AccessControlType.Allow))
        End Select
        dirinfo.SetAccessControl(dirsecurity)
    End Sub
End Class

Public Class Profile
    Private Declare Function GetPrivateProfileString Lib "kernel32" Alias "GetPrivateProfileStringA" (ByVal lpApplicationName As String, ByVal lpKeyName As String, ByVal lpDefault As String, ByVal lpReturnedString As String, ByVal nSize As Integer, ByVal lpFileName As String) As Long
    Private Declare Function GetWindowsDirectory Lib "kernel32" Alias "GetWindowsDirectoryA" (ByVal lpBuffer As String, ByVal nSize As Long) As Long
    Private Declare Function WritePrivateProfileString Lib "kernel32" Alias "WritePrivateProfileStringA" (ByVal lpApplicationName As String, ByVal lpKeyName As String, ByVal lpString As String, ByVal lplFilename As String) As Long
    Public Shared Function WriteOneString(ByVal section As String, ByVal key As String, ByVal value As String, ByVal filename As String) As Long
        Dim X As Boolean
        Dim Nbuff As New String(CType(" ", Char), 1000)
        Nbuff = value + Chr(0)
        X = WritePrivateProfileString(section, key, Nbuff, filename)
        WriteOneString = X
    End Function

    Public Shared Function ReadOneString(ByVal section As String, ByVal key As String, ByVal Filename As String) As String
        Dim X As Long, i As Integer

        Dim Buffer As New String(CType(" ", Char), 1000)
        X = GetPrivateProfileString(section, key, "", Buffer, 1000, Application.StartupPath & "/OneKeyHosts.Setting")
        i = InStr(Buffer, Chr(0))
        ReadOneString = Trim(Left(Buffer, i - 1))
    End Function
End Class
