#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#include <string>
#pragma comment(lib,"winmm.lib")//导入声音的链接库  
#pragma comment(lib,"d2d1.lib")//D2D1链接库
#pragma comment (lib, "dwrite.lib")//DWrite链接库
#define  LRHYTHEM -1
#define  MRHYTHEM 0
#define  RRHYTHEM 1
#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)

struct  _RhythmI
{
	int RhythmTime;
	int RhythmPosition;
	int RhythmType;
	int Score;
};

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void InitGame(LPSTR lpCmdLine);
void ProgressRender();
bool LoadD2DImage(WCHAR ImagePath[], ID2D1Bitmap** BitMapMayClear);
bool LoadD2DImageA(char ImagePath[], ID2D1Bitmap** BitMapMayClear);
void ProgressCaculate();
void ScoreLastNote(int Position);

ID2D1Factory* D2D1Factory = NULL;
IDWriteFactory* DWriteFactory = NULL;
ID2D1HwndRenderTarget* D2D1HwndRenderTarget = NULL;
IWICImagingFactory* WICImagingFactory = NULL;
HWND MainHwnd = NULL;
RECT WindowRect = { 0,0,405,720 };
_RhythmI RhythmInfo[1000] = {0};
ID2D1Bitmap* BackgroundPic = NULL, *RhythmPic0=NULL,*RhythmPic1=NULL;
ID2D1Bitmap* UnderLine = NULL;
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	WNDCLASSEX wnd = { NULL };
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wnd.hCursor = LoadCursor(0, IDC_ARROW);
	wnd.hIcon = LoadIcon(0, IDC_ICON);
	wnd.hInstance = hInstance;
	wnd.lpfnWndProc = MainWndProc;
	wnd.lpszClassName = TEXT("GalEngine Blank Win32 Window");
	wnd.style = CS_VREDRAW || CS_HREDRAW;
	RegisterClassEx(&wnd);
	AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, false);
	MainHwnd = CreateWindow(wnd.lpszClassName, TEXT("MY MusicEngine By JackMyth"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, NULL, NULL, hInstance, NULL);
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &D2D1Factory);
	D2D1Factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(MainHwnd, D2D1::SizeU(405, 720)), &D2D1HwndRenderTarget);
	CoInitialize(NULL);
	CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&WICImagingFactory));
	InitGame(lpCmdLine);
	SetTimer(MainHwnd, 0, 0.03, NULL);
	ShowWindow(MainHwnd, nShowCmd);
	MSG msg = { NULL };
	while (msg.message != WM_QUIT)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		GetMessage(&msg, 0, 0, 0);
	}
	return 0;
}

void InitGame(LPSTR lpCmdLine)
{
	MCIERROR M;
	if (!strlen(lpCmdLine))
		exit(0);
	char PathA[256];
	char MusicCMD[300];
	int RhythmCount;
	strcpy(PathA, lpCmdLine);
	strcat(PathA, ".cfg");
	FILE* RhythmConfig = fopen(PathA, "r");
	fscanf(RhythmConfig, "%s", PathA);//音乐标题
	fscanf(RhythmConfig, "%s", PathA);//音乐文件
	sprintf(MusicCMD, "open %s alias bgm", PathA);
	mciSendStringA(MusicCMD,NULL,0,NULL);
	//M=mciSendStringA("open Hellohello.mp3 alias bgm", NULL, 0, NULL);
	//MusicHwnd = MCIWndCreateA(MainHwnd, GetModuleHandle(NULL), WS_CHILD, PathA);
	fscanf(RhythmConfig, "%s", PathA);//背景文件
	fscanf(RhythmConfig, "%d", &RhythmCount);//音符总数
	for (int i=0;i<RhythmCount;i++)
	{
		fscanf(RhythmConfig, "%d,%d,%d", &RhythmInfo[i].RhythmTime,&RhythmInfo[i].RhythmPosition,&RhythmInfo[i].RhythmType);//单个音符信息：时间，位置，类型
	}
	LoadD2DImageA(PathA, &BackgroundPic);
	LoadD2DImage(TEXT("R/RhythmPic0.png"), &RhythmPic0);
	LoadD2DImage(TEXT("R/RhythmPic1.png"), &RhythmPic1);
	LoadD2DImage(TEXT("R/UnderLine.png"), &UnderLine);
	M=mciSendString(TEXT("play bgm repeat"), NULL, 0, NULL);
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_TIMER:
		//MusicPosition = (float)(MCIWndGetPosition(MusicHwnd)/MCIWndGetLength(MusicHwnd));
		ProgressRender();
		ProgressCaculate();
		break;
	case WM_KEYDOWN:
	{
		switch (wParam)
		{
		case VK_LEFT:
			ScoreLastNote(LRHYTHEM);
			break;
		case VK_DOWN:
			ScoreLastNote(MRHYTHEM);
			break;
		case VK_RIGHT:
			ScoreLastNote(RRHYTHEM);
		default:
			break;
		}
	}
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
		break;
	}
	return 0;
}

