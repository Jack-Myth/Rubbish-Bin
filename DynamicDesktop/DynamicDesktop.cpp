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

class MediaEventCallback :public IMFAsyncCallback
{
	IMFMediaSession* MFSession = nullptr;
	int MediaWidth, MediaHeight;
public:
	MediaEventCallback(IMFMediaSession* _MFSession,int _MediaWidth,int _MediaHeight)
	{
		MFSession = _MFSession;
		MediaWidth = _MediaWidth;
		MediaHeight = _MediaHeight;
	}
	virtual HRESULT STDMETHODCALLTYPE GetParameters(__RPC__out DWORD* pdwFlags, __RPC__out DWORD* pdwQueue)
	{
		return S_OK;
	}


	virtual HRESULT STDMETHODCALLTYPE Invoke(__RPC__in_opt IMFAsyncResult* pAsyncResult)
	{
		IMFMediaEvent* Mediaevent;
		MFSession->EndGetEvent(pAsyncResult, &Mediaevent);
		MediaEventType met;
		Mediaevent->GetType(&met);
		switch (met)
		{
		case MEEndOfPresentation:
			if (ShouldLoop)
			{
				PROPVARIANT varStart;
				PropVariantInit(&varStart);
				varStart.vt = VT_I8;
				varStart.hVal.LowPart = 0;
				HRESULT hr = MFSession->Start(&GUID_NULL, &varStart);
				PropVariantClear(&varStart);
			}
			else
				exit(0);
			break;
		case MESessionClosed:
			exit(0);
			break;
		case MESessionTopologyStatus:
		{
			UINT32 status;
			HRESULT hr = Mediaevent->GetUINT32(MF_EVENT_TOPOLOGY_STATUS, &status);
			if (SUCCEEDED(hr) && (status == MF_TOPOSTATUS_READY))
			{

				// Get the IMFVideoDisplayControl interface from EVR. This call is
				// expected to fail if the media file does not have a video stream.
				IMFVideoDisplayControl* m_pVideoDisplay;
				(void)MFGetService(MFSession, MR_VIDEO_RENDER_SERVICE,
					IID_PPV_ARGS(&m_pVideoDisplay));
				float DPIScale = GetDpiForWindow(DesktopHwnd) / 96.f;
				if (FullVideo)
				{
					RECT rcDest = { 0, 0, long(ScreenWidth * DPIScale) ,long(ScreenHeight* DPIScale)};
					m_pVideoDisplay->SetVideoPosition(nullptr, &rcDest);
				}
				else
				{
					if ((float)MediaWidth / (float)MediaHeight > (float)ScreenWidth / (float)ScreenHeight)
					{
						int TargetWidth =(int)(ScreenHeight * (float)MediaWidth / (float)MediaHeight * DPIScale);
						int offet = (int)(TargetWidth - ScreenWidth* DPIScale);
						RECT rcDest = { -offet / 2, 0, -offet/2 + TargetWidth , int(ScreenHeight * DPIScale)};
						m_pVideoDisplay->SetVideoPosition(nullptr, &rcDest);
					}
					else
					{
						int TargetHeight = int(ScreenWidth * (float)MediaHeight/ (float)MediaWidth * DPIScale);
						int offet =(int)(TargetHeight - ScreenHeight* DPIScale);
						RECT rcDest = { 0,-offet / 2, (int)(ScreenWidth*DPIScale), -offet/2+ TargetHeight };
						m_pVideoDisplay->SetVideoPosition(nullptr, &rcDest);
					}
				}
				break;
			}
		}
		default:
			break;
		}
		return MFSession->BeginGetEvent(this, NULL);
	}


	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID riid, void** ppvObject)
	{
		return S_OK;
	}


	virtual ULONG STDMETHODCALLTYPE AddRef(void)
	{
		return S_OK;
	}


	virtual ULONG STDMETHODCALLTYPE Release(void)
	{
		return S_OK;
	}

};

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
		if (MFStartup(MF_VERSION) != S_OK)
		{
			MessageBox(nullptr, TEXT("初始化Media Foundation失败"), TEXT("Dynamic Desktop"), MB_OK);
			exit(-1);
		}
		IMFMediaSession* MediaSession=nullptr;
		MFCreateMediaSession(nullptr, &MediaSession);
		IMFSourceResolver* MediaResolver;
		MFCreateSourceResolver(&MediaResolver);
		MF_OBJECT_TYPE OBJType;
		IUnknown* MediaSource=NULL;
		MediaResolver->CreateObjectFromURL(VideoPath.c_str(),
			MF_RESOLUTION_MEDIASOURCE,
			NULL,
			&OBJType,
			&MediaSource);
		IMFMediaSource* pMediaSource=NULL;
		MediaSource->QueryInterface(IID_PPV_ARGS(&pMediaSource));
		IMFPresentationDescriptor* PresentationDescriptor;
		pMediaSource->CreatePresentationDescriptor(&PresentationDescriptor);
		IMFTopology* pTopology = NULL;
		MFCreateTopology(&pTopology);
		DWORD cSourceStreams = 0;
		PresentationDescriptor->GetStreamDescriptorCount(&cSourceStreams);
		UINT32 MediaHeight = 0, MediaWidth = 0;
		for (DWORD i = 0; i < cSourceStreams; i++)
		{
			BOOL Selected=FALSE;
			IMFStreamDescriptor* pSD = NULL;
			PresentationDescriptor->GetStreamDescriptorByIndex(i, &Selected, &pSD);
			if (Selected)
			{
				IMFMediaTypeHandler* MediaTypeHandler;
				pSD->GetMediaTypeHandler(&MediaTypeHandler);
				GUID MajorType;
				MediaTypeHandler->GetMajorType(&MajorType);
				IMFActivate* pActivate = NULL;
				if (MajorType == MFMediaType_Audio)
				{
					if (NoSound)
						continue;
					else
						MFCreateAudioRendererActivate(&pActivate);
				}
				else if (MajorType == MFMediaType_Video)
				{
					MFCreateVideoRendererActivate(DesktopHwnd, &pActivate);
					IMFMediaType* MediaType;
					MediaTypeHandler->GetCurrentMediaType(&MediaType);

					MFGetAttributeSize(MediaType, MF_MT_FRAME_SIZE, &MediaWidth, &MediaHeight);
				}
				IMFTopologyNode* SourceTopologyNode,*OutputTopologyNode;
				MFCreateTopologyNode(MF_TOPOLOGY_SOURCESTREAM_NODE, &SourceTopologyNode);
				SourceTopologyNode->SetUnknown(MF_TOPONODE_SOURCE, pMediaSource);
				SourceTopologyNode->SetUnknown(MF_TOPONODE_PRESENTATION_DESCRIPTOR, PresentationDescriptor);
				SourceTopologyNode->SetUnknown(MF_TOPONODE_STREAM_DESCRIPTOR, pSD);
				pTopology->AddNode(SourceTopologyNode);
				MFCreateTopologyNode(MF_TOPOLOGY_OUTPUT_NODE, &OutputTopologyNode);
				OutputTopologyNode->SetObject(pActivate);
				OutputTopologyNode->SetUINT32(MF_TOPONODE_NOSHUTDOWN_ON_REMOVE, TRUE);
				OutputTopologyNode->SetUINT32(MF_TOPONODE_STREAMID, 0);
				SourceTopologyNode->ConnectOutput(0, OutputTopologyNode, 0);
				pTopology->AddNode(OutputTopologyNode);
			}
		}
		MediaSession->SetTopology(MFSESSION_SETTOPOLOGY_IMMEDIATE, pTopology);
		PROPVARIANT varStart;
		PropVariantInit(&varStart);
		varStart.vt = VT_EMPTY;
		HRESULT hr = MediaSession->Start(&GUID_NULL, &varStart);
		PropVariantClear(&varStart);
		MediaSession->BeginGetEvent(new MediaEventCallback(MediaSession,MediaWidth,MediaHeight), NULL);
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