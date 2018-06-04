Imports System.Net
Imports System.Net.Sockets
Imports System.Text


Public Class MainForm
    Dim udpc As New UdpClient(1703)
    Dim Map As New Collection
    Dim RndManager As New Random
    Delegate Sub ST(T As String)
    Dim ST_T As New ST(AddressOf SetText)

    Private Sub MainForm_Load(sender As Object, e As EventArgs) Handles MyBase.Load
        udpc.BeginReceive(AddressOf ReceiveCallback, Nothing)
    End Sub

    Private Sub ReceiveCallback(ar As IAsyncResult)
        Dim Addr As New IPEndPoint(IPAddress.Any, 0)
        Dim Dt As Byte() = udpc.EndReceive(ar, Addr)
        Dim Str As String = Encoding.ASCII.GetString(Dt)
        Select Case Str.Substring(0, 4)
            Case "ADDN"
                Dim HostID As Integer
                Do
                    HostID = (RndManager.Next() Mod 899999 + 100000)
                Loop While (Map.Contains(HostID))
                Map.Add(Addr, HostID)
                Me.TextBox1.Invoke(ST_T, "C:ADDN")
                Dim AAA = Encoding.ASCII.GetBytes("ACPT " + HostID.ToString + vbNullChar)
                udpc.Send(Encoding.ASCII.GetBytes("ACPT " + HostID.ToString + vbNullChar), Encoding.ASCII.GetBytes("ACPT " + HostID.ToString + vbNullChar).Length, Addr)
                Me.TextBox1.Invoke(ST_T, "S:ACPT")
            Case "KILL"
                Me.TextBox1.Invoke(ST_T, "C:KILL")
                If Map.Contains(Str.Substring(5, 6)) Then Map.Remove(Str.Substring(5, 6))
            Case "CRNT"
                Me.TextBox1.Invoke(ST_T, "C:CRNT")
                If Map.Contains(Str.Substring(5, 6)) Then
                    Dim A = CType(Map.Item(Str.Substring(5, 6)), IPEndPoint)
                    udpc.Send(Encoding.ASCII.GetBytes("NATT " + A.Address.ToString + ":" + A.Port + vbNullChar), Encoding.ASCII.GetBytes("NATT " + A.Address.ToString + ":" + A.Port + vbNullChar).Length, Addr)
                    Me.TextBox1.Invoke(ST_T, "S:NATT")
                    udpc.Send(Encoding.ASCII.GetBytes("NATT " + Addr.Address.ToString + ":" + A.Port + vbNullChar), Encoding.ASCII.GetBytes("NATT " + Addr.Address.ToString + ":" + A.Port + vbNullChar).Length, A)
                    Me.TextBox1.Invoke(ST_T, "S:NATT")
                End If
            Case Else
                Me.TextBox1.Invoke(ST_T, "C:UNRECOGNIZECMD:" + Str)
        End Select
        udpc.BeginReceive(AddressOf ReceiveCallback, Nothing)
    End Sub

    Private Sub SetText(T As String)
        Me.TextBox1.Text = Me.TextBox1.Text + vbCrLf + T
        Me.TextBox1.Select(Me.TextBox1.Text.Length, 0)
    End Sub
End Class