Imports System.Net.Sockets
Imports System.Net
Imports System.Text

Public Class MainForm
    Dim SktConnection As UdpClient

    Private Sub Form1_Load(sender As Object, e As EventArgs) Handles Me.Load
        SktConnection = New UdpClient(65401)
        SktConnection.BeginReceive(AddressOf OnDataReceived, Nothing)
    End Sub

    Private Sub OnDataReceived(IAR As IAsyncResult)

    End Sub

    Private Sub Button1_Click_1(sender As Object, e As EventArgs) Handles Button1.Click
        ShareSetting.Show()
    End Sub
End Class
