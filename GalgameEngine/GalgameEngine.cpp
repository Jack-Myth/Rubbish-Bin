#include <stdio.h>
#include <windows.h>
#include <locale.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#include <stack>
#include <dshow.h>
#include <map>
#include <vector>
#include <string>
#pragma comment(lib, "WINMM.LIB")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma  comment(lib,"strmiids.lib")

enum Anim_Style
{
	AnimStyle_NoAnim,
	AnimStyle_NormalAnim,
	AnimStyle_FadeIn,
	AnimStyle_FadeOut,
	AnimStyle_FadeOutAndFadeIn,
};

struct WhileInfo
{
	long StreamPosition;
	char expression[200];
};

struct  _ChatStyle
{
	float FontSize;
	int ItalicStyle;
	int ColorR;
	int ColorG;
	int ColorB;
	WCHAR FontName[100];
};

struct IMGInfo
{
	char ImagePath[512];
	ID2D1Bitmap* D2D1Bitmap;
	D2D1_RECT_F RectToDraw;
};

struct BTNInfo
{
	D2D1_RECT_F Rect;			//按钮位置
	long StreamPosition;			//目标代码位置
};

struct  TextInfo
{
	_ChatStyle ChatStyle;
	WCHAR ChatMessage[1000] = { NULL };
	IDWriteTextFormat* D2DTextFormat = NULL;
	ID2D1Brush* D2DBrush = NULL;
	D2D1_RECT_F RectForChat;
};

class AnimClass
{
public:
	Anim_Style AnimStyle;
	virtual void FrameNotify() = 0;
};

class ImageAnimClass :public AnimClass
{
	int AnimState = 0;
	ImageAnimClass** MePoint;
public:
	IMGInfo* PicInfo;
	ImageAnimClass(IMGInfo*, ImageAnimClass**);
	virtual void FrameNotify() override;
};

class ChatAnimClass :public AnimClass
{
	int AnimState = 1000;
	int FrameCounter = 9;
	WCHAR ChatToDisplay[1000];
public:
	ChatAnimClass();
	virtual void FrameNotify() override;
	void ChatChangeNotify();
	bool EndAnim();
};

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void ProcessScript(bool ReadLine);
int StrReplace(char strRes[], char from[], const char to[]);
float CaculateExpression(char expression[]);
void* GetMinPoint(void* A, void* B);
void FindElsePlace();
void FindCloseCase(char BeginCase[], char CloseCase[]);
bool LoadD2DImage(WCHAR ImagePath[], ID2D1Bitmap** BitMapMayClear);
void SaveGame(char FilePath[]);
void LoadGame(char FilePath[]);
void ClearGameState();
HRESULT InitWindowlessVMR(HWND hwndApp, IGraphBuilder*, IVMRWindowlessControl**);
std::string& trim(std::string &s);
void ScanFileUntilZero(FILE* pFile, char* StrBuffer);
std::string ParseVariable(char* Str);

ID2D1Factory* D2D1Factory=NULL;
IDWriteFactory* DWriteFactory = NULL;
ID2D1HwndRenderTarget* D2D1HwndRenderTarget = NULL;
IWICImagingFactory* WICImagingFactory = NULL;
HWND MainHwnd = NULL, MyPlayer=NULL;
FILE* FileHandle = NULL;
ID2D1Bitmap* BackgroundBitmap = NULL;
char BackgroundPath[512] = {NULL};
char BGMPath[512] = { NULL };
IMGInfo ImageInfo[30] = { NULL };
TextInfo ChatInfo = {NULL};
RECT WindowRect = { 0,0,1280,720 };
std::map<std::string, std::string> GameVariables;
//std::stack<std::vector<std::map<std::string, std::string>::iterator>> LocalVariableRef;
std::stack<std::vector<std::string>> LocalVariableRef;
//int GameFlag[100] = { 0 };
//char StrFlag[10][1000] = { 0 };
BTNInfo ButtonInfo[30] = {NULL};
std::stack<WhileInfo> whileStack;
std::stack<long> buttonStack;
std::stack<long> subStack;
ImageAnimClass* ImageAnim[30] = {NULL};
ChatAnimClass ChatAnim;
IGraphBuilder *pGraph = NULL;
IMediaControl *pControl = NULL;
IMediaEvent   *pEvent = NULL;
IMediaSeeking *pSeeking = NULL;
IVMRWindowlessControl* pWindowLessControl = NULL;
int DelayFrameRemain = -1;
int StretchRuler = 768;
bool Videoing = false;
long WindowHeight = 1280, WindowWidth = 720;
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	WNDCLASSEX wnd = { NULL };
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wnd.hCursor = LoadCursor(0, IDC_ARROW);
	wnd.hIcon = LoadIcon(0, IDC_ICON);
	wnd.hInstance = hInstance;
	wnd.lpfnWndProc = MainWndProc;
	wnd.lpszClassName = TEXT("GalEngine Blank Win32 Window"); 
	wnd.style = CS_VREDRAW||CS_HREDRAW;
	RegisterClassEx(&wnd);
	AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, false);
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &D2D1Factory);
	CoInitialize(NULL);
	CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&WICImagingFactory));
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&DWriteFactory));
	MainHwnd = CreateWindow(wnd.lpszClassName, TEXT("My Galgame Engine"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WindowRect.right-WindowRect.left,WindowRect.bottom- WindowRect.top, NULL, NULL, hInstance, NULL);
	setlocale(LC_ALL, "");
	DWriteFactory->CreateTextFormat(TEXT("微软雅黑"), NULL,
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, 20, TEXT("zh-cn"), &ChatInfo.D2DTextFormat);
	ChatInfo.ChatStyle = { 20,0,0,0,0,TEXT("微软雅黑") };
	D2D1HwndRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), (ID2D1SolidColorBrush**)&ChatInfo.D2DBrush);
	ChatInfo.RectForChat = D2D1::RectF(0.1, 0.7, 0.9, 1);
	CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraph);
	pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
	pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
	pGraph->QueryInterface(IID_IMediaSeeking, (void **)&pSeeking);
	InitWindowlessVMR(MainHwnd, pGraph, &pWindowLessControl);
	ProcessScript(false);
	SetTimer(MainHwnd, 0, 0, NULL);
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

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC RePrintDC = NULL;
	HDC MainHDC = NULL;
	PAINTSTRUCT paintStruct = {NULL};
	bool ImageHasBeenAnimed=false;
	switch (message)
	{
	case WM_CREATE:
		FileHandle = fopen("Script.ges", "r");
		if (!FileHandle)
		{
			MessageBox(NULL, TEXT("没有找到脚本文件，退出!"), TEXT("Can not Find Scrpit.ges"), MB_OK);
			exit(-1);
		}
		D2D1Factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM)), D2D1::HwndRenderTargetProperties(hwnd, D2D1::SizeU(1280, 720)), &D2D1HwndRenderTarget);
		break;
	case WM_SIZE:
	{
		int Siz;
		GetClientRect(MainHwnd, &WindowRect);
		if (WindowRect.right - WindowRect.left > WindowRect.bottom - WindowRect.top)
			Siz = WindowRect.bottom - WindowRect.top;
		else
			Siz = WindowRect.right - WindowRect.left;
		if (ChatInfo.D2DTextFormat)
			ChatInfo.D2DTextFormat->Release();
		DWriteFactory->CreateTextFormat(ChatInfo.ChatStyle.FontName, NULL, DWRITE_FONT_WEIGHT_REGULAR, (DWRITE_FONT_STYLE)((bool)(ChatInfo.ChatStyle.ItalicStyle) * 2), DWRITE_FONT_STRETCH_NORMAL, ChatInfo.ChatStyle.FontSize*((float)Siz/(float)StretchRuler), TEXT("zh-cn"), &ChatInfo.D2DTextFormat);
		long VWidth, VHeight;
		RECT SourceR;
		pWindowLessControl->GetNativeVideoSize(&VWidth, &VHeight, NULL, NULL);
		SourceR = { 0,0,VWidth,VHeight };
		pWindowLessControl->SetVideoPosition(&SourceR, &WindowRect);
		break;
	}
	case WM_TIMER:
		if (!Videoing)
		{
			D2D1_RECT_F rc;
			GetClientRect(hwnd, &WindowRect);
			D2D1HwndRenderTarget->Resize(D2D1::SizeU(WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top));
			D2D1HwndRenderTarget->BeginDraw();
			//////////////////////////////////////////////////////////////////////////Direct2D绘图
			if (BackgroundBitmap)
			{
				D2D1HwndRenderTarget->DrawBitmap(BackgroundBitmap, D2D1::RectF(0, 0, D2D1HwndRenderTarget->GetPixelSize().width, D2D1HwndRenderTarget->GetPixelSize().height), 1,
					D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, D2D1::RectF(0, 0, BackgroundBitmap->GetPixelSize().width, BackgroundBitmap->GetPixelSize().height));
			}
			ImageHasBeenAnimed = false;
			for (int i = 0; i < 30; i++)
			{
				if (ImageInfo[i].D2D1Bitmap)
				{
					for (int a = 0; a < 30; a++)
					{
						if (ImageAnim[a])
						{
							if (ImageAnim[a]->PicInfo == &ImageInfo[i])
							{
								ImageHasBeenAnimed = true;
								ImageAnim[a]->FrameNotify();
								break;
							}
						}
					}
					if (ImageHasBeenAnimed)
					{
						ImageHasBeenAnimed = false;
						continue;
					}
					rc.bottom = ImageInfo[i].RectToDraw.bottom*D2D1HwndRenderTarget->GetPixelSize().height;
					rc.top = ImageInfo[i].RectToDraw.top*D2D1HwndRenderTarget->GetPixelSize().height;
					rc.left = ImageInfo[i].RectToDraw.left*D2D1HwndRenderTarget->GetPixelSize().width;
					rc.right = ImageInfo[i].RectToDraw.right*D2D1HwndRenderTarget->GetPixelSize().width;
					//////////////////////////////////////////////////////////////////////////Direct2D绘图
					D2D1HwndRenderTarget->DrawBitmap(ImageInfo[i].D2D1Bitmap, rc, 1,
						D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, D2D1::RectF(0, 0, ImageInfo[i].D2D1Bitmap->GetPixelSize().width, ImageInfo[i].D2D1Bitmap->GetPixelSize().height));
				}
			}
			ChatAnim.FrameNotify();
			D2D1HwndRenderTarget->EndDraw();
		}
		else
		{
			LONGLONG Cur, Dur;
			pSeeking->GetCurrentPosition(&Cur);
			pSeeking->GetDuration(&Dur);
			if (Cur >= Dur)
			{
				Videoing = false;
				ProcessScript(false);
			}
		}
		if (DelayFrameRemain == 0)
		{
			DelayFrameRemain--;
			ProcessScript(false);
		}
		else if (DelayFrameRemain > 0)
			DelayFrameRemain--;
		//ReleaseDC(MainHwnd, MainHDC);
		break;
	case WM_LBUTTONDOWN:
		if (ChatAnim.EndAnim())
			break;
		POINT ScreenP;
		ScreenP.x= LOWORD(lParam);
		ScreenP.y = HIWORD(lParam);
		//ScreenToClient(MainHwnd, &ScreenP);
		for (BTNInfo BI:ButtonInfo)
		{
			if (!BI.StreamPosition)
				continue;
			if (ScreenP.x > BI.Rect.left*D2D1HwndRenderTarget->GetPixelSize().width&&
				ScreenP.x<BI.Rect.right*D2D1HwndRenderTarget->GetPixelSize().width&&
				ScreenP.y>BI.Rect.top*D2D1HwndRenderTarget->GetPixelSize().height&&
				ScreenP.y < BI.Rect.bottom*D2D1HwndRenderTarget->GetPixelSize().height)
			{
				buttonStack.push(ftell(FileHandle));
				fseek(FileHandle, BI.StreamPosition, SEEK_SET);
				break;
			}
		}
		ProcessScript(false);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SYSCOMMAND:
		DefWindowProc(hwnd, message, wParam, lParam);
		if (wParam==SC_RESTORE)
			SetWindowPos(MainHwnd, HWND_TOP, 0, 0,WindowWidth,WindowHeight, SWP_NOMOVE);
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
		break;
	}
	return 0;
}

void ProcessScript(bool ReadLine)
{
	char Command[500] = { NULL };
	char ArgStr[10][512] = { NULL };
	char ArgLongStr[1000] = { NULL };
	WCHAR WCharTMP[1000] = { NULL };
	long LastStreamIndex;
	do 
	{
		if ((Command[0] = fgetc(FileHandle)) != '\n'&&Command[0] != ' '&&Command[0] != '\t')
			fseek(FileHandle, -1, SEEK_CUR);
		else 
			continue;
		if (fgetc(FileHandle) != '#')
			fseek(FileHandle, -1, SEEK_CUR);
		else
		{
			fseek(FileHandle, -1, SEEK_CUR);
			fscanf(FileHandle,"%*[^\n]");
			continue;
		}
		LastStreamIndex = ftell(FileHandle);
		fscanf(FileHandle,"%[^(]", Command);
		if (!strcmp(Command, "settitle"))
		{
			fscanf(FileHandle,"(%[^)])", ArgStr[0]);
			SetWindowTextA(MainHwnd, ParseVariable(ArgStr[0]).c_str());
		} 
		else if (!strcmp(Command, "setbackground"))
		{
			fscanf(FileHandle,"(%[^)])", ArgStr[0]);
			mbstowcs(WCharTMP, ParseVariable(ArgStr[0]).c_str(), 1000);
			if (LoadD2DImage(WCharTMP, &BackgroundBitmap))
				strcpy(BackgroundPath, ParseVariable(ArgStr[0]).c_str());
		} 
		else if (!strcmp(Command, "setchat"))
		{
			fscanf(FileHandle, "(%[^,],%[^)])", ArgLongStr,ArgStr[0]);
			strcpy(ArgLongStr,ParseVariable(ArgLongStr).c_str());
			StrReplace(ArgLongStr, "\\n", "\n");
			mbstowcs(ChatInfo.ChatMessage, ArgLongStr, 1000);
			ChatAnim.AnimStyle = (Anim_Style)(int)CaculateExpression(ArgStr[0]);
			ChatAnim.ChatChangeNotify();
		}
		else if (!strcmp(Command, "setimage"))
		{
			fscanf(FileHandle, "(%[^,],%[^,],%[^,],%[^,],%[^,],%[^)])", ArgStr[0], ArgStr[1], ArgStr[2], ArgStr[3],ArgStr[4], ArgStr[5]);
			float left = CaculateExpression(ArgStr[1]);
			float top = CaculateExpression(ArgStr[2]);
			float right = CaculateExpression(ArgStr[3]);
			float buttom = CaculateExpression(ArgStr[4]);
			int ImageLayer= CaculateExpression(ArgStr[5]);
			if (!(ImageLayer >=0 && ImageLayer < 30))
				continue;
			mbstowcs(WCharTMP, ParseVariable(ArgStr[0]).c_str(), 1000);
			for (int i = 0; i < 30; i++)
			{
				if (ImageAnim[i] && ImageAnim[i]->PicInfo == &ImageInfo[ImageLayer])
				{
					delete ImageAnim[i];
					ImageAnim[i] = NULL;
					break;
				}
			}
			if(LoadD2DImage(WCharTMP, &ImageInfo[ImageLayer].D2D1Bitmap))
				strcpy(ImageInfo[ImageLayer].ImagePath, ParseVariable(ArgStr[0]).c_str());
			ImageInfo[ImageLayer].RectToDraw.left = left;
			ImageInfo[ImageLayer].RectToDraw.top = top;
			ImageInfo[ImageLayer].RectToDraw.right = right;
			ImageInfo[ImageLayer].RectToDraw.bottom = buttom;
		}
		else if (!strcmp(Command, "setimageA"))
		{
			fscanf(FileHandle, "(%[^,],%[^,],%[^,],%[^,],%[^,],%[^,],%[^)])", ArgStr[0], ArgStr[1], ArgStr[2], ArgStr[3], ArgStr[4], ArgStr[5], ArgStr[6]);
			float left = CaculateExpression(ArgStr[1]);
			float top = CaculateExpression(ArgStr[2]);
			float right = CaculateExpression(ArgStr[3]);
			float buttom = CaculateExpression(ArgStr[4]);
			int ImageLayer = CaculateExpression(ArgStr[5]);
			if (!(ImageLayer >= 0 && ImageLayer < 30))
				continue;
			mbstowcs(WCharTMP, ParseVariable(ArgStr[0]).c_str(), 1000);
			ImageAnimClass** IAC=NULL;
			for (int i = 0; i < 30; i++)
			{
				if (ImageAnim[i] && ImageAnim[i]->PicInfo == &ImageInfo[ImageLayer])
				{
					IAC = &ImageAnim[i];
					break;
				}
			}
			if (!IAC)
				for (int i = 0; i < 30; i++)
				{
					if (!ImageAnim[i])
					{
						IAC = &ImageAnim[i];
						break;
					}
				}
			switch ((int)CaculateExpression(ArgStr[6]))
			{
			case 0: case 1: case 2: case 3:
				if (IAC)
				{
					delete *IAC;
					if (LoadD2DImage(WCharTMP, &ImageInfo[ImageLayer].D2D1Bitmap))
						strcpy(ImageInfo[ImageLayer].ImagePath, ParseVariable(ArgStr[0]).c_str());
					*IAC = new ImageAnimClass(&ImageInfo[ImageLayer], IAC);
					(*IAC)->AnimStyle = (Anim_Style)(int)CaculateExpression(ArgStr[6]);
				}
				break;
			case 4:
				if (IAC)
				{
					delete *IAC;
					strcpy(ImageInfo[ImageLayer].ImagePath, ParseVariable(ArgStr[0]).c_str());
					*IAC = new ImageAnimClass(&ImageInfo[ImageLayer], IAC);
					(*IAC)->AnimStyle = (Anim_Style)(int)CaculateExpression(ArgStr[6]);
				}
				break;
			default:
				break;
			}
			ImageInfo[ImageLayer].RectToDraw.left = left;
			ImageInfo[ImageLayer].RectToDraw.top = top;
			ImageInfo[ImageLayer].RectToDraw.right = right;
			ImageInfo[ImageLayer].RectToDraw.bottom = buttom;
		}
		else if (!strcmp(Command, "clearimage"))
		{
			fscanf(FileHandle, "(%[^)])", ArgStr[0]);
			int ImageLayer = (int)CaculateExpression(ArgStr[0]);
			if (!(ImageLayer >= 0 && ImageLayer < 30))
				continue;
			for (int i=0;i<30;i++)
			{
				if (ImageAnim[i]&&ImageAnim[i]->PicInfo==&ImageInfo[ImageLayer])
				{
					delete ImageAnim[i];
					ImageAnim[i] = NULL;
					break;
				}
			}
			if (ImageInfo[ImageLayer].D2D1Bitmap)
			{
				ImageInfo[ImageLayer].D2D1Bitmap->Release();
				ImageInfo[ImageLayer].D2D1Bitmap = NULL;
			}
		}
		else if (!strcmp(Command, "setchatrect"))
		{
			fscanf(FileHandle, "(%[^,],%[^,],%[^,],%[^)])", ArgStr[0], ArgStr[1], ArgStr[2], ArgStr[3]);
			ChatInfo.RectForChat.left = CaculateExpression(ArgStr[0]);
			ChatInfo.RectForChat.top = CaculateExpression(ArgStr[1]);
			ChatInfo.RectForChat.right = CaculateExpression(ArgStr[2]);
			ChatInfo.RectForChat.bottom = CaculateExpression(ArgStr[3]);
		} 
		else if (!strcmp(Command, "setchatstyle"))				//设置对话文字风格。参数：【字体名称，字体大小，是否斜体(1或0)，文字颜色(三个参数对应RGB)】
		{
			IDWriteTextFormat* Tmp = ChatInfo.D2DTextFormat;
			ID2D1Brush* BrushTmp = ChatInfo.D2DBrush;
			fscanf(FileHandle, "(%[^,],%[^,],%[^,],%[^,],%[^,],%[^)])", ArgStr[0],&ArgStr[1],&ArgStr[2], &ArgStr[3], &ArgStr[4], &ArgStr[5]);
			mbstowcs(WCharTMP,ParseVariable(ArgStr[0]).c_str(), 1000);
			int Siz;
			GetClientRect(MainHwnd, &WindowRect);
			if (WindowRect.right - WindowRect.left > WindowRect.bottom - WindowRect.top)
				Siz = WindowRect.bottom - WindowRect.top;
			else
				Siz = WindowRect.right - WindowRect.left;
			DWriteFactory->CreateTextFormat(WCharTMP, NULL, DWRITE_FONT_WEIGHT_REGULAR, (DWRITE_FONT_STYLE)((bool)((int)CaculateExpression(ArgStr[2])) * 2), DWRITE_FONT_STRETCH_NORMAL, CaculateExpression(ArgStr[1])* ((float)Siz / (float)StretchRuler), TEXT("zh-cn"), &ChatInfo.D2DTextFormat);
			if (ChatInfo.D2DTextFormat)
			{
				Tmp->Release();
				wcscpy(ChatInfo.ChatStyle.FontName, WCharTMP);
				ChatInfo.ChatStyle.FontSize = CaculateExpression(ArgStr[1]);
				ChatInfo.ChatStyle.ItalicStyle = CaculateExpression(ArgStr[2]);
			}
			else
				ChatInfo.D2DTextFormat = Tmp;
			int ColorR = CaculateExpression(ArgStr[3]);
			int ColorG = CaculateExpression(ArgStr[4]);
			int ColorB = CaculateExpression(ArgStr[5]);
			D2D1HwndRenderTarget->CreateSolidColorBrush(D2D1::ColorF(RGB(ColorR, ColorG, ColorB)), (ID2D1SolidColorBrush**)&ChatInfo.D2DBrush);
			if (ChatInfo.D2DBrush)
			{
				BrushTmp->Release();
				ChatInfo.ChatStyle.ColorR = ColorR;
				ChatInfo.ChatStyle.ColorG = ColorG;
				ChatInfo.ChatStyle.ColorB = ColorB;
			}
			else
				ChatInfo.D2DBrush = BrushTmp;
		}
		else if (!strcmp(Command, "playbgm"))
		{
			fscanf(FileHandle, "(%[^,],%[^)])", ArgStr[0],ArgStr[1]);
			std::string MusicPath = ParseVariable(ArgStr[0]);
			sprintf(ArgStr[2], "open %s alias bgm", MusicPath.c_str());
			mciSendString(TEXT("close bgm"), NULL, 0, NULL);
			mciSendStringA(ArgStr[2], NULL, 0, NULL);
			if (!CaculateExpression(ArgStr[1]))
				mciSendString(TEXT("play bgm"), NULL, 0, NULL);
			else
				mciSendString(TEXT("play bgm repeat"), NULL, 0, NULL);
			strcpy(BGMPath, MusicPath.c_str());
		} 
		else if (!strcmp(Command, "playvideo"))
		{
			long VHeight, VWidth;
			LONGLONG DuratioTmp;
			fscanf(FileHandle, "(%[^)])", ArgStr[0]);
			mbstowcs(WCharTMP, ParseVariable(ArgStr[0]).c_str(), 200);
			pGraph->RenderFile(WCharTMP, NULL);
			RECT SourceR;
			pWindowLessControl->GetNativeVideoSize(&VWidth, &VHeight, NULL, NULL);
			SourceR = { 0,0,VWidth,VHeight };
			GetClientRect(MainHwnd, &WindowRect);
			pWindowLessControl->SetVideoPosition(&SourceR, &WindowRect);
			pControl->Run();
			Videoing = true;
			break;
		}
		else if (!strcmp(Command, "stopvideo"))
		{
			fscanf(FileHandle, "%*s");
			Videoing = false;
			pControl->Stop();
		}
		else if (!strcmp(Command, "playvoice"))
		{
			fscanf(FileHandle, "(%[^)])", ArgStr[0]);
			//PlaySoundA(NULL, NULL, NULL);
			PlaySoundA(ParseVariable(ArgStr[0]).c_str(), NULL, SND_FILENAME| SND_ASYNC);
		}
		else if (!strcmp(Command, "stopvoice"))
		{
			fscanf(FileHandle, "%*s");
			PlaySoundA(NULL,NULL,NULL);
		}
		else if (!strcmp(Command, "stopbgm"))
		{
			fscanf(FileHandle, "%*s");
			mciSendString(TEXT("close bgm"), NULL, 0, MainHwnd);
			strcpy(BGMPath, "");
		} 
		else if (!strcmp(Command, "waitclick"))
		{
			fscanf(FileHandle, "(%*d)");
			//if (ArgInt[0] != 1)
			//	SendMessage(MainHwnd, WM_PAINT, 0, 0);
			DelayFrameRemain = -1;
			break;
		} 
		else if (!strcmp(Command, "button"))
		{
			fscanf(FileHandle, "(%[^,],%[^,],%[^,],%[^,],%[^)])", ArgStr[0], ArgStr[1], ArgStr[2], ArgStr[3], ArgStr[4]);
			int ButtonID = CaculateExpression(ArgStr[4]);
			if (!(ButtonID<30&& ButtonID >=0))
				continue;
			ButtonInfo[ButtonID].StreamPosition = ftell(FileHandle);
			ButtonInfo[ButtonID].Rect = D2D1::RectF(CaculateExpression(ArgStr[0]), CaculateExpression(ArgStr[1]), CaculateExpression(ArgStr[2]), CaculateExpression(ArgStr[3]));
			FindCloseCase("button", "endbutton");
		}
		else if (!strcmp(Command, "endbutton"))		
		{
			fscanf(FileHandle, "%*[^\n]");
			if (buttonStack.empty())
				continue;
			fseek(FileHandle, buttonStack.top(), SEEK_SET);
			buttonStack.pop();
		}
		else if (!strcmp(Command, "removebutton"))
		{
			fscanf(FileHandle, "(%[^)])", ArgStr[0]);
			int ButtonID = CaculateExpression(ArgStr[4]);
			if (!(ButtonID < 30 && ButtonID >= 0))
				continue;
			ButtonInfo[ButtonID].StreamPosition = 0;
		}
		else if (!strcmp(Command, "clearbutton"))
		{
			fscanf(FileHandle, "%*[^\n]");
			for (BTNInfo& BI : ButtonInfo)
				BI.StreamPosition = 0;
		}
		else if (!strcmp(Command, "save"))
		{
			fscanf(FileHandle, "(%[^)])",ArgStr[0]);
			sprintf(ArgStr[0], "%d.save", (int)CaculateExpression(ArgStr[0]));
			FILE* SaveFile = fopen(ArgStr[0],"r");
			if (SaveFile)
			{
				fclose(SaveFile);
				if (MessageBox(MainHwnd, TEXT("存档已存在，是否覆盖？"), TEXT("存档已存在"), MB_YESNO) == IDNO)
				{
					fclose(SaveFile);
					continue;
				}
			}
			SaveGame(ArgStr[0]);
		}
		else if (!strcmp(Command, "load"))
		{
			fscanf(FileHandle, "(%[^)])", ArgStr[0]);
			sprintf(ArgStr[0], "%d.save", (int)CaculateExpression(ArgStr[0]));
			LoadGame(ArgStr[0]);
		}
		else if (!strcmp(Command, "setwindowsize"))
		{
			fscanf(FileHandle, "(%[^,],%[^)])", ArgStr[0], ArgStr[1]);
			WindowWidth = CaculateExpression(ArgStr[0]);
			WindowHeight = CaculateExpression(ArgStr[1]);
			SetWindowPos(MainHwnd, HWND_TOP, 0, 0, WindowWidth, WindowHeight, SWP_NOMOVE);
			if (WindowWidth > WindowHeight)
				StretchRuler = WindowHeight;
			else
				StretchRuler = WindowWidth;
		}
		else if (!strcmp(Command, "setflag"))////////////////////////////////////////////////////////////////////非功能逻辑控制语句///////////////////////////////////////////////////////////////////////////
		{
			fscanf(FileHandle, "(%[^,],%[^)])", ArgStr[0],ArgStr[1]);
			GameVariables[trim(std::string(ArgStr[0]))] = ParseVariable(ArgStr[1]);
		}
		//else if (!strcmp(Command, "setstrflag"))
		//{
		//	fscanf(FileHandle, "(%d,%[^)])", &ArgInt[0], ArgLongStr);
		//	for (int i = 0; i < 10; i++)
		//	{
		//		sprintf(ArgStr[1], "str[%d]", i);
		//		StrReplace(ArgLongStr, ArgStr[1], StrFlag[i]);
		//	}
		//	if (ArgInt[0] < 10 && ArgInt[0] >= 0)
		//		strcpy(StrFlag[ArgInt[0]], ArgLongStr);
		//}
		else if (!strcmp(Command, "if"))
		{
			fscanf(FileHandle, "(%[^)])", ArgStr[0]);
			if (!CaculateExpression(ArgStr[0]))
			{
				FindElsePlace();
			}
		}
		else if (!strcmp(Command, "else"))
		{
			fscanf(FileHandle, "%*[^\n]");
			FindElsePlace();
		}
		else if (!strcmp(Command, "while"))
		{
			fscanf(FileHandle, "(%[^)])", ArgStr[0]);
			if (CaculateExpression(ArgStr[0]))
			{
				WhileInfo WI;
				WI.StreamPosition = ftell(FileHandle);
				strcpy(WI.expression, ArgStr[0]);
				whileStack.push(WI);
			}
			else
				FindCloseCase("while","loop");
		}
		else if (!strcmp(Command, "loop"))
		{
			fscanf(FileHandle, "%*[^\n]");
			if (whileStack.empty())
				continue;
			if (CaculateExpression(whileStack.top().expression))
				fseek(FileHandle, whileStack.top().StreamPosition, SEEK_SET);
			else
				whileStack.pop();
		}
		else if (!strcmp(Command, "goto"))
		{
			fscanf(FileHandle, "(%[^)])",ArgStr[0]);
			long SteamCurrent = ftell(FileHandle);
			fseek(FileHandle, 0, SEEK_SET);
			while (!feof(FileHandle))
			{
				if(fgetc(FileHandle) != '#')
					continue;
				fscanf(FileHandle, "%s", ArgStr[1]);
				if (!strcmp(ArgStr[0],ArgStr[1]))
				{
					while (!whileStack.empty())
						whileStack.pop();
					while (!buttonStack.empty())
						buttonStack.pop();
					while (!subStack.empty())
						subStack.pop();
					SteamCurrent = ftell(FileHandle);
					break;
				}
			}
			fseek(FileHandle, SteamCurrent, SEEK_SET);
		}
		else if (!strcmp(Command, "delay"))
		{
			fscanf(FileHandle, "(%[^)])", &ArgStr[0]);
			DelayFrameRemain = (int)(CaculateExpression(ArgStr[0]) * 30);
			break;
		}
		else if (!strcmp(Command, "endsub"))
		{
			fscanf(FileHandle, "%*s");
			fseek(FileHandle, subStack.top(), SEEK_SET);
			subStack.pop();
			for (auto it=LocalVariableRef.top().begin();it<LocalVariableRef.top().end();++it)
			{
				GameVariables.erase(*it);
			}
			LocalVariableRef.pop();
		}
		else if (!strcmp(Command, "end"))
		{
			exit(0);
		}
		else
		{
			fscanf(FileHandle, "(%[^)])", &ArgStr[0]);
			ArgStr[0][strlen(ArgStr[0])+1] = 0;
			ArgStr[0][strlen(ArgStr[0])] = ',';
			subStack.push(ftell(FileHandle));
			fseek(FileHandle, 0, SEEK_SET);
			std::string subName = std::string("sub ") + Command + "(";
			while (!feof(FileHandle))
			{
				char tmpCommandLine[1024];
				fscanf(FileHandle, "%[^\n]\n", tmpCommandLine);
				std::string tmpCommandStr(tmpCommandLine);
				tmpCommandStr.erase(0, tmpCommandStr.find_first_not_of(' '));
				int subPosition = tmpCommandStr.find(subName);
				if (subPosition == 0)
				{
					tmpCommandStr.erase(0, subName.length());
					LocalVariableRef.push(std::vector<std::string>());
					char* tmpArgs = ArgStr[0];
					while (1)
					{
						char FormalArg[512] = {0};
						char ActualArg[512] = { 0 };
						sscanf(tmpCommandStr.c_str(), "%[^,)]", FormalArg);
						tmpCommandStr.erase(0, strlen(FormalArg));
						sscanf(tmpArgs, "%[^,)]", ActualArg);
						tmpArgs += strlen(ActualArg)+1;
						std::string FormalArgStr = trim(std::string(FormalArg));
						if (GameVariables.find(FormalArgStr)!=GameVariables.end())
						{
							char ErrorMessage[600];
							sprintf(ErrorMessage, "Variable Already Exist: %s", FormalArgStr.c_str());
							MessageBoxA(MainHwnd, ErrorMessage, "Script Error", MB_OK);
							LocalVariableRef.pop();
							break;
						}
						GameVariables[FormalArgStr] = std::string(ActualArg);
						LocalVariableRef.top().push_back(FormalArgStr);
						if (tmpCommandStr[0] == ')')
						{
							subStack.push(ftell(FileHandle));
							break;
						}
						tmpCommandStr.erase(0, 1);
					}
					break;
				}
			}
			fseek(FileHandle, subStack.top(), SEEK_SET);
			subStack.pop();
			//
			//
			//long FileCurrent,SubCurrent;
			////fscanf(FileHandle, "%*s");
			//FileCurrent = ftell(FileHandle);
			////strcpy(ArgStr[0], Command);
			//bool Findway = false;
			//fseek(FileHandle, 0, SEEK_SET);
			//while (!feof(FileHandle))
			//{
			//	if ((ArgStr[0][0] = fgetc(FileHandle)) != '\n'&&ArgStr[0][0] != ' '&&ArgStr[0][0] != '\t')
			//		fseek(FileHandle, -1, SEEK_CUR);
			//	else
			//		continue;
			//	if (fgetc(FileHandle) != '#')
			//		fseek(FileHandle, -1, SEEK_CUR);
			//	else
			//	{
			//		fseek(FileHandle, -1, SEEK_CUR);
			//		fscanf(FileHandle, "%*s");
			//		continue;
			//	}
			//	fscanf(FileHandle, "%s", ArgStr[0]);
			//	if (!strcmp(ArgStr[0], "sub"))
			//	{
			//		while (fgetc(FileHandle) == ' ');
			//		fseek(FileHandle, -1, SEEK_CUR);
			//		fscanf(FileHandle, "%[^(]", ArgStr[0]);
			//		if (!strcmp(ArgStr[0], Command))
			//		{
			//			fscanf(FileHandle, "%*s");
			//			SubCurrent = ftell(FileHandle);
			//			Findway = true;
			//			break;
			//		}
			//	}
			//}
			//fseek(FileHandle, FileCurrent, SEEK_SET);
			//if (!Findway)
			//{
			//	fscanf(FileHandle, "%*s");
			//	continue;
			//}
			//fscanf(FileHandle, "(%d", &ArgInt[0]);
			//while (fgetc(FileHandle)!= ')')
			//{
			//	fscanf(FileHandle, "%[^,)]", ArgLongStr);
			//	strcpy(StrFlag[ArgInt[0]], ArgLongStr);
			//	ArgInt[0]++;
			//}
			//subStack.push(ftell(FileHandle));
			//fseek(FileHandle, SubCurrent, SEEK_SET);
		 }
	} while (!ReadLine);
}

