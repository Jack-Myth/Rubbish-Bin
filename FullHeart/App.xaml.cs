using System;
using System.Collections.Generic;
using System.Configuration;
using System.Data;
using System.Linq;
using System.Threading.Tasks;
using System.Windows;

namespace FullHeart
{
    /// <summary>
    /// App.xaml 的交互逻辑
    /// </summary>
    public partial class App : Application
    {
        private static double ScreenWidth = SystemParameters.WorkArea.Width;
        private static double ScreenHeight = SystemParameters.WorkArea.Height;

        [STAThread]
        private static void Main()
        {
            List<MainWindow> mws=new List<MainWindow>();
            for (double x = 0; x < ScreenWidth; x+=100)
            {
                for (double y = 0;  y< ScreenHeight; y+=100)
                {
                    var mw = new MainWindow();
                    mw.Top = y;
                    mw.Left = x;
                    mws.Add(mw);
                    mw.Show();
                }
            }

            MessageBox.Show("♥");
            for (int i = 0; i < mws.Count; i++)
                mws[i].Close();
            Environment.Exit(0);
        }
    }
}
