Public Class Form1
    Public Declare Function GetAsyncKeyState Lib "user32 " (ByVal vKey As Integer) As Integer
    Dim a As New Media.SoundPlayer
    Dim Offset As Integer = 0
    Private Sub Form1_Load(sender As Object, e As EventArgs) Handles MyBase.Load
        AxWindowsMediaPlayer1.settings.autoStart = False
        'AxWindowsMediaPlayer1.Ctlcontrols.play()
    End Sub

    Private Sub Form1_MouseDoubleClick(sender As Object, e As MouseEventArgs) Handles Me.MouseDoubleClick
        OpenFileDialog1.ShowDialog()
        AxWindowsMediaPlayer1.URL = OpenFileDialog1.FileName
    End Sub

    Private Sub TrackBar1_ValueChanged(sender As Object, e As EventArgs) Handles TrackBar1.ValueChanged
        AxWindowsMediaPlayer1.settings.rate = TrackBar1.Value / 10
    End Sub

    Private Sub Button1_Click(sender As Object, e As EventArgs) Handles Button1.Click
        FileClose(1)
        FileOpen(1, "C.cfg", OpenAccess.Write)
        AxWindowsMediaPlayer1.Ctlcontrols.stop()
        AxWindowsMediaPlayer1.Ctlcontrols.play()
        Button2.Enabled = False
    End Sub

    Private Sub Timer1_Tick(sender As Object, e As EventArgs) Handles Timer1.Tick
        On Error Resume Next
        ProgressBar1.Value = AxWindowsMediaPlayer1.Ctlcontrols.currentPosition / AxWindowsMediaPlayer1.currentMedia.duration * 100
        Dim g As Graphics = Me.CreateGraphics()
        Dim BlueB As SolidBrush = New SolidBrush(Color.Blue)
        Dim WhiteB As SolidBrush = New SolidBrush(Color.White)
        Dim RedB As SolidBrush = New SolidBrush(Color.Red)
        g.FillRectangle(WhiteB, New Rectangle(10, 150, 70, 10))
        If GetAsyncKeyState(Keys.D) = -32767 Then g.FillEllipse(BlueB, New Rectangle(10, 150, 10, 10))
        If GetAsyncKeyState(Keys.F) = -32767 Then g.FillEllipse(BlueB, New Rectangle(30, 150, 10, 10))
        If GetAsyncKeyState(Keys.NumPad1) = -32767 Then g.FillEllipse(BlueB, New Rectangle(50, 150, 10, 10))
        If GetAsyncKeyState(Keys.NumPad2) = -32767 Then g.FillEllipse(BlueB, New Rectangle(70, 150, 10, 10))
        If GetAsyncKeyState(Keys.E) = -32767 Then g.FillEllipse(RedB, New Rectangle(10, 150, 10, 10))
        If GetAsyncKeyState(Keys.R) = -32767 Then g.FillEllipse(RedB, New Rectangle(30, 150, 10, 10))
        If GetAsyncKeyState(Keys.NumPad4) = -32767 Then g.FillEllipse(RedB, New Rectangle(50, 150, 10, 10))
        If GetAsyncKeyState(Keys.NumPad5) = -32767 Then g.FillEllipse(RedB, New Rectangle(70, 150, 10, 10))
    End Sub

    Private Sub Timer2_Tick(sender As Object, e As EventArgs) Handles Timer2.Tick
        PrintLine(1, CStr(CInt(AxWindowsMediaPlayer1.Ctlcontrols.currentPosition * 1000 - 100) + Offset) & ",0,1")
    End Sub

    Private Sub Timer3_Tick(sender As Object, e As EventArgs) Handles Timer3.Tick
        PrintLine(1, CStr(CInt(AxWindowsMediaPlayer1.Ctlcontrols.currentPosition * 1000 - 100) + Offset) & ",1,1")
    End Sub

    Private Sub Timer4_Tick(sender As Object, e As EventArgs) Handles Timer4.Tick
        PrintLine(1, CStr(CInt(AxWindowsMediaPlayer1.Ctlcontrols.currentPosition * 1000 - 100) + Offset) & ",2,1")
    End Sub

    Private Sub Timer5_Tick(sender As Object, e As EventArgs) Handles Timer5.Tick
        PrintLine(1, CStr(CInt(AxWindowsMediaPlayer1.Ctlcontrols.currentPosition * 1000 - 100) + Offset) & ",3,1")
    End Sub


    Private Sub Button2_Click(sender As Object, e As EventArgs) Handles Button2.Click
        Dim ErrStr As String = ""
        Dim LastRhythmTime As Integer
        Dim LastRhythmTime2 As Integer
        Dim LastRhythmStr As String
        Dim HaveNormalNote As Boolean
        For i = 0 To 3
            FileOpen(1, "C.cfg", OpenAccess.Read)
            While Not EOF(1)
                LastRhythmStr = LineInput(1)
                If LastRhythmStr.Split(",")(1) = i And LastRhythmStr.Split(",")(2) <> 1 Then
                    LastRhythmTime = LastRhythmStr.Split(",")(0)
                    Exit While
                End If
            End While
            While Not EOF(1)
                LastRhythmStr = LineInput(1)
                If LastRhythmStr.Split(",")(1) = i And LastRhythmStr.Split(",")(2) <> 1 Then
                    LastRhythmTime2 = LastRhythmStr.Split(",")(0)
                    If LastRhythmTime2 - LastRhythmTime < 200 Then
                        ErrStr = ErrStr + LastRhythmStr + vbCrLf
                    End If
                    LastRhythmTime = LastRhythmTime2
                End If
            End While
            FileClose(1)
        Next
        If ErrStr <> "" Then MsgBox("发现部分同轨道点击音符距离过近：" + vbCrLf + ErrStr)
        ErrStr = ""
        For i = 0 To 3
            FileOpen(1, "C.cfg", OpenAccess.Read)
            While Not EOF(1)
                LastRhythmStr = LineInput(1)
                If LastRhythmStr.Split(",")(1) = i And LastRhythmStr.Split(",")(2) <> 0 Then
                    LastRhythmTime = LastRhythmStr.Split(",")(0)
                    Exit While
                End If
            End While
            While Not EOF(1)
                LastRhythmStr = LineInput(1)
                If LastRhythmStr.Split(",")(1) = i Then
                    If LastRhythmStr.Split(",")(2) = 1 Then
                        If LastRhythmStr.Split(",")(0) - LastRhythmTime < 300 And HaveNormalNote = True Then
                            ErrStr = ErrStr & LastRhythmStr & vbCrLf
                        End If
                        LastRhythmTime = LastRhythmStr.Split(",")(0)
                        HaveNormalNote = False
                    Else
                        HaveNormalNote = True
                    End If
                End If
            End While
            FileClose(1)
            HaveNormalNote = False
        Next
        If ErrStr <> "" Then MsgBox("发现部分同轨道距离很近的按住音符间夹杂有点击音符：" + vbCrLf + ErrStr)
    End Sub

    Private Sub TextBox1_TextChanged(sender As Object, e As EventArgs) Handles TextBox1.TextChanged
        On Error Resume Next
        Offset = TextBox1.Text
    End Sub

    Private Sub Form1_KeyUp(sender As Object, e As KeyEventArgs) Handles Me.KeyUp
        If Button2.Enabled Then Exit Sub
        Select Case e.KeyCode
            Case Keys.E
                Timer2.Enabled = False
                PrintLine(1, CStr(CInt(AxWindowsMediaPlayer1.Ctlcontrols.currentPosition * 1000) + Offset) & ",0,1")
            Case Keys.R
                Timer3.Enabled = False
                PrintLine(1, CStr(CInt(AxWindowsMediaPlayer1.Ctlcontrols.currentPosition * 1000) + Offset) & ",1,1")
            Case Keys.NumPad4
                Timer4.Enabled = False
                PrintLine(1, CStr(CInt(AxWindowsMediaPlayer1.Ctlcontrols.currentPosition * 1000) + Offset) & ",2,1")
            Case Keys.NumPad5
                Timer5.Enabled = False
                PrintLine(1, CStr(CInt(AxWindowsMediaPlayer1.Ctlcontrols.currentPosition * 1000) + Offset) & ",3,1")
        End Select
    End Sub

    Private Sub Form1_KeyDown(sender As Object, e As KeyEventArgs) Handles Me.KeyDown
        If Button2.Enabled Then Exit Sub
        Select Case e.KeyCode
            Case Keys.D
                PrintLine(1, CStr(CInt(AxWindowsMediaPlayer1.Ctlcontrols.currentPosition * 1000) + Offset) & ",0,0")
            Case Keys.F
                PrintLine(1, CStr(CInt(AxWindowsMediaPlayer1.Ctlcontrols.currentPosition * 1000) + Offset) & ",1,0")
            Case Keys.NumPad1
                PrintLine(1, CStr(CInt(AxWindowsMediaPlayer1.Ctlcontrols.currentPosition * 1000) + Offset) & ",2,0")
            Case Keys.NumPad2
                PrintLine(1, CStr(CInt(AxWindowsMediaPlayer1.Ctlcontrols.currentPosition * 1000) + Offset) & ",3,0")
            Case Keys.E
                Timer2.Enabled = True
            Case Keys.R
                Timer3.Enabled = True
            Case Keys.NumPad4
                Timer4.Enabled = True
            Case Keys.NumPad5
                Timer5.Enabled = True
        End Select
    End Sub

    Protected Overrides Sub OnPaint(ByVal e As System.Windows.Forms.PaintEventArgs)

    End Sub
    Private Sub ProgressBar1_MouseUp(sender As Object, e As MouseEventArgs) Handles ProgressBar1.MouseUp
        AxWindowsMediaPlayer1.Ctlcontrols.currentPosition = (e.Location.X - ProgressBar1.Location.X) / ProgressBar1.Size.Width * AxWindowsMediaPlayer1.currentMedia.duration
    End Sub

    Private Sub Button3_Click(sender As Object, e As EventArgs) Handles Button3.Click
        Dim Str As String
        Dim Time(0) As Integer
        Dim Position(0) As String
        Dim Type(0) As String
        FileOpen(1, "C.cfg", OpenAccess.Read)
        While Not EOF(1)
            Str = LineInput(1)
            Time(UBound(Time)) = Str.Split(",")(0)
            Position(UBound(Position)) = Str.Split(",")(1)
            Type(UBound(Type)) = Str.Split(",")(2)
            ReDim Preserve Time(UBound(Time) + 1)
            ReDim Preserve Position(UBound(Position) + 1)
            ReDim Preserve Type(UBound(Type) + 1)
        End While
        FileClose(1)
        FileOpen(1, "C.cfg", OpenAccess.Write)
        For i = 0 To UBound(Time) - 1
            PrintLine(1, Time(i) + Offset & "," & Position(i) & "," & Type(i))
        Next
        FileClose(1)
        MsgBox("Finish")
    End Sub

    Private Sub Button4_Click(sender As Object, e As EventArgs) Handles Button4.Click
        Dim Str As String
        Dim Time(0) As Integer
        Dim Position(0) As String
        Dim Type(0) As String
        Dim L As Integer = 0
        FileOpen(1, "C.cfg", OpenAccess.Read)
        While Not EOF(1)
            Str = LineInput(1)
            Time(UBound(Time)) = Str.Split(",")(0)
            Position(UBound(Position)) = Str.Split(",")(1)
            Type(UBound(Type)) = Str.Split(",")(2)
            ReDim Preserve Time(UBound(Time) + 1)
            ReDim Preserve Position(UBound(Position) + 1)
            ReDim Preserve Type(UBound(Type) + 1)
        End While
        FileClose(1)
        FileOpen(1, "C.cfg", OpenAccess.Write)
        For i = 0 To UBound(Time) - 2
            If (Math.Abs(Time(i) - Time(i + 1)) < 100 And Type(i) = 0 And Type(i + 1) = 0) Then
                Time(I + 1) = Time(I)
                L = L + 1
            End If
            PrintLine(1, Time(i) & "," & Position(i) & "," & Type(i))
        Next
        PrintLine(1, Time(UBound(Time) - 1) & "," & Position(UBound(Time) - 1) & "," & Type(UBound(Time) - 1))
        FileClose(1)
        MsgBox("Finish," & L & "个音符已修正")
    End Sub
End Class