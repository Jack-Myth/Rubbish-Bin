Imports System.IO

Public Class Console
    Private Sub Button1_Click(sender As Object, e As EventArgs) Handles Button1.Click
        If (CurrentYGIntance.IsRunning = False) Then
            CurrentYGIntance.ProcessInstance.StartInfo.Arguments = TextBox_CMDLine.Text
            CurrentYGIntance.ProcessInstance.Start()
            CurrentYGIntance.IsRunning = True
        End If
    End Sub
End Class