#include <stdio.h>
#include <mfapi.h>
#include <windows.h>
#include <string>
#include <chrono>
#include "LineAndCircle.h"
#include <mfidl.h>
#include <evr.h>
#include "Stars.h"
#include <corecrt_io.h>
#include <fcntl.h>
#include "MediaFoundationUtils.h"
#pragma comment(lib,"d2d1.lib")
#pragma comment(lib,"mfplat.lib")
#pragma comment(lib,"mf.lib")
#pragma comment(lib,"mfreadwrite.lib")
#pragma comment(lib,"mfuuid.lib")
#pragma  comment(lib,"Strmiids.lib")


//////////////////////////////////////////////////////////////////////////
struct ID2D1Factory* D2D1Factory = NULL;
struct ID2D1HwndRenderTarget* D2D1HwndRenderTarget = NULL;
struct IWICImagingFactory* WICImagingFactory = NULL;
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
IMFMediaSession* MediaSession = nullptr;
IMFSourceResolver* MediaResolver=nullptr;
IMFByteStream* CurMFByteStream=nullptr;
IMFMediaSource* CurMediaSource = NULL;
IMFTopology* pTopology = NULL;
//////////////////////////////////////////////////////////////////////////


HWND DesktopHwnd=NULL;
HWND DesktopWindowHwnd=NULL;
HHOOK hHook = NULL;
bool ShouldLoop = true;
bool NoSound = false;
bool FullVideo = false;
std::wstring VideoPath=TEXT("");
IModelBase* CurModel = nullptr;
int ScreenWidth = GetSystemMetrics(SM_CXSCREEN);
int ScreenHeight = GetSystemMetrics(SM_CYSCREEN);
inline void GetDesktopBackgroundHwnd()
{
	HWND WorkerWHwnd = FindWindow(TEXT("WorkerW"), TEXT(""));
	do 
	{
		bool bVisible = (::GetWindowLong(WorkerWHwnd, GWL_STYLE) & WS_VISIBLE) != 0;
		HWND ShellDLLHwnd = FindWindowEx(WorkerWHwnd, NULL, TEXT("SHELLDLL_DefView"), TEXT(""));
		if (ShellDLLHwnd)
		{
			DesktopWindowHwnd = WorkerWHwnd;
		}
		else if (bVisible)
			break;
		WorkerWHwnd= FindWindowEx(NULL, WorkerWHwnd, TEXT("WorkerW"), TEXT(""));
	} while (WorkerWHwnd);
	DesktopHwnd = WorkerWHwnd;
}

LRESULT CALLBACK LowLevelMouseProc(INT nCode, WPARAM wParam, LPARAM lParam);

int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd )
{
	srand((unsigned int)time(NULL));
	GetDesktopBackgroundHwnd();
	int Argc=1;
	LPWSTR* ArgvW = CommandLineToArgvW(lpCmdLine, &Argc);
	for (int i=0;i<Argc;i++)
	{
		auto cArg = std::wstring(ArgvW[i]);
		if (cArg==TEXT("-noloop"))
		{
			ShouldLoop = false;
		}
		else if (cArg == TEXT("-nosound"))
		{
			NoSound = true;
		}
		else if (cArg == TEXT("-fullvideo"))
		{
			FullVideo = true;
		}
		else if (cArg == TEXT("-h"))
		{
			MessageBox(NULL,TEXT("Dynamic Desktop\nDesigned By JackMyth\n\n-nosound\t不播放视频的声音\n-fullvideo\t在桌面上显示整个儿视频（可能有黑边）\n-noloop\t视频只播放一次（不循环）\n\n退出方法:\n星星特效\t中键点击桌面6次\n球和连线\t右键桌面直到没有球为止\n视频播放\t任务管理器->结束进程:P"),
				TEXT("Dynamic Desktop"),MB_OK);
			exit(0);
		}
		else
		{
			VideoPath = ArgvW[i];
		}
	}
	if (VideoPath == TEXT("") || wcslen(lpCmdLine)==0)
	{
		switch (rand()%2)
		{
		case 0:
		{
			auto* lac = new LineAndCircle();
			CurModel = lac;
			break;
		}
		case 1:
			auto * s = new Stars();
			CurModel = s;
			break;
		}
		CurModel->StartupModel();
#ifndef _DEBUG
		hHook=SetWindowsHookEx(WH_MOUSE_LL, (HOOKPROC)LowLevelMouseProc, hInstance, NULL);
#endif
		using namespace std::chrono;
		auto t = std::chrono::system_clock::now();
		SetTimer(nullptr, 0, 16, NULL);
		MSG msg = {NULL};
		while (1)
		{
			GetMessage(&msg, nullptr, 0, 0);
			if (msg.message == WM_TIMER)
			{
				auto ttmp = std::chrono::system_clock::now();
				auto td = ttmp - t;
				t = ttmp;
				CurModel->Tick(duration_cast<milliseconds>(td).count() / 1000.f);
			}
			else
				DispatchMessage(&msg);
		}
	}
	else
	{
		//Play Video
		if (!InitMediaFoundation())
		{
			MessageBox(nullptr, TEXT("初始化Media Foundation失败"), TEXT("Dynamic Desktop"), MB_OK);
			exit(-1);
		}
		auto* callback = new MediaEventCallback();
		callback->OnMediaEnd = []() {exit(0); };
		PlayVideoByURL(VideoPath.c_str(), DesktopHwnd, !NoSound, callback, ShouldLoop, FullVideo);
		while (1)
			Sleep(16);
	}
}

LRESULT CALLBACK LowLevelMouseProc(INT nCode, WPARAM wParam, LPARAM lParam)
{
	HWND CurWindow = GetForegroundWindow();
	bool IsOnDesktop = DesktopWindowHwnd == CurWindow;
	switch (nCode)
	{
	case HC_ACTION:
	{
		switch (wParam)
		{
		case WM_LBUTTONDOWN:
			CurModel->OnMouseDown(VK_LBUTTON, IsOnDesktop);
			break;
		case	 WM_RBUTTONDOWN:
			CurModel->OnMouseDown(VK_RBUTTON, IsOnDesktop);
			break;
		case WM_MBUTTONDOWN:
			CurModel->OnMouseDown(VK_MBUTTON, IsOnDesktop);
			break;
		case WM_LBUTTONUP:
			CurModel->OnMouseUp(VK_LBUTTON);
			break;
		case WM_RBUTTONUP:
			CurModel->OnMouseUp(VK_RBUTTON);
			break;
		case WM_MBUTTONUP:
			CurModel->OnMouseUp(VK_MBUTTON);
			break;
		}
	}
	}
	return CallNextHookEx(hHook, nCode, wParam, lParam);
}