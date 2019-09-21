using System;
using System.Collections.Generic;
using System.Drawing;
using System.Drawing.Imaging;
using System.IO;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows;
using System.Windows.Controls;
using System.Windows.Data;
using System.Windows.Documents;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;
using Microsoft.Win32;
using Brush = System.Windows.Media.Brush;
using ColorConverter = System.Windows.Media.ColorConverter;
using Image = System.Drawing.Image;
using Rectangle = System.Windows.Shapes.Rectangle;

namespace SteamLibCoverGroupMaker
{
    /// <summary>
    /// WindowSetting.xaml 的交互逻辑
    /// </summary>
    public partial class WindowSetting : Window
    {
        double DpiX = Graphics.FromHwnd(IntPtr.Zero).DpiX / 96.0;
        double DpiY = Graphics.FromHwnd(IntPtr.Zero).DpiY / 96.0;
        private Bitmap PreviewImg;
        private MainWindow mMainWindow;
        private List<Button> PreviewPositionRect=new List<Button>();
        private bool Useffmpeg = false;
        private string SourceFile;
        private string FFmpegPath;
        public WindowSetting()
        {
            InitializeComponent();
        }

        BitmapImage BitmapToImageSource(Bitmap bitmap)
        {
            using (MemoryStream memory = new MemoryStream())
            {
                bitmap.Save(memory, System.Drawing.Imaging.ImageFormat.Bmp);
                memory.Position = 0;
                BitmapImage bitmapimage = new BitmapImage();
                bitmapimage.BeginInit();
                bitmapimage.StreamSource = memory;
                bitmapimage.CacheOption = BitmapCacheOption.OnLoad;
                bitmapimage.EndInit();

                return bitmapimage;
            }
        }

        private bool IsFFmpegValid()
        {
            return (FFmpegPath != "" && System.IO.File.Exists(FFmpegPath));
        }

        Bitmap LoadThumbnail(string filePath)
        {
            string tmpName = System.IO.Path.GetTempFileName();
            System.IO.File.Delete(tmpName);
            tmpName = tmpName + ".png";
            System.IO.File.Delete(tmpName);
            System.Diagnostics.ProcessStartInfo psi = new System.Diagnostics.ProcessStartInfo(FFmpegPath);
            psi.CreateNoWindow = true;
            psi.Arguments = "-i \"" + filePath + "\" -vframes 1 \"" + tmpName+"\"";
            psi.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;
            psi.UseShellExecute = false;
            var process = System.Diagnostics.Process.Start(psi);
            if (process == null)
                return null;
            process.WaitForExit();
            if (!System.IO.File.Exists(tmpName))
                return null;
            var Bitm = new Bitmap(tmpName);
            //System.IO.File.Delete(tmpName);
            return Bitm;
        }

        private void Button_Click(object sender, RoutedEventArgs e)
        {
            var dialog = new OpenFileDialog();
            dialog.Filter = "Image or Video|*.png;*.jpg;*.gif;*.mp4";
            if (dialog.ShowDialog() == false)
                return;
            string FilePath = dialog.FileName;
            if ((FilePath.EndsWith("gif", StringComparison.CurrentCultureIgnoreCase) ||
                 FilePath.EndsWith("mp4", StringComparison.CurrentCultureIgnoreCase)))
            {
                if (!IsFFmpegValid())
                {
                    MessageBox.Show("看起来你选择了一个会动的玩意儿，如果你想要处理它你必须指定ffmpeg的路径.");
                    return;
                }
                if (FilePath.EndsWith("gif", StringComparison.CurrentCultureIgnoreCase))
                    MessageBox.Show("看起来你选择了一个GIF，如果你有原始视频的话可以直接选择MP4,这样可以获得更好的质量.");
                PreviewImg = LoadThumbnail(FilePath);
                if(PreviewImg==null)
                {
                    MessageBox.Show("获取帧失败!请检查ffmpeg和图片是否正确");
                    return;
                }

                SourceFile = FilePath;
                Useffmpeg = true;
            }
            else
            {
                PreviewImg=new Bitmap(FilePath);
                Useffmpeg = false;
                SourceFile = FilePath;
            }

            if (mMainWindow==null)
                mMainWindow=new MainWindow();
            mMainWindow.Show();
            mMainWindow.PreviewImage.Source = BitmapToImageSource(PreviewImg);
            mMainWindow.Width = PreviewImg.Width/DpiX;
            mMainWindow.Height = PreviewImg.Height / DpiY + SystemParameters.CaptionHeight;
            RefreshPreviewPositionRect();
            this.Export.IsEnabled = true;
        }