bool LoadD2DImage(WCHAR ImagePath[], ID2D1Bitmap** BitMapMayClear)
{
	IWICBitmap*    m_pWicBitmap = NULL;
	IWICBitmapDecoder* m_pWicDecoder = NULL;
	IWICBitmapFrameDecode* m_pWicFrameDecoder = NULL;
	if (FAILED(WICImagingFactory->CreateDecoderFromFilename(ImagePath, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &m_pWicDecoder)))
		return false;
	m_pWicDecoder->GetFrame(0, &m_pWicFrameDecoder);
	IWICBitmapSource* pWicSource = NULL;
	m_pWicFrameDecoder->QueryInterface(IID_PPV_ARGS(&pWicSource));
	IWICFormatConverter* pCovert = NULL;
	WICImagingFactory->CreateFormatConverter(&pCovert);
	pCovert->Initialize(
		pWicSource,
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		NULL,
		0.f,
		WICBitmapPaletteTypeCustom
	);
	WICImagingFactory->CreateBitmapFromSource(pCovert, WICBitmapCacheOnDemand, &m_pWicBitmap);
	if (*BitMapMayClear)
		(*BitMapMayClear)->Release();
	D2D1HwndRenderTarget->CreateBitmapFromWicBitmap(m_pWicBitmap, NULL, BitMapMayClear);
	pCovert->Release();
	m_pWicDecoder->Release();
	m_pWicFrameDecoder->Release();
	return true;
}

bool LoadD2DImageA(char ImagePath[], ID2D1Bitmap** BitMapMayClear)
{
	WCHAR WImagePath[256];
	mbstowcs(WImagePath, ImagePath, 256);
	return LoadD2DImage(WImagePath,BitMapMayClear);
}

void ProgressRender()
{
	D2D1HwndRenderTarget->BeginDraw();
	D2D1HwndRenderTarget->DrawBitmap(BackgroundPic, D2D1::RectF(0, 0, 405, 720));//背景
	D2D1HwndRenderTarget->DrawBitmap(UnderLine, D2D1::RectF(0, 555, 405, 565));//UnderLine
	char sPosition[256];
	long CurTime;
	mciSendStringA("status bgm position", sPosition, 255, 0);
	CurTime = strtol(sPosition, NULL, 10);
	for (int i=0;i<1000;i++)
	{
		if (RhythmInfo[i].Score!=0)
			continue;
		if (RhythmInfo[i].RhythmTime>CurTime -500&& RhythmInfo[i].RhythmTime<CurTime + 2000&& !RhythmInfo[i].Score)
		{
			switch (RhythmInfo[i].RhythmType)
			{
			case 0:
				D2D1HwndRenderTarget->DrawBitmap(RhythmPic0, D2D1::RectF(130 + RhythmInfo[i].RhythmPosition * 120, -((float)(RhythmInfo[i].RhythmTime - CurTime) / 2000 * 576) + 556, 230 + RhythmInfo[i].RhythmPosition * 120, -((float)(RhythmInfo[i].RhythmTime - CurTime) / 2000 * 576) + 566));
				break;
			case 1:
				D2D1HwndRenderTarget->DrawBitmap(RhythmPic1, D2D1::RectF(130 + RhythmInfo[i].RhythmPosition * 120, -((float)(RhythmInfo[i].RhythmTime - CurTime) / 2000 * 576) + 556, 230 + RhythmInfo[i].RhythmPosition * 120, -((float)(RhythmInfo[i].RhythmTime - CurTime) / 2000 * 576) + 566));
				break;
			}
		}
	}
	D2D1HwndRenderTarget->EndDraw();
}

void ProgressCaculate()
{
	char sPosition[256]; 
	long CurTime;
	mciSendStringA("status bgm position", sPosition, 255, 0); 
	CurTime = strtol(sPosition, NULL, 10);
	for (int i = 0; i < 1000; i++)
	{
		if (RhythmInfo[i].Score != 0)
			continue;
		if (RhythmInfo[i].RhythmType==1&&RhythmInfo[i].RhythmTime<CurTime+31&& RhythmInfo[i].RhythmTime>CurTime - 31)
		{
			switch (RhythmInfo[i].RhythmPosition)
			{
			case LRHYTHEM:
				if (KEY_DOWN(VK_LEFT))
				{
					RhythmInfo[i].Score = 10;
				}
				break;
			case MRHYTHEM:
				if (KEY_DOWN(VK_DOWN))
				{
					RhythmInfo[i].Score = 10;
				}
				break;
			case RRHYTHEM:
				if (KEY_DOWN(VK_RIGHT))
				{
					RhythmInfo[i].Score = 10;
				}
				break;
			default:
				break;
			}
		}
	}
}

void ScoreLastNote(int Position)
{
	char sPosition[256];
	long CurTime;
	mciSendStringA("status bgm position", sPosition, 255, 0);
	CurTime = strtol(sPosition, NULL, 10);
	for (int i = 0; i < 1000; i++)
	{
		if (RhythmInfo[i].Score != 0)
			continue;
		if (RhythmInfo[i].RhythmType == 0 && RhythmInfo[i].RhythmPosition==Position&& RhythmInfo[i].RhythmTime<CurTime + 200 && RhythmInfo[i].RhythmTime>CurTime - 200)
		{
			RhythmInfo[i].Score = 15;
			break;
		}
	}
}