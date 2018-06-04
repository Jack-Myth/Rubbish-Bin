' https://go.microsoft.com/fwlink/?LinkId=402352&clcid=0x804 上介绍了“空白页”项模板

''' <summary>
''' 可用于自身或导航至 Frame 内部的空白页。
''' </summary>
Public NotInheritable Class MainPage
    Inherits Page
    Dim TimerForMusicUpdate As DispatcherTimer
    Dim CanSeekMediaPosition As Boolean
    Private Async Sub Button_ClickAsync(sender As Object, e As RoutedEventArgs)
        Dim FilePicker As New Windows.Storage.Pickers.FileOpenPicker
        FilePicker.FileTypeFilter.Add(".mp3")
        Dim File As Windows.Storage.StorageFile = Await FilePicker.PickSingleFileAsync()
        Dim Steam As Windows.Storage.Streams.IRandomAccessStream = Await File.OpenAsync(Windows.Storage.FileAccessMode.Read)
        mediaElement.SetSource(Steam, File.ContentType)
    End Sub

    Private Sub Button_Click(sender As Object, e As RoutedEventArgs)
        mediaElement.Play()
    End Sub

    Private Sub Button_Click_1(sender As Object, e As RoutedEventArgs)
        mediaElement.Pause()
    End Sub

    Private Sub MainPage_Loaded(sender As Object, e As RoutedEventArgs) Handles Me.Loaded
        TimerForMusicUpdate = New DispatcherTimer
        TimerForMusicUpdate.Interval = TimeSpan.FromSeconds(1)
        AddHandler TimerForMusicUpdate.Tick, AddressOf TimerTick
        TimerForMusicUpdate.Start()
    End Sub

    Private Sub TimerTick(sender As Object, e As Object)
        pBar.Maximum = mediaElement.NaturalDuration.TimeSpan.TotalSeconds
        pBar.Value = mediaElement.Position.TotalSeconds
    End Sub

    Private Sub Rectangle_Tapped(sender As Object, e As TappedRoutedEventArgs)
        Dim PointPoint As Point = e.GetPosition(sender)
        mediaElement.Position = TimeSpan.FromSeconds(PointPoint.X / CType(sender, Shapes.Rectangle).Width * pBar.Maximum)
        pBar.Maximum = mediaElement.NaturalDuration.TimeSpan.TotalSeconds
        pBar.Value = mediaElement.Position.TotalSeconds
    End Sub
End Class