        private void RefreshPreviewPositionRect()
        {
            if (mMainWindow == null)
                return;
            int HCount = (int)this.HCountSlider.Value;
            int VCount = (int)this.VCountSlider.Value;
            float Scale = (float) this.ScaleSlider.Value;
            float SplitXScale = (float) this.SplitSlider.Value;
            float StartPosX = (float) this.XStartSlider.Value;
            float StartPosY = (float) this.YStartSlider.Value;
            int defaultwidth = 558;
            int defaultheight = 800;
            int defaultSplitY = 115;
            int defaultSplitX = 77;
            

            //Add element if not enough
            if (PreviewPositionRect.Count < HCount * VCount)
            {
                int RemainCount = HCount * VCount - PreviewPositionRect.Count;
                for (int i = 0; i < RemainCount; i++)
                {
                    var newrect = new Button();
                    newrect.Opacity = 0.7;
                    mMainWindow.MainGrid.Children.Add(newrect);
                    newrect.Visibility = Visibility.Visible;
                    PreviewPositionRect.Add(newrect);
                }
            }
            //Remove no needed rect
            while (PreviewPositionRect.Count>HCount*VCount)
            {
                mMainWindow.MainGrid.Children.Remove(PreviewPositionRect.Last());
                PreviewPositionRect.Remove(PreviewPositionRect.Last());
            }

            for (int x = 0; x < HCount; x++)
            {
                for (int y = 0; y < VCount; y++)
                {
                    PreviewPositionRect[x * VCount + y].Width = defaultwidth*Scale/DpiX;
                    PreviewPositionRect[x * VCount + y].Height= defaultheight * Scale/DpiY;
                    Thickness tmpMargin= PreviewPositionRect[x * VCount + y].Margin;
                    PreviewPositionRect[x * VCount + y].HorizontalAlignment = HorizontalAlignment.Left;
                    PreviewPositionRect[x * VCount + y].VerticalAlignment = VerticalAlignment.Top;
                    tmpMargin.Left = ((defaultSplitX * SplitXScale + defaultwidth) * x *
                                      Scale+ PreviewImg.Width* StartPosX)/DpiX + mMainWindow.PreviewImage.Margin.Left;
                    tmpMargin.Top = ((defaultSplitY+ defaultheight) * y * 
                                     Scale + PreviewImg.Height* StartPosY) / DpiY+ mMainWindow.PreviewImage.Margin.Top;
                    PreviewPositionRect[x * VCount + y].Margin = tmpMargin;
                }
            }
            mMainWindow.MainGrid.UpdateLayout();
        }

        private void Slider_ValueChanged(object sender, RoutedPropertyChangedEventArgs<double> e)
        {
            RefreshPreviewPositionRect();
        }

        Image[] getFrames(Image originalImg)
        {
            int numberOfFrames;
            FrameDimension TargetDimension;
            try
            {
                numberOfFrames = originalImg.GetFrameCount(FrameDimension.Time);
                TargetDimension = FrameDimension.Time;
            }
            catch
            {
                numberOfFrames = originalImg.GetFrameCount(FrameDimension.Page);
                TargetDimension = FrameDimension.Page;
            }
            
            Image[] frames = new Image[numberOfFrames];

            for (int i = 0; i < numberOfFrames; i++)
            {
                originalImg.SelectActiveFrame(TargetDimension, i);
                frames[i] = ((Image)originalImg.Clone());
            }

            return frames;
        }