int StrReplace(char strRes[], char from[], const char to[]) 
{
	int i, flag = 0;
	char *p, *q, *ts;
	for (i = 0; strRes[i]; ++i) 
	{
		if (strRes[i] == from[0]) 
		{
			p = strRes + i;
			q = from;
			while (*q && (*p++ == *q++));
			if (*q == '\0') 
			{
				ts = (char *)malloc(strlen(strRes) + 1);
				strcpy(ts, p);
				strRes[i] = '\0';
				strcat(strRes, to);
				strcat(strRes, ts);
				free(ts);
				flag = 1;
			}
		}
	}
	return flag;
}

float CaculateExpression(char Originexpression[])
{
	void InfixToPostfix(char* InfixExpression, char* PostfixExpression, int BufferSize);
	float CaculatePostfixExpression(char* PostfixExpression);
	char expression[1024];
	strcpy(expression, Originexpression);
	for (int i=0;i<strlen(expression);i++)
	{
		if (expression[i]=='[')
			for (int a=i+1;a<strlen(expression);a++)
			{
				if (expression[a] == ']')
				{
					char VariableName[512] = {0};
					memcpy(VariableName, expression + i+1, a - i - 1);
					char tmpStrToReplace[514];
					sprintf(tmpStrToReplace, "[%s]", VariableName);
					auto it = GameVariables.find(std::string(VariableName));
					if (it != GameVariables.end())
					{
						StrReplace(expression, tmpStrToReplace, it->second.c_str());
					}
					else
					{
						char ErrorMessage[600];
						sprintf(ErrorMessage, "Undefinded Variable %s", VariableName);
						MessageBoxA(MainHwnd, ErrorMessage, "Script Warning", MB_OK);
						StrReplace(expression, tmpStrToReplace, "");
					}
					i = a;
					break;
				}
			}
	}
	char PostFix[1024];
	InfixToPostfix(expression, PostFix,1024);
	return CaculatePostfixExpression(PostFix);
}

std::string ParseVariable(char* Str)
{
	char* VariableTmp = Str;
	std::string VariableName;
	while (*VariableTmp ==' '&&*VariableTmp)
	{
		VariableTmp++;
	}
	if (*VariableTmp != '[')
		return std::string(Str);
	VariableTmp++;
	int VariableNameLength = 0;
	while (VariableTmp+VariableNameLength)
	{
		if(*(VariableTmp+VariableNameLength)==']')
		{
			VariableName = std::string(VariableTmp, VariableNameLength);
			break;
		}
		VariableNameLength++;
	}
	auto it = GameVariables.find(VariableName);
	if (it != GameVariables.end())
		return it->second.c_str();
	return std::string(Str);
}

void* GetMinPoint(void* A, void* B)
{
	if ((long)A == 0)
		return B;
	if ((long)B == 0)
		return A;
	if ((long)A > (long)B)
		return B;
	else
		return A;
}

void FindElsePlace()
{
	char Command[50] = { NULL };
	int IfStack = 0;
	while (1)
	{
		if ((Command[0] = fgetc(FileHandle)) != '\n'&&Command[0] != ' '&&Command[0] != '\t')
			fseek(FileHandle, -1, SEEK_CUR);
		else
			continue;
		if (fgetc(FileHandle) != '#')
			fseek(FileHandle, -1, SEEK_CUR);
		else
		{
			fseek(FileHandle, -1, SEEK_CUR);
			fscanf(FileHandle, "%*s");
			continue;
		}
		fscanf(FileHandle, "%[^(]", Command);
		if (!strcmp(Command, "endif"))
		{
			IfStack--;
			if (IfStack < 0)
			{
				fscanf(FileHandle, "%*s");
				return;
			}
		} else if(!strcmp(Command, "else"))
		{
			if (IfStack <= 0)
			{
				fscanf(FileHandle, "%*s");
				return;
			}
		} else if (!strcmp(Command, "if"))
		{
			IfStack++;
		}
		fscanf(FileHandle, "%*[^\n]");
	}
}

void FindCloseCase(char BeginCase[],char CloseCase[])
{
	char Command[50] = { NULL };
	int IfStack = 0;
	while (1)
	{
		if ((Command[0] = fgetc(FileHandle)) != '\n'&&Command[0] != ' '&&Command[0] != '\t')
			fseek(FileHandle, -1, SEEK_CUR);
		else
			continue;
		if (fgetc(FileHandle) != '#')
			fseek(FileHandle, -1, SEEK_CUR);
		else
		{
			fseek(FileHandle, -1, SEEK_CUR);
			fscanf(FileHandle, "%*s");
			continue;
		}
		fscanf(FileHandle, "%[^(]", Command);
		if (!strcmp(Command, CloseCase))
		{
			IfStack--;
			if (IfStack < 0)
			{
				fscanf(FileHandle, "%*s");
				return;
			}
		}
		else if (!strcmp(Command, BeginCase))
		{
			IfStack++;
		}
		fscanf(FileHandle, "%*[^\n]");
	}
}

bool LoadD2DImage(WCHAR ImagePath[],ID2D1Bitmap** BitMapMayClear)
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

//储存游戏进度
void SaveGame(char FilePath[])
{
	FILE* SaveGameFile = fopen(FilePath, "wb+");
	if (!SaveGameFile)
		return;
	WhileInfo* TmpWhileinfo = new WhileInfo[whileStack.size()];
	int Size = whileStack.size();
	fprintf(SaveGameFile, "%c", (char)Size);
	for (int i=0;i<Size;i++)
	{
		TmpWhileinfo[i] = whileStack.top();
		whileStack.pop();
	}
	fwrite(TmpWhileinfo, sizeof(WhileInfo), Size, SaveGameFile);
	for (int i = 0; i < Size; i++)
		whileStack.push(TmpWhileinfo[Size - i - 1]);
	delete[] TmpWhileinfo;
	Size = buttonStack.size();
	fprintf(SaveGameFile, "%c", (char)Size);
	long* BTNSTmp = new long[Size];
	for (int i = 0; i < Size; i++)
	{
		BTNSTmp[i] = buttonStack.top();
		buttonStack.pop();
	}
	fwrite(BTNSTmp, sizeof(long), Size, SaveGameFile);
	for (int i = 0; i < Size; i++)
		buttonStack.push(BTNSTmp[Size - i - 1]);
	delete[] BTNSTmp;
	fwrite(ButtonInfo, sizeof(BTNInfo), 30, SaveGameFile);
	short VariableCount = GameVariables.size();
	fwrite(&VariableCount, sizeof(short), 1, SaveGameFile);
	for (auto it=GameVariables.begin();it!=GameVariables.end();++it)
	{
		fwrite(it->first.c_str(), it->first.length(), 1, SaveGameFile);
		fputc(0, SaveGameFile);
		fwrite(it->second.c_str(), it->second.length(), 1, SaveGameFile);
		fputc(0, SaveGameFile);
	}
	{
		short StackSize = LocalVariableRef.size();
		std::stack<std::vector<std::string>> tmpStack;
		fwrite(&StackSize, sizeof(StackSize), 1, SaveGameFile);
		while(LocalVariableRef.size())
		{
			tmpStack.push(LocalVariableRef.top());
			LocalVariableRef.pop();
		}
		while (tmpStack.size())
		{
			LocalVariableRef.push(tmpStack.top());
			short VariableCount = LocalVariableRef.top().size();
			for (int i=0;i<VariableCount;i++)
			{
				fwrite(LocalVariableRef.top()[i].c_str(), LocalVariableRef.top()[i].length(), 1, SaveGameFile);
				fputc(0, SaveGameFile);
			}
			tmpStack.pop();
		}
	}
	/*fwrite(GameFlag, sizeof(int), 100, SaveGameFile);
	fwrite(StrFlag, 1000, 10, SaveGameFile);*/
	fwrite(BackgroundPath,strlen(BackgroundPath), 1, SaveGameFile);
	fputc(0, SaveGameFile);
	fwrite(ImageInfo, sizeof(IMGInfo), 30, SaveGameFile);
	fwrite(BGMPath, strlen(BGMPath), 1, SaveGameFile);
	fputc(0, SaveGameFile);
	long CurrentF = ftell(FileHandle);
	fwrite(&CurrentF, sizeof(long), 1, SaveGameFile);
	fwrite(&ChatInfo, sizeof(TextInfo), 1, SaveGameFile);
	fclose(SaveGameFile);
}

