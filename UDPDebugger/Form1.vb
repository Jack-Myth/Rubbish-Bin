Imports System.Net
Imports System.Net.Sockets
Imports System.Text

Public Class Form1
    Dim UDPC As UdpClient
    Dim TargetIP As New IPEndPoint(IPAddress.Any, 0)
    Delegate Sub ST(T As String)
    Dim ST_Instance As New ST(AddressOf SetText)
    Private Sub Button1_Click(sender As Object, e As EventArgs) Handles Button1.Click
        If Me.TextBox5.ReadOnly = False Then
            Me.TextBox5.ReadOnly = True
            UDPC = New UdpClient(CInt(Me.TextBox5.Text))
            UDPC.BeginReceive(AddressOf OnUDPReceived, Nothing)
            Me.Button1.Text = "Send"
            Exit Sub
        End If
        TargetIP.Address = IPAddress.Parse(Me.TextBox2.Text)
        TargetIP.Port = Me.TextBox3.Text
        UDPC.Send(Encoding.ASCII.GetBytes(Me.TextBox1.Text), Me.TextBox1.TextLength, TargetIP)
        Me.TextBox4.Text = Me.TextBox4.Text + "Self:" + vbCrLf + Me.TextBox1.Text + vbCrLf
    End Sub

    Private Sub TextBox5_KeyPress(sender As Object, e As KeyPressEventArgs) Handles TextBox5.KeyPress
        If IsNumeric(e.KeyChar) = False Then
            e.Handled = False
        End If
    End Sub

    Private Sub OnUDPReceived(AR As IAsyncResult)
        Dim RemoteIP As New IPEndPoint(IPAddress.Any, 0)
        Dim DataStr = Encoding.ASCII.GetString(UDPC.EndReceive(AR, RemoteIP))
        Me.Invoke(ST_Instance, RemoteIP.ToString + ":" + vbCrLf + DataStr + vbCrLf)
        UDPC.BeginReceive(AddressOf OnUDPReceived, Nothing)
    End Sub
    Private Sub SetText(T As String)
        Me.TextBox4.Text = Me.TextBox4.Text + T
        Me.TextBox1.Select(Me.TextBox1.TextLength, 0)
    End Sub
End Class