        private async void Export_Click(object sender, RoutedEventArgs e)
        {
            SaveFileDialog dialog = new SaveFileDialog();
            dialog.Filter = "PNG(Actually WebP)(*.png)|*.png";
            if (dialog.ShowDialog() != true)
                return;
            string TargetFileNameBase = dialog.FileName;
            string FileName = System.IO.Path.GetFileNameWithoutExtension(TargetFileNameBase);
            string FilePath = System.IO.Path.GetDirectoryName(TargetFileNameBase);
            int HCount = (int) this.HCountSlider.Value;
            int VCount = (int) this.VCountSlider.Value;
            float Scale = (float) this.ScaleSlider.Value;
            float SplitXScale = (float) this.SplitSlider.Value;
            float StartPosX = (float) this.XStartSlider.Value;
            float StartPosY = (float) this.YStartSlider.Value;
            int defaultwidth = 558;
            int defaultheight = 800;
            int defaultSplitY = 115;
            int defaultSplitX = 77;

            if (!Useffmpeg)
            {
                for (int x = 0; x < HCount; x++)
                {
                    for (int y = 0; y < VCount; y++)
                    {
                        var TargetImage = new Bitmap((int)(defaultwidth*Scale), (int)(defaultheight * Scale));
                        var workingGraph = Graphics.FromImage(TargetImage);
                        workingGraph.DrawImage(PreviewImg,
                            new System.Drawing.Rectangle(0, 0, TargetImage.Width, TargetImage.Height),
                            (defaultSplitX * SplitXScale + defaultwidth) * x * Scale + PreviewImg.Width * StartPosX,
                            (defaultSplitY + defaultheight) * y * Scale + PreviewImg.Height * StartPosY,
                            defaultwidth * Scale, defaultheight * Scale, GraphicsUnit.Pixel);
                        string TargeFilename = System.IO.Path.Combine(FilePath, FileName +
                                                                                "_" + x.ToString() + "_" +
                                                                                y.ToString() + ".png");
                        TargetImage.Save(TargeFilename);
                    }
                }
            }
            else
            {
                this.Export.IsEnabled = false;
                this.Processing.IsIndeterminate = true;
                await Task.Run(() =>
                {
                    for (int x = 0; x < HCount; x++)
                    {
                        for (int y = 0; y < VCount; y++)
                        {
                            string tmpName = System.IO.Path.Combine(FilePath, FileName +
                                                                              "_" + x.ToString() + "_" +
                                                                              y.ToString() + ".webp");
                            string TargetFilename = System.IO.Path.Combine(FilePath, FileName +
                                                                                     "_" + x.ToString() + "_" +
                                                                                     y.ToString() + ".png");
                            System.Diagnostics.ProcessStartInfo psi = new System.Diagnostics.ProcessStartInfo(FFmpegPath);
                            psi.CreateNoWindow = true;
                            psi.Arguments = "-i \"" + SourceFile + "\" -an -vf crop="+ 
                                            ((int)(defaultwidth*Scale)).ToString()+":"+
                                            ((int)(defaultheight * Scale)).ToString()+":" +
                                            ((int)((defaultSplitX * SplitXScale + defaultwidth) * x * Scale + PreviewImg.Width * StartPosX)).ToString()+":"+
                                            ((int)((defaultSplitY + defaultheight) * y * Scale + PreviewImg.Height* StartPosY)).ToString()+":"+" "+
                                            tmpName;
                            psi.WindowStyle = System.Diagnostics.ProcessWindowStyle.Hidden;
                            psi.UseShellExecute = false;
                            var process = System.Diagnostics.Process.Start(psi);
                            if(process==null)
                                return;
                            process.WaitForExit();
                            if (!System.IO.File.Exists(tmpName))
                                return;
                            System.IO.File.Move(tmpName, TargetFilename);
                        }
                    }

                    MessageBox.Show("裁剪已完成，请注意导出的png不一定能够播放，但是在Steam中是可以播放的。具体请前往Steam预览");
                });
                this.Export.IsEnabled = true;
                this.Processing.IsIndeterminate = false;
            }
        }

        private void Button_Click_1(object sender, RoutedEventArgs e)
        {
            var dialog=new OpenFileDialog();
            dialog.Filter = "EXE|*.exe";
            if (dialog.ShowDialog()!=true)
                return;
            FFmpegPath = dialog.FileName;
            this.SelectFFmpeg.Content = "FFMPEG已选择";
        }

        private void Window_Closed(object sender, EventArgs e)
        {
            Application.Current.Shutdown(0);
        }
    }
}