//读取游戏进度
void LoadGame(char FilePath[])
{
	FILE* SaveGameFile = fopen(FilePath, "rb");
	if (!SaveGameFile)
		return;
	ClearGameState();
	char CSize;
	WhileInfo WI = { NULL };
	fscanf(SaveGameFile, "%c", &CSize);
	for (int i=0;i<CSize;i++)
	{
		fread(&WI, sizeof(WhileInfo), 1, SaveGameFile);
		whileStack.push(WI);
	}
	long BTNSTmp = 0;
	fscanf(SaveGameFile, "%c", &CSize);
	for (int i = 0; i < CSize; i++)
	{
		fread(&BTNSTmp, sizeof(long), 1, SaveGameFile);
		buttonStack.push(BTNSTmp);
	}
	fread(ButtonInfo, sizeof(BTNInfo), 30, SaveGameFile);
	{
		short VariableCount;
		fread(&VariableCount, sizeof(VariableCount), 1, SaveGameFile);
		while(VariableCount)
		{
			char tmpChars[1000];
			std::string VariableName;
			ScanFileUntilZero(SaveGameFile, tmpChars);
			VariableName = std::string(tmpChars);
			ScanFileUntilZero(SaveGameFile, tmpChars);
			GameVariables[VariableName] = std::string(tmpChars);
			VariableCount--;
		}
		{
			short StackCount;
			fread(&StackCount, sizeof(StackCount), 1, SaveGameFile);
			while (StackCount)
			{
				short LocalVariableCount;
				fread(&LocalVariableCount, sizeof(LocalVariableCount), 1, SaveGameFile);
				std::vector<std::string> tmpLocalVariableVector;
				while (LocalVariableCount)
				{
					char VariableName[512];
					ScanFileUntilZero(SaveGameFile, VariableName);
					tmpLocalVariableVector.push_back(std::string(VariableName));
					LocalVariableCount--;
				}
				LocalVariableRef.push(tmpLocalVariableVector);
				StackCount--;
			}
		}
	}
	/*fread(GameFlag, sizeof(int), 100, SaveGameFile);
	fread(StrFlag, 1000, 10, SaveGameFile);*/
	ScanFileUntilZero(SaveGameFile, BackgroundPath);
	//fscanf(SaveGameFile, "%[^\n]", BackgroundPath);
	//fread(BackgroundPath, 200, 1, SaveGameFile);
	fread(ImageInfo, sizeof(IMGInfo), 30, SaveGameFile);
	ScanFileUntilZero(SaveGameFile, BGMPath);
	//fscanf(SaveGameFile, "%[^\n]", BGMPath);
	//fread(BGMPath, 200, 1, SaveGameFile);
	long CurrentF;
	fread(&CurrentF, sizeof(long), 1, SaveGameFile);
	fread(&ChatInfo, sizeof(TextInfo), 1, SaveGameFile);
	//重建资源
	WCHAR WCHARTmp[512];
	mbstowcs(WCHARTmp, BackgroundPath, 512);
	BackgroundBitmap = NULL;
	LoadD2DImage(WCHARTmp, &BackgroundBitmap);
	for (int i=0;i<30;i++)
	{
		if (ImageInfo[i].D2D1Bitmap)
		{
			mbstowcs(WCHARTmp, ImageInfo[i].ImagePath, 512);
			ImageInfo[i].D2D1Bitmap = NULL;
			LoadD2DImage(WCHARTmp, &ImageInfo[i].D2D1Bitmap);
		}
	}
	DWriteFactory->CreateTextFormat(ChatInfo.ChatStyle.FontName, NULL, DWRITE_FONT_WEIGHT_REGULAR, 
		(DWRITE_FONT_STYLE)((bool)(ChatInfo.ChatStyle.ItalicStyle) * 2), DWRITE_FONT_STRETCH_NORMAL, ChatInfo.ChatStyle.FontSize, TEXT("zh-cn"), &ChatInfo.D2DTextFormat);
	D2D1HwndRenderTarget->CreateSolidColorBrush(D2D1::ColorF(RGB(ChatInfo.ChatStyle.ColorR, ChatInfo.ChatStyle.ColorG, ChatInfo.ChatStyle.ColorB)), 
		(ID2D1SolidColorBrush**)&ChatInfo.D2DBrush);
	char BGMP[600];
	sprintf(BGMP, "open %s alias bgm", BGMPath);
	mciSendStringA(BGMP, NULL, 0, NULL);
	mciSendString(TEXT("play bgm repeat"), NULL, 0, NULL);
	fseek(FileHandle, CurrentF, SEEK_SET);
	fclose(SaveGameFile);
}

