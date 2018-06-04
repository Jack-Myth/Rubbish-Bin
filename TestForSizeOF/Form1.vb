Imports System.Runtime.InteropServices

Public Class Form1
    Private Sub Form1_Load(sender As Object, e As EventArgs) Handles MyBase.Load
        MsgBox(Marshal.SizeOf(Me.GetType()))
    End Sub
End Class