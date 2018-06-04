Module FunctionLib
    Private Declare Function GetPrivateProfileString Lib "kernel32" Alias "GetPrivateProfileStringA" (ByVal lpApplicationName As String, ByVal lpKeyName As String, ByVal lpDefault As String, ByVal lpReturnedString As String, ByVal nSize As Integer, ByVal lpFileName As String) As Long
    Private Declare Function GetWindowsDirectory Lib "kernel32" Alias "GetWindowsDirectoryA" (ByVal lpBuffer As String, ByVal nSize As Long) As Long
    Private Declare Function WritePrivateProfileString Lib "kernel32" Alias "WritePrivateProfileStringA" (ByVal lpApplicationName As String, ByVal lpKeyName As String, ByVal lpString As String, ByVal lplFilename As String) As Long
    Structure DownloadInfomation
        Dim LocalFilePath As String
        Dim 
    End Structure

    Public Function WriteOneString(ByVal section As String, ByVal key As String, ByVal value As String, ByVal filename As String) As Long
        Dim X As Boolean
        Dim Nbuff As New String(CType(" ", Char), 1000)
        Nbuff = value + Chr(0)
        X = WritePrivateProfileString(section, key, Nbuff, filename)
        WriteOneString = X
    End Function

    Public Function ReadOneString(ByVal section As String, ByVal key As String, ByVal Filename As String) As String
        Dim X As Long, i As Integer
        Dim Buffer As New String(CType(" ", Char), 1000)
        X = GetPrivateProfileString(section, key, "", Buffer, 1000, Filename)
        i = InStr(Buffer, Chr(0))
        ReadOneString = Trim(Left(Buffer, i - 1))
    End Function
End Module