//重置引擎，清理并释放资源
void ClearGameState()
{
	while (!whileStack.empty())
		whileStack.pop();
	while (!buttonStack.empty())
		buttonStack.pop();
	for (int i = 0; i < 30; i++)
	{
		ButtonInfo[i] = { NULL };
		if (ImageInfo[i].D2D1Bitmap)
			ImageInfo[i].D2D1Bitmap->Release();
		ImageInfo[i].D2D1Bitmap = NULL;
		ImageInfo[i] = { NULL };
	}
	if (BackgroundBitmap)
	{
		BackgroundBitmap->Release();
		BackgroundBitmap = NULL;
	}
	memset(BackgroundPath, 0, 512);
	mciSendString(TEXT("close bgm"), NULL, 0, NULL);
	PlaySoundA(NULL, NULL, NULL);
	GameVariables.clear();
	while (LocalVariableRef.size())
		LocalVariableRef.pop();
}

void ScanFileUntilZero(FILE* pFile, char* StrBuffer)
{
	char tmpChar;
	do 
	{
		tmpChar = (char)fgetc(pFile);
		*StrBuffer = tmpChar;
		StrBuffer++;
	} while (tmpChar);
}

ImageAnimClass::ImageAnimClass(IMGInfo* _PicInfo, ImageAnimClass** _MePoint)
{
	PicInfo = _PicInfo;
	MePoint = _MePoint;
}

void ImageAnimClass::FrameNotify()
{
	AnimState++;
	switch (AnimStyle)
	{
	case AnimStyle_NoAnim:
		*MePoint = NULL;
		delete this;
		return;
	case AnimStyle_NormalAnim: case AnimStyle_FadeIn:
	{
		D2D1_RECT_F rc;
		rc.bottom =  PicInfo->RectToDraw.bottom*D2D1HwndRenderTarget->GetPixelSize().height;
		rc.top = PicInfo->RectToDraw.top*D2D1HwndRenderTarget->GetPixelSize().height;
		rc.left = PicInfo->RectToDraw.left*D2D1HwndRenderTarget->GetPixelSize().width;
		rc.right = PicInfo->RectToDraw.right*D2D1HwndRenderTarget->GetPixelSize().width;
		D2D1HwndRenderTarget->DrawBitmap(PicInfo->D2D1Bitmap, rc,(float)AnimState/30,
			D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, D2D1::RectF(0, 0, PicInfo->D2D1Bitmap->GetPixelSize().width, PicInfo->D2D1Bitmap->GetPixelSize().height));
		if (AnimState >= 30)
		{
			*MePoint = NULL;
			delete this;
		}
		return;
	}
	case AnimStyle_FadeOut:
	{
		D2D1_RECT_F rc;
		rc.bottom = PicInfo->RectToDraw.bottom*D2D1HwndRenderTarget->GetPixelSize().height;
		rc.top = PicInfo->RectToDraw.top*D2D1HwndRenderTarget->GetPixelSize().height;
		rc.left = PicInfo->RectToDraw.left*D2D1HwndRenderTarget->GetPixelSize().width;
		rc.right = PicInfo->RectToDraw.right*D2D1HwndRenderTarget->GetPixelSize().width;
		D2D1HwndRenderTarget->DrawBitmap(PicInfo->D2D1Bitmap, rc, 1-(float)AnimState / 30,
			D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, D2D1::RectF(0, 0, PicInfo->D2D1Bitmap->GetPixelSize().width, PicInfo->D2D1Bitmap->GetPixelSize().height));
		if (AnimState >= 30)
			AnimState--;
		return;
	}
	case AnimStyle_FadeOutAndFadeIn:
	{
		D2D1_RECT_F rc;
		rc.bottom = PicInfo->RectToDraw.bottom*D2D1HwndRenderTarget->GetPixelSize().height;
		rc.top = PicInfo->RectToDraw.top*D2D1HwndRenderTarget->GetPixelSize().height;
		rc.left = PicInfo->RectToDraw.left*D2D1HwndRenderTarget->GetPixelSize().width;
		rc.right = PicInfo->RectToDraw.right*D2D1HwndRenderTarget->GetPixelSize().width;
		D2D1HwndRenderTarget->DrawBitmap(PicInfo->D2D1Bitmap, rc, abs(1 - (float)AnimState / 30),
			D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, D2D1::RectF(0, 0, PicInfo->D2D1Bitmap->GetPixelSize().width, PicInfo->D2D1Bitmap->GetPixelSize().height));
		if (AnimState==30)
		{
			WCHAR IMGPath[512];
			mbstowcs(IMGPath, PicInfo->ImagePath,512);
			LoadD2DImage(IMGPath, &(PicInfo->D2D1Bitmap));
		}
		if (AnimState >= 60)
		{
			*MePoint = NULL;
			delete this;
		}
		return;
	}
	default:
		break;
	}
}

