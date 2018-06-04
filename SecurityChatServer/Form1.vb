Imports System.Net
Imports System.Net.Sockets
Imports System.Text



Public Class Form1
    Structure ChaterItem
        Dim ID As String
        Dim Addr As IPEndPoint
    End Structure
    Dim ChaterList As New Collection
    Dim SocketUDP As New UdpClient(1703)
    Delegate Sub ST(T As String)
    Dim ST_Instance As New ST(AddressOf SetText)
    Private Sub TextBox1_TextChanged(sender As Object, e As EventArgs) Handles TextBox1.TextChanged
        Me.TextBox1.Select(Me.TextBox1.TextLength - 1, 0)
    End Sub

    Private Sub Form1_Load(sender As Object, e As EventArgs) Handles MyBase.Load
        'SocketUDP.Client.Bind(SocketUDP.Client.LocalEndPoint)
        SocketUDP.BeginReceive(AddressOf OnUDPReceived, Nothing)
    End Sub
    Private Sub OnUDPReceived(a As IAsyncResult)
        Dim Addr As New IPEndPoint(IPAddress.Any, 0)
        Dim Datas = SocketUDP.EndReceive(a, Addr)
        Dim DataStr As String = Encoding.ASCII.GetString(Datas)
        Me.Invoke(ST_Instance, "C:" + DataStr + "         From:" + Addr.ToString + vbCrLf)
        Select Case (DataStr.Substring(0, 4))
            Case "XREG"
                If ChaterList.Contains(DataStr.Substring(4)) Then
                    Dim CI As New ChaterItem
                    CI.ID = CType(ChaterList.Item(DataStr.Substring(4)), ChaterItem).ID
                    ChaterList.Remove(DataStr.Substring(4))
                    CI.Addr = Addr
                    ChaterList.Add(CI, DataStr.Substring(4))
                    SocketUDP.Send(Encoding.ASCII.GetBytes("TRUE" + CI.ID), CI.ID.Length + 4, Addr)
                    Me.Invoke(ST_Instance, "S:" + "TRUE" + CI.ID + vbCrLf)
                Else
                    Dim CI As New ChaterItem
                    Do
                        CI.ID = Int(Microsoft.VisualBasic.VBMath.Rnd * 899999) + 100000
                    Loop While (IsValueExist(ChaterList, CI.ID))
                    CI.Addr = Addr
                    ChaterList.Add(CI, DataStr.Substring(4))
                    SocketUDP.Send(Encoding.ASCII.GetBytes("TRUE" + CI.ID), CI.ID.Length + 4, Addr)
                    Me.Invoke(ST_Instance, "S:" + "TRUE" + CI.ID + vbCrLf)
                End If
            Case "FIND"
                If ChaterList.Contains(DataStr.Substring(4)) Then
                    Dim TmpCI = CType(ChaterList.Item(DataStr.Substring(4)), ChaterItem)
                    SocketUDP.Send(Encoding.ASCII.GetBytes("TRUE" + TmpCI.Addr.ToString), TmpCI.Addr.ToString.Length + 4, Addr)
                    Me.Invoke(ST_Instance, "S:TRUE" + TmpCI.Addr.ToString + vbCrLf)
                Else
                    SocketUDP.Send(Encoding.ASCII.GetBytes("LOST"), 4, Addr)
                    Me.Invoke(ST_Instance, "S:LOST" + vbCrLf)
                End If
        End Select
        SocketUDP.BeginReceive(AddressOf OnUDPReceived, Nothing)
    End Sub
    Private Function IsValueExist(C As Collection, value As Object) As Boolean
        For i = 1 To C.Count Step 1
            If C.Item(i).Equals(value) Then Return True
        Next
        Return False
    End Function
    Private Sub SetText(T As String)
        Me.TextBox1.Text = Me.TextBox1.Text + T
        Me.TextBox1.Select(Me.TextBox1.TextLength, 0)
    End Sub
End Class
