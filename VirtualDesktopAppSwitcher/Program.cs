using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading;
using System.Threading.Tasks;
using System.Windows.Forms;
using WindowsDesktop;

namespace VirtualDesktopAppSwitcher
{

    class Program
    {
        [DllImport("user32.dll", EntryPoint = "ShowWindow", SetLastError = true)]
        static extern int ShowWindow(IntPtr hWnd, uint nCmdShow);
        static void Main(string[] args)
        {
            if (args.Length < 1)
                return;
            SynchronizationContext.SetSynchronizationContext(new SynchronizationContext());
            VirtualDesktopProvider.Default.Initialize().Wait();
            string TargetArgs = "";
            for (int i = 1; i < args.Length; i++)
                if (args[i].Contains(" "))
                    TargetArgs += "\"" + args[i] + "\" ";
                else
                    TargetArgs += args[i] + " ";
            Process targetProcess = Process.Start(args[0], TargetArgs);
            for (int i = 0; i < 100; i++)
            {
                Thread.Sleep(100);
                if (targetProcess.MainWindowHandle.ToInt64() != 0)
                {
                    VirtualDesktop[] desktops = VirtualDesktop.GetDesktops();
                    if (desktops.Length < 2)
                        desktops.Append(VirtualDesktop.Create());
                    VirtualDesktopHelper.MoveToDesktop(targetProcess.MainWindowHandle, desktops[1]);
                    //ShowWindow(targetProcess.MainWindowHandle, 1);
                    return;
                }
            }
            targetProcess.Kill();
        }
    }
}