void ChatAnimClass::FrameNotify()
{
	if (!ChatInfo.D2DTextFormat)
		return;
	D2D_RECT_F rc;
	rc.bottom = ChatInfo.RectForChat.bottom*D2D1HwndRenderTarget->GetPixelSize().height;
	rc.top = ChatInfo.RectForChat.top*D2D1HwndRenderTarget->GetPixelSize().height;
	rc.left = ChatInfo.RectForChat.left*D2D1HwndRenderTarget->GetPixelSize().width;
	rc.right = ChatInfo.RectForChat.right*D2D1HwndRenderTarget->GetPixelSize().width;
	if (AnimStyle == AnimStyle_NoAnim)
	{
		D2D1HwndRenderTarget->DrawText(ChatInfo.ChatMessage, wcslen(ChatInfo.ChatMessage), ChatInfo.D2DTextFormat, rc, ChatInfo.D2DBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
		return;
	}
	FrameCounter++;
	if (FrameCounter < 5)
	{
		D2D1HwndRenderTarget->DrawText(ChatToDisplay, wcslen(ChatToDisplay), ChatInfo.D2DTextFormat, rc, ChatInfo.D2DBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
		return;
	}
	FrameCounter = 0;
	AnimState++;
	wcscpy(ChatToDisplay, ChatInfo.ChatMessage);
	ChatToDisplay[AnimState + 1] = 0;
	D2D1HwndRenderTarget->DrawText(ChatToDisplay, wcslen(ChatToDisplay), ChatInfo.D2DTextFormat, rc, ChatInfo.D2DBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
	if (AnimState >= wcslen(ChatInfo.ChatMessage))
		AnimState--;
}

void ChatAnimClass::ChatChangeNotify()
{
	FrameCounter = 9;
	wcscpy(ChatToDisplay, TEXT(""));
	AnimState = 0;
}

bool ChatAnimClass::EndAnim()
{
	if (AnimState >= wcslen(ChatInfo.ChatMessage)-1)
		return false;
	else
	{
		AnimState = wcslen(ChatInfo.ChatMessage) - 1;
		return true;
	}
}

ChatAnimClass::ChatAnimClass()
{
	AnimState = wcslen(ChatInfo.ChatMessage) - 1;
}

HRESULT InitWindowlessVMR(
	HWND hwndApp,                  // 指定窗口的句柄，输入参数
	IGraphBuilder* pGraph,         // Filter Graph Manager指针，输入参数.
	IVMRWindowlessControl** ppWc // 输出参数.
)
{
	if (!pGraph || !ppWc) return E_POINTER;
	IBaseFilter* pVmr = NULL;
	IVMRWindowlessControl* pWc = NULL;
	// Create the VMR.
	HRESULT hr = CoCreateInstance(CLSID_VideoMixingRenderer, NULL,
		CLSCTX_INPROC, IID_IBaseFilter, (void**)&pVmr);
	if (FAILED(hr))
	{
		return hr;
	}

	// Add the VMR to the filter graph.
	hr = pGraph->AddFilter(pVmr, L"Video Mixing Renderer");
	if (FAILED(hr))
	{
		pVmr->Release();
		return hr;
	}
	// Set the rendering mode.
	IVMRFilterConfig* pConfig;
	hr = pVmr->QueryInterface(IID_IVMRFilterConfig, (void**)&pConfig);
	if (SUCCEEDED(hr))
	{
		hr = pConfig->SetRenderingMode(VMRMode_Windowless);
		pConfig->Release();
	}
	if (SUCCEEDED(hr))
	{
		// Set the window.
		hr = pVmr->QueryInterface(IID_IVMRWindowlessControl, (void**)&pWc);
		if (SUCCEEDED(hr))
		{
			hr = pWc->SetVideoClippingWindow(hwndApp);
			if (SUCCEEDED(hr))
			{
				*ppWc = pWc; // Return this as an AddRef'd pointer.
			}
			else
			{
				// An error occurred, so release the interface.
				pWc->Release();
			}
		}
	}
	pVmr->Release();
	return hr;
}

bool IsCharInCollection(char CharToTest, std::initializer_list<char> CharsList)
{
	for (auto it = CharsList.begin(); it < CharsList.end(); ++it)
	{
		if (CharToTest == *it)
			return true;
	}
	return false;
}

char OperatorPriority(char Operator)
{
	if (IsCharInCollection(Operator, { '(',')' }))
		return 0;
	if (IsCharInCollection(Operator, { '>','<','=' }))
		return 1;
	if (IsCharInCollection(Operator, { '+','-' }))
		return 2;
	return 3;
}

void AddCharToString(char* Dest, char CharToAdd)
{
	char Tmp[2];
	Tmp[0] = CharToAdd;
	Tmp[1] = 0;
	strcat(Dest, Tmp);
}

char* gcvtTrim(char* StrToTrim)
{
	if (StrToTrim[strlen(StrToTrim) - 1] == '.')
		StrToTrim[strlen(StrToTrim) - 1] = ' ';
	else
	{
		StrToTrim[strlen(StrToTrim) + 1] = 0;
		StrToTrim[strlen(StrToTrim)] = ' ';
	}
	return StrToTrim;
}

void InfixToPostfix(char* InfixExpression, char* PostfixExpression, int BufferSize)
{
	struct DoubleFlipData
	{
		bool IsOperator;
		float Number;
		char Operator;
	};
	std::stack<DoubleFlipData> ExpressionTmp;
	std::stack<char> OperatorStack;
	memset(PostfixExpression, 0, BufferSize);
	char* InfixIndex = InfixExpression + strlen(InfixExpression);
	while (InfixIndex >= InfixExpression)
	{
		while ((InfixIndex--), InfixIndex >= InfixExpression && *InfixIndex == ' ');
		InfixIndex++;
		char *InfixIndexTmp = InfixIndex;
		while ((InfixIndex--), InfixIndex >= InfixExpression && !IsCharInCollection(*InfixIndex, { '+','-','*','/','%','(',')','<','>','=' ,' ' }));
		if (InfixIndexTmp - ++InfixIndex == 0)
		{
			if (InfixIndex <= InfixExpression)       //中缀表达式操作符不会出现在表达式最前方
				break;
			InfixIndex--;
			ExpressionTmp.push({ true,0,*InfixIndex });
		}
		else
		{
			float Number;
			sscanf(InfixIndex, "%f", &Number);
			ExpressionTmp.push({ false,Number,0 });
		}
	}
	while (!ExpressionTmp.empty())
	{
		if (ExpressionTmp.top().IsOperator)
		{
			switch (ExpressionTmp.top().Operator)
			{
			case ')':
				while (OperatorStack.top() != '(')
				{
					AddCharToString(PostfixExpression, OperatorStack.top());
					OperatorStack.pop();
				}
				OperatorStack.pop();
				break;
			case '(':
				OperatorStack.push(ExpressionTmp.top().Operator);
				break;
			default:
				while (!OperatorStack.empty() && OperatorPriority(OperatorStack.top()) >= OperatorPriority(ExpressionTmp.top().Operator))
				{
					AddCharToString(PostfixExpression, OperatorStack.top());
					OperatorStack.pop();
				}
				OperatorStack.push(ExpressionTmp.top().Operator);
			}
		}
		else
		{
			//char NumberChars[25];
			sprintf(PostfixExpression, "%s %f", PostfixExpression, ExpressionTmp.top().Number);
			//strcat(PostfixExpression, gcvtTrim(_gcvt(ExpressionTmp.top().Number, 6, NumberChars)));
		}
		ExpressionTmp.pop();
	}
	while (!OperatorStack.empty())
	{
		AddCharToString(PostfixExpression, OperatorStack.top());
		OperatorStack.pop();
	}
}

float CaculatePostfixExpression(char* PostfixExpression)
{
	char* PostfixExpressionIndex = PostfixExpression;
	std::stack<float> NumberStack;
	char TmpChar[16];
	do
	{
		sscanf(PostfixExpressionIndex, "%s", TmpChar);
		while (*PostfixExpressionIndex == ' ')
			PostfixExpressionIndex++;
		if (IsCharInCollection(*TmpChar, { '+','-','*','/','%','<','>','=' }))
		{
			float NumberB = NumberStack.top();
			NumberStack.pop();
			float NumberA = NumberStack.top();
			NumberStack.pop();
			switch (*TmpChar)
			{
			case '+':
				NumberStack.push(NumberA + NumberB);
				break;
			case '-':
				NumberStack.push(NumberA - NumberB);
				break;
			case '*':
				NumberStack.push(NumberA * NumberB);
				break;
			case '/':
				NumberStack.push(NumberA / NumberB);
				break;
			case '%':
				NumberStack.push((int)NumberA % (int)NumberB);
				break;
			case '>':
				NumberStack.push(NumberA > NumberB);
				break;
			case '<':
				NumberStack.push(NumberA < NumberB);
				break;
			case '=':
				NumberStack.push(NumberA == NumberB);
				break;
			}
			PostfixExpressionIndex++;
		}
		else
		{
			{
				char* TmpCharIndex = TmpChar;
				int TmpCharLen = strlen(TmpChar);
				while (!IsCharInCollection(*TmpCharIndex, { '+','-','*','/','%','>','<','=' }) && TmpCharLen > 0)
					TmpCharIndex++, TmpCharLen--;
				*TmpCharIndex = 0;
			}
			NumberStack.push(atof(TmpChar));
			PostfixExpressionIndex += strlen(TmpChar);
		}
	} while (PostfixExpressionIndex < PostfixExpression + strlen(PostfixExpression));
	return NumberStack.top();
}

std::string& trim(std::string &s)
{
	if (s.empty())
	{
		return s;
	}
	s.erase(0, s.find_first_not_of(" "));
	s.erase(s.find_last_not_of(" ") + 1);
	return s;
}