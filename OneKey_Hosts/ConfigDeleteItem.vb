Imports System.ComponentModel

Public Class ConfigDeleteItem
    Private Const INSIDE_ITEMS_COUNT = 3
    Dim K As String

    Private Sub ConfigDeleteItem_Load(sender As Object, e As EventArgs) Handles MyBase.Load
        K = Profile.ReadOneString("Setting", "DeleteItems", Application.StartupPath & "/OneKeyHosts.Setting")
        If K.Equals("") Then Return
        Dim B = K.Split(",")
        For Each C As String In B
            For i = 0 To Me.CheckedListBox1.Items.Count - 1
                If Me.CheckedListBox1.Items(i).Equals(C) Then
                    Me.CheckedListBox1.SetItemChecked(i, True)
                    Exit For
                End If
            Next
        Next
    End Sub

    Private Sub ConfigDeleteItem_Closing(sender As Object, e As CancelEventArgs) Handles Me.Closing
        K = ""
        For Each S As String In Me.CheckedListBox1.CheckedItems
            K = K + "," + S
        Next
        If K.Length > 0 Then K = K.Substring(1)
        'If Not My.Computer.FileSystem.FileExists(Application.StartupPath & "/OneKeyHosts.Setting") Then IO.File.Create(Application.StartupPath & "/OneKeyHosts.Setting")
        Profile.WriteOneString("Setting", "DeleteItems", K, Application.StartupPath & "/OneKeyHosts.Setting")
    End Sub

    Private Sub CheckedListBox1_Click(sender As Object, e As EventArgs) Handles CheckedListBox1.Click
        ReDim Form1.DeleteItems(0)
        For Each S As String In Me.CheckedListBox1.CheckedItems
            Form1.DeleteItems(UBound(Form1.DeleteItems)) = S
            K = K + "," + S
            ReDim Preserve Form1.DeleteItems(UBound(Form1.DeleteItems) + 1)
        Next
        If K.Length > 0 Then K = K.Substring(1)
        ReDim Preserve Form1.DeleteItems(UBound(Form1.DeleteItems) - 1)
    End Sub
End Class