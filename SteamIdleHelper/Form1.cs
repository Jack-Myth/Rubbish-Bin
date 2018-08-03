using System;
using System.Collections;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Diagnostics;
using System.Drawing;
using System.Linq;
using System.Runtime.InteropServices;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace SteamIdleHelper
{
    public partial class Form1 : Form
    {
        public const int WM_SHOWWINDOW = 0x0018;
        private const int SW_SHOWDEFAULT = 10;
        [DllImport("user32.dll")]
        private static extern bool ShowWindowAsync(IntPtr hWnd, int nCmdShow);

        public Form1()
        {
            InitializeComponent();
        }

        private void button1_Click(object sender, EventArgs e)
        {
            OpenFileDialog ofd = new OpenFileDialog();
            ofd.Filter = "TXT|*.txt";
            ofd.ValidateNames = true;
            ofd.CheckPathExists = true;
            ofd.CheckFileExists = true;
            ofd.Title = "选择条目文件";
            if (ofd.ShowDialog() == DialogResult.OK)
            {
                var fs= System.IO.File.OpenRead(ofd.FileName);
                System.IO.StreamReader read = new System.IO.StreamReader(fs, Encoding.Default);
                string Str;
                foreach (Process p in Process.GetProcessesByName("steam-idle"))
                {
                    p.Kill();
                }
                while (!read.EndOfStream&&(Str=read.ReadLine())!="")
                {
                    Process.Start(new ProcessStartInfo("steam-idle.exe", Str+ " 0"));
                }
            }
        }
        private bool EnumChild(IntPtr handle, string num)
        {
            return true;
        }

        private void button4_Click(object sender, EventArgs e)
        {
            foreach (Process p in Process.GetProcessesByName("steam-idle"))
            {
                p.Kill();
            }
        }

        private void button2_Click(object sender, EventArgs e)
        {
            IntPtr mainWindowHandle = IntPtr.Zero;
            foreach (Process p in Process.GetProcessesByName("steam-idle"))
            {
                ShowWindowAsync(User32API.GetWindowHandle((uint)p.Id), SW_SHOWDEFAULT);
            }
        }

        private void button3_Click(object sender, EventArgs e)
        {
            foreach (Process p in Process.GetProcessesByName("steam-idle"))
            {
                ShowWindowAsync(p.MainWindowHandle, 0);
            }
        }
    }
}

public class User32API
{
    private static Hashtable processWnd = null;

    public delegate bool WNDENUMPROC(IntPtr hwnd, uint lParam);

    static User32API()
    {
        if (processWnd == null)
        {
            processWnd = new Hashtable();
        }
    }

    [DllImport("user32.dll", EntryPoint = "EnumWindows", SetLastError = true)]
    public static extern bool EnumWindows(WNDENUMPROC lpEnumFunc, uint lParam);

    [DllImport("user32.dll", EntryPoint = "GetParent", SetLastError = true)]
    public static extern IntPtr GetParent(IntPtr hWnd);

    [DllImport("user32.dll", EntryPoint = "GetWindowThreadProcessId")]
    public static extern uint GetWindowThreadProcessId(IntPtr hWnd, ref uint lpdwProcessId);

    [DllImport("user32.dll", EntryPoint = "IsWindow")]
    public static extern bool IsWindow(IntPtr hWnd);

    [DllImport("kernel32.dll", EntryPoint = "SetLastError")]
    public static extern void SetLastError(uint dwErrCode);

    public static IntPtr GetWindowHandle(uint uiPid)
    {
        IntPtr ptrWnd = IntPtr.Zero;
        //uint uiPid = (uint)Process.GetCurrentProcess().Id;  // 当前进程 ID
        object objWnd = processWnd[uiPid];

        if (objWnd != null)
        {
            ptrWnd = (IntPtr)objWnd;
            if (ptrWnd != IntPtr.Zero && IsWindow(ptrWnd))  // 从缓存中获取句柄
            {
                return ptrWnd;
            }
            else
            {
                ptrWnd = IntPtr.Zero;
            }
        }

        bool bResult = EnumWindows(new WNDENUMPROC(EnumWindowsProc), uiPid);
        // 枚举窗口返回 false 并且没有错误号时表明获取成功
        if (!bResult && Marshal.GetLastWin32Error() == 0)
        {
            objWnd = processWnd[uiPid];
            if (objWnd != null)
            {
                ptrWnd = (IntPtr)objWnd;
            }
        }

        return ptrWnd;
    }

    private static bool EnumWindowsProc(IntPtr hwnd, uint lParam)
    {
        uint uiPid = 0;

        if (GetParent(hwnd) == IntPtr.Zero)
        {
            GetWindowThreadProcessId(hwnd, ref uiPid);
            if (uiPid == lParam)    // 找到进程对应的主窗口句柄
            {
                processWnd[uiPid] = hwnd;   // 把句柄缓存起来
                SetLastError(0);    // 设置无错误
                return false;   // 返回 false 以终止枚举窗口
            }
        }

        return true;
    }
}
