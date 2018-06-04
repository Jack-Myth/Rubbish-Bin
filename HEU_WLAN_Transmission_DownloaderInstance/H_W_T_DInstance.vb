Public Class H_W_T_DInstance
    Dim IsExplanded As Boolean

    Private Sub H_W_T_DInstance_MouseEnter(sender As Object, e As EventArgs) Handles Me.MouseEnter
        Me.BorderStyle = BorderStyle.FixedSingle
    End Sub

    Private Sub H_W_T_DInstance_MouseLeave(sender As Object, e As EventArgs) Handles Me.MouseLeave
        Me.BorderStyle = BorderStyle.None
    End Sub

    Private Sub H_W_T_DInstance_Click(sender As Object, e As EventArgs) Handles Me.Click
        If IsExplanded = False Then
            Me.Panel1.Visible = True
            IsExplanded = True
            For i = 80 To 166
                Me.Height = i
                i = i + 1
                System.Threading.Thread.Sleep(10)
            Next
        Else
            If IsExplanded = True Then
                IsExplanded = False
                Dim i As Integer = 166
                While (i > 80)
                    Me.Height = i
                    i = i - 2
                    System.Threading.Thread.Sleep(10)
                    Me.Panel1.Visible = False
                End While
                Me.Height = 80
            End If
        End If
    End Sub
End Class
