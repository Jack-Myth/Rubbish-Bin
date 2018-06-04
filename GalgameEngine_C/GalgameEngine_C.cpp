#include <stdio.h>
#include <windows.h>
#include <locale.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#include <dshow.h>
#pragma comment(lib, "WINMM.LIB")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma  comment(lib,"strmiids.lib")

typedef enum Anim_Style
{
	AnimStyle_NoAnim,
	AnimStyle_NormalAnim,
	AnimStyle_FadeIn,
	AnimStyle_FadeOut,
	AnimStyle_FadeOutAndFadeIn,
} Anim_Style;

typedef struct _WhileInfo
{
	long StreamPosition;
	char expression[200];
} WhileInfo;

typedef struct  __ChatStyle
{
	float FontSize;
	int ItalicStyle;
	int ColorR;
	int ColorG;
	int ColorB;
	WCHAR FontName[100];
} _ChatStyle;

typedef struct _IMGInfo
{
	char ImagePath[200];
	ID2D1Bitmap* D2D1Bitmap;
	D2D1_RECT_F RectToDraw;
} IMGInfo;

typedef struct _BTNInfo
{
	D2D1_RECT_F Rect;
	long StreamPosition;
} BTNInfo;

typedef struct  _TextInfo
{
	_ChatStyle ChatStyle;
	WCHAR ChatMessage[1000];
	IDWriteTextFormat* D2DTextFormat;
	ID2D1Brush* D2DBrush;
	D2D1_RECT_F RectForChat;
} TextInfo;

typedef struct  _ImageAnimClass
{
	Anim_Style AnimStyle;
	int AnimState;
	IMGInfo* PicInfo;
	//ImageAnimClass(IMGInfo*, ImageAnimClass**);
	//virtual void FrameNotify() override;
} ImageAnimClass;

typedef struct _ChatAnimClass
{
	Anim_Style AnimStyle;
	int AnimState;
	int FrameCounter;
	WCHAR ChatToDisplay[1000];
} ChatAnimClass;

typedef struct _StackData
{
	void* pData[1024];
	int pIndex ;
} StackData;

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void ProcessScript(BOOL ReadLine);
int StrReplace(char strRes[], char from[], char to[]);
int CaculateExpression(char expression[]);
void* GetMinPoint(void* A, void* B);
void FindElsePlace();
void FindCloseCase(char BeginCase[], char CloseCase[]);
BOOL LoadD2DImage(WCHAR ImagePath[], ID2D1Bitmap** BitMapMayClear);
void SaveGame(char FilePath[]);
void LoadGame(char FilePath[]);
void ClearGameState();
HRESULT InitWindowlessVMR(HWND hwndApp, IGraphBuilder*, IVMRWindowlessControl**);
void Stack_Init(StackData* SD);
void Stack_push(StackData* SD, void* pOint);
BOOL Stack_IsEmpty(StackData* SD);
void* Stack_top(StackData* SD);
void Stack_pop(StackData* SD);
int Stack_size(StackData* SD);
float abs(float Value);

ID2D1Factory* D2D1Factory=NULL;
IDWriteFactory* DWriteFactory = NULL;
ID2D1HwndRenderTarget* D2D1HwndRenderTarget = NULL;
IWICImagingFactory* WICImagingFactory = NULL;
HWND MainHwnd = NULL;
FILE* FileHandle = NULL;
ID2D1Bitmap* BackgroundBitmap = NULL;
char BackgroundPath[200] = {NULL};
char BGMPath[200] = { NULL };
IMGInfo ImageInfo[30] = { NULL };
TextInfo ChatInfo = {0};
RECT WindowRect = { 0,0,1280,720 };
int GameFlag[100] = { 0 };
char StrFlag[10][1000] = { 0 };
BTNInfo ButtonInfo[30] = {0};
StackData whileStack;
StackData buttonStack;
StackData subStack;
ImageAnimClass* ImageAnim[30] = {NULL};
ChatAnimClass ChatAnim;
IGraphBuilder *pGraph = NULL;
IMediaControl *pControl = NULL;
IMediaEvent   *pEvent = NULL;
IMediaSeeking *pSeeking = NULL;
IVMRWindowlessControl* pWindowLessControl = NULL;
int DelayFrameRemain = -1;
int StretchRuler = 768;
long WindowHeight = 1280, WindowWidth = 720;
BOOL Videoing = FALSE;
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
	AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, FALSE);
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &D2D1Factory);
	CoInitialize(NULL);
	CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&WICImagingFactory));
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&DWriteFactory));
	MainHwnd = CreateWindow(wnd.lpszClassName, TEXT("My Galgame Engine C version"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WindowRect.right-WindowRect.left,WindowRect.bottom- WindowRect.top, NULL, NULL, hInstance, NULL);
	setlocale(LC_ALL, "");
	DWriteFactory->CreateTextFormat(TEXT("微软雅黑"), NULL,
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, 20, TEXT("zh-cn"), &ChatInfo.D2DTextFormat);
	ChatInfo.ChatStyle = { 20,0,0,0,0,TEXT("微软雅黑") };
	D2D1HwndRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), (ID2D1SolidColorBrush**)&ChatInfo.D2DBrush);
	ChatInfo.RectForChat = D2D1::RectF(0.1, 0.7, 0.9, 1);
	ChatAnim.AnimState = 0;
	ChatAnim.FrameCounter = 9;
	ChatAnim.AnimState= wcslen(ChatInfo.ChatMessage) - 1;
	CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&pGraph);
	pGraph->QueryInterface(IID_IMediaControl, (void **)&pControl);
	pGraph->QueryInterface(IID_IMediaEvent, (void **)&pEvent);
	pGraph->QueryInterface(IID_IMediaSeeking, (void **)&pSeeking);
	InitWindowlessVMR(MainHwnd, pGraph, &pWindowLessControl);
	ProcessScript(FALSE);
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
	BOOL ImageHasBeenAnimed=FALSE;
	switch (message)
	{
	case WM_CREATE:
		FileHandle = fopen("Script.ges", "r");
		if (!FileHandle)
		{
			MessageBox(NULL, TEXT("没有找到脚本文件，退出"), TEXT("Can not Find Scrpit.ges"), MB_OK);
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
		DWriteFactory->CreateTextFormat(ChatInfo.ChatStyle.FontName, NULL, DWRITE_FONT_WEIGHT_REGULAR, (DWRITE_FONT_STYLE)((BOOL)(ChatInfo.ChatStyle.ItalicStyle) * 2), DWRITE_FONT_STRETCH_NORMAL, ChatInfo.ChatStyle.FontSize*((float)Siz/(float)StretchRuler), TEXT("zh-cn"), &ChatInfo.D2DTextFormat);
		RECT SourceR;
		long VWidth, VHeight;
		pWindowLessControl->GetNativeVideoSize(&VWidth, &VHeight, NULL, NULL);
		SourceR = { 0,0,VWidth,VHeight };
		GetClientRect(MainHwnd, &WindowRect);
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
			ImageHasBeenAnimed = FALSE;
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
								ImageHasBeenAnimed = TRUE;
								{
									ImageAnim[a]->AnimState++;
									switch (ImageAnim[a]->AnimStyle)
									{
									case AnimStyle_NoAnim:
										free(ImageAnim[a]);
										ImageAnim[a] = NULL;
										break;
									case AnimStyle_NormalAnim: case AnimStyle_FadeIn:
									{
										D2D1_RECT_F rc;
										rc.bottom = ImageAnim[a]->PicInfo->RectToDraw.bottom*D2D1HwndRenderTarget->GetPixelSize().height;
										rc.top = ImageAnim[a]->PicInfo->RectToDraw.top*D2D1HwndRenderTarget->GetPixelSize().height;
										rc.left = ImageAnim[a]->PicInfo->RectToDraw.left*D2D1HwndRenderTarget->GetPixelSize().width;
										rc.right = ImageAnim[a]->PicInfo->RectToDraw.right*D2D1HwndRenderTarget->GetPixelSize().width;
										D2D1HwndRenderTarget->DrawBitmap(ImageAnim[a]->PicInfo->D2D1Bitmap, rc, (float)ImageAnim[a]->AnimState / 30,
											D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, D2D1::RectF(0, 0, ImageAnim[a]->PicInfo->D2D1Bitmap->GetPixelSize().width, ImageAnim[a]->PicInfo->D2D1Bitmap->GetPixelSize().height));
										if (ImageAnim[a]->AnimState >= 30)
										{
											free(ImageAnim[a]);
											ImageAnim[a] = NULL;
										}
										break;
									}
									case AnimStyle_FadeOut:
									{
										D2D1_RECT_F rc;
										rc.bottom = ImageAnim[a]->PicInfo->RectToDraw.bottom*D2D1HwndRenderTarget->GetPixelSize().height;
										rc.top = ImageAnim[a]->PicInfo->RectToDraw.top*D2D1HwndRenderTarget->GetPixelSize().height;
										rc.left = ImageAnim[a]->PicInfo->RectToDraw.left*D2D1HwndRenderTarget->GetPixelSize().width;
										rc.right = ImageAnim[a]->PicInfo->RectToDraw.right*D2D1HwndRenderTarget->GetPixelSize().width;
										D2D1HwndRenderTarget->DrawBitmap(ImageAnim[a]->PicInfo->D2D1Bitmap, rc, 1 - (float)ImageAnim[a]->AnimState / 30,
											D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, D2D1::RectF(0, 0, ImageAnim[a]->PicInfo->D2D1Bitmap->GetPixelSize().width, ImageAnim[a]->PicInfo->D2D1Bitmap->GetPixelSize().height));
										if (ImageAnim[a]->AnimState >= 30)
											ImageAnim[a]->AnimState--;
										break;
									}
									case AnimStyle_FadeOutAndFadeIn:
									{
										D2D1_RECT_F rc;
										rc.bottom = ImageAnim[a]->PicInfo->RectToDraw.bottom*D2D1HwndRenderTarget->GetPixelSize().height;
										rc.top = ImageAnim[a]->PicInfo->RectToDraw.top*D2D1HwndRenderTarget->GetPixelSize().height;
										rc.left = ImageAnim[a]->PicInfo->RectToDraw.left*D2D1HwndRenderTarget->GetPixelSize().width;
										rc.right = ImageAnim[a]->PicInfo->RectToDraw.right*D2D1HwndRenderTarget->GetPixelSize().width;
										D2D1HwndRenderTarget->DrawBitmap(ImageAnim[a]->PicInfo->D2D1Bitmap, rc, abs(1 - (float)ImageAnim[a]->AnimState / 30),
											D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, D2D1::RectF(0, 0, ImageAnim[a]->PicInfo->D2D1Bitmap->GetPixelSize().width, ImageAnim[a]->PicInfo->D2D1Bitmap->GetPixelSize().height));
										if (ImageAnim[a]->AnimState == 30)
										{
											WCHAR IMGPath[200];
											mbstowcs(IMGPath, ImageAnim[a]->PicInfo->ImagePath, 200);
											LoadD2DImage(IMGPath, &(ImageAnim[a]->PicInfo->D2D1Bitmap));
										}
										if (ImageAnim[a]->AnimState >= 60)
										{
											free(ImageAnim[a]);
											ImageAnim[a] = NULL;
										}
										break;
									}
									default:
										break;
									}
								}
								break;
							}
						}
					}
					if (ImageHasBeenAnimed)
					{
						ImageHasBeenAnimed = FALSE;
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
			do
			{
				if (!ChatInfo.D2DTextFormat)
					break;
				D2D_RECT_F rc;
				rc.bottom = ChatInfo.RectForChat.bottom*D2D1HwndRenderTarget->GetPixelSize().height;
				rc.top = ChatInfo.RectForChat.top*D2D1HwndRenderTarget->GetPixelSize().height;
				rc.left = ChatInfo.RectForChat.left*D2D1HwndRenderTarget->GetPixelSize().width;
				rc.right = ChatInfo.RectForChat.right*D2D1HwndRenderTarget->GetPixelSize().width;
				if (ChatAnim.AnimStyle == AnimStyle_NoAnim)
				{
					D2D1HwndRenderTarget->DrawText(ChatInfo.ChatMessage, wcslen(ChatInfo.ChatMessage), ChatInfo.D2DTextFormat, rc, ChatInfo.D2DBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
					break;;
				}
				ChatAnim.FrameCounter++;
				if (ChatAnim.FrameCounter < 5)
				{
					D2D1HwndRenderTarget->DrawText(ChatAnim.ChatToDisplay, wcslen(ChatAnim.ChatToDisplay), ChatInfo.D2DTextFormat, rc, ChatInfo.D2DBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
					break;;
				}
				ChatAnim.FrameCounter = 0;
				ChatAnim.AnimState++;
				wcscpy(ChatAnim.ChatToDisplay, ChatInfo.ChatMessage);
				ChatAnim.ChatToDisplay[ChatAnim.AnimState + 1] = 0;
				D2D1HwndRenderTarget->DrawText(ChatAnim.ChatToDisplay, wcslen(ChatAnim.ChatToDisplay), ChatInfo.D2DTextFormat, rc, ChatInfo.D2DBrush, D2D1_DRAW_TEXT_OPTIONS_ENABLE_COLOR_FONT);
				if (ChatAnim.AnimState >= wcslen(ChatInfo.ChatMessage))
					ChatAnim.AnimState--;
			} while (0);
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
			ProcessScript(FALSE);
		}
		else if (DelayFrameRemain > 0)
			DelayFrameRemain--;
		//ReleaseDC(MainHwnd, MainHDC);
		break;
	case WM_LBUTTONDOWN:
		BOOL EndAnim;
		if (ChatAnim.AnimState >= wcslen(ChatInfo.ChatMessage) - 1)
			EndAnim = FALSE;
		else
		{
			ChatAnim.AnimState = wcslen(ChatInfo.ChatMessage) - 1;
			EndAnim = TRUE;
		}
		if (EndAnim)
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
				long* TmpStack = (long*)malloc(sizeof(long));
				*TmpStack = ftell(FileHandle);
				Stack_push(&buttonStack, TmpStack);
				fseek(FileHandle, BI.StreamPosition, SEEK_SET);
				break;
			}
		}
		ProcessScript(FALSE);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_SYSCOMMAND:
		DefWindowProc(hwnd, message, wParam, lParam);
		if (wParam == SC_RESTORE)
			SetWindowPos(MainHwnd, HWND_TOP, 0, 0, WindowWidth, WindowHeight, SWP_NOMOVE);
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
		break;
	}
	return 0;
}

void ProcessScript(BOOL ReadLine)
{
	char Command[50] = { NULL };
	float ArgFloat[5] = { 0 };
	int ArgInt[5] = { 0 };
	char ArgStr[3][200] = { NULL };
	char ArgLongStr[1000] = { NULL };
	WCHAR WCharTMP[100] = { NULL };
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
			for (int i = 0; i < 10; i++)
			{
				sprintf(ArgStr[1], "str[%d]", i);
				StrReplace(ArgStr[0], ArgStr[1], StrFlag[i]);
			}
			SetWindowTextA(MainHwnd, ArgStr[0]);
		} 
		else if (!strcmp(Command, "setbackground"))
		{
			fscanf(FileHandle,"(%[^)])", ArgStr[0]);
			for (int i = 0; i < 10; i++)
			{
				sprintf(ArgStr[1], "str[%d]", i);
				StrReplace(ArgStr[0], ArgStr[1], StrFlag[i]);
			}
			mbstowcs(WCharTMP, ArgStr[0], 100);
			if (LoadD2DImage(WCharTMP, &BackgroundBitmap))
				strcpy(BackgroundPath, ArgStr[0]);
		} 
		else if (!strcmp(Command, "setchat"))
		{
			fscanf(FileHandle, "(%[^,],%d)", ArgLongStr,&ArgInt[0]);
			for (int i = 0; i < 10; i++)
			{
				sprintf(ArgStr[1], "str[%d]", i);
				StrReplace(ArgLongStr, ArgStr[1], StrFlag[i]);
			}
			StrReplace(ArgLongStr, "\\n", "\n");
			mbstowcs(ChatInfo.ChatMessage, ArgLongStr, 1000);
			ChatAnim.AnimStyle = (Anim_Style)ArgInt[0];
			{
				ChatAnim.FrameCounter = 9;
				wcscpy(ChatAnim.ChatToDisplay, TEXT(""));
				ChatAnim.AnimState = 0;
			}
		}
		else if (!strcmp(Command, "setimage"))
		{
			fscanf(FileHandle, "(%[^,],%f,%f,%f,%f,%d)", ArgStr[0], &ArgFloat[0], &ArgFloat[1], &ArgFloat[2], &ArgFloat[3], &ArgInt[0]);
			if (!(ArgInt[0] >= 0 && ArgInt[0] < 30))
				continue;
			for (int i = 0; i < 10; i++)
			{
				sprintf(ArgStr[1], "str[%d]", i);
				StrReplace(ArgStr[0], ArgStr[1], StrFlag[i]);
			}
			mbstowcs(WCharTMP, ArgStr[0], 100);
			for (int i = 0; i < 30; i++)
			{
				if (ImageAnim[i] && ImageAnim[i]->PicInfo == &ImageInfo[ArgInt[0]])
				{
					free(ImageAnim[i]);
					ImageAnim[i] = NULL;
					break;
				}
			}
			if(LoadD2DImage(WCharTMP, &ImageInfo[ArgInt[0]].D2D1Bitmap))
				strcpy(ImageInfo[ArgInt[0]].ImagePath, ArgStr[0]);
			ImageInfo[ArgInt[0]].RectToDraw.left = ArgFloat[0];
			ImageInfo[ArgInt[0]].RectToDraw.top = ArgFloat[1];
			ImageInfo[ArgInt[0]].RectToDraw.right = ArgFloat[2];
			ImageInfo[ArgInt[0]].RectToDraw.bottom = ArgFloat[3];
		}
		else if (!strcmp(Command, "setimageA"))
		{
			fscanf(FileHandle, "(%[^,],%f,%f,%f,%f,%d,%d)", ArgStr[0], &ArgFloat[0], &ArgFloat[1], &ArgFloat[2], &ArgFloat[3], &ArgInt[0], &ArgInt[1]);
			if (!(ArgInt[0] >= 0 && ArgInt[0] < 30))
				continue;
			for (int i = 0; i < 10; i++)
			{
				sprintf(ArgStr[1], "str[%d]", i);
				StrReplace(ArgStr[0], ArgStr[1], StrFlag[i]);
			}
			mbstowcs(WCharTMP, ArgStr[0], 100);
			ImageAnimClass** IAC=NULL;
			for (int i = 0; i < 30; i++)
			{
				if (ImageAnim[i] && ImageAnim[i]->PicInfo == &ImageInfo[ArgInt[0]])
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
			switch (ArgInt[1])
			{
			case 0: case 1: case 2: case 3:
				if (IAC)
				{
					free(*IAC);
					if (LoadD2DImage(WCharTMP, &ImageInfo[ArgInt[0]].D2D1Bitmap))
						strcpy(ImageInfo[ArgInt[0]].ImagePath, ArgStr[0]);
					*IAC = (ImageAnimClass*)malloc(sizeof(ImageAnimClass));
					(*IAC)->AnimState = 0;
					(*IAC)->PicInfo = &ImageInfo[ArgInt[0]];
					//(*IAC)->MePoint = _MePoint;
					(*IAC)->AnimStyle = (Anim_Style)ArgInt[1];
				}
				break;
			case 4:
				if (IAC)
				{
					free(*IAC);
					strcpy(ImageInfo[ArgInt[0]].ImagePath, ArgStr[0]);
					*IAC = (ImageAnimClass*)malloc(sizeof(ImageAnimClass));
					(*IAC)->AnimState = 0;
					(*IAC)->PicInfo = &ImageInfo[ArgInt[0]];
					//(*IAC)->MePoint = _MePoint;
					(*IAC)->AnimStyle = (Anim_Style)ArgInt[1];
				}
				break;
			default:
				break;
			}
			ImageInfo[ArgInt[0]].RectToDraw.left = ArgFloat[0];
			ImageInfo[ArgInt[0]].RectToDraw.top = ArgFloat[1];
			ImageInfo[ArgInt[0]].RectToDraw.right = ArgFloat[2];
			ImageInfo[ArgInt[0]].RectToDraw.bottom = ArgFloat[3];

		}
		else if (!strcmp(Command, "clearimage"))
		{
			fscanf(FileHandle, "(%d)", &ArgInt[0]);
			if (!(ArgInt[0] >= 0 && ArgInt[0] < 30))
				continue;
			for (int i=0;i<30;i++)
			{
				if (ImageAnim[i]&&ImageAnim[i]->PicInfo==&ImageInfo[ArgInt[0]])
				{
					free(ImageAnim[i]);
					ImageAnim[i] = NULL;
					break;
				}
			}
			if (ImageInfo[ArgInt[0]].D2D1Bitmap)
			{
				ImageInfo[ArgInt[0]].D2D1Bitmap->Release();
				ImageInfo[ArgInt[0]].D2D1Bitmap = NULL;
			}
		}
		else if (!strcmp(Command, "setchatrect"))
		{
			fscanf(FileHandle, "(%f,%f,%f,%f)", &ArgFloat[0], &ArgFloat[1], &ArgFloat[2], &ArgFloat[3]);
			ChatInfo.RectForChat.left = ArgFloat[0];
			ChatInfo.RectForChat.top = ArgFloat[1];
			ChatInfo.RectForChat.right = ArgFloat[2];
			ChatInfo.RectForChat.bottom = ArgFloat[3];
		} 
		else if (!strcmp(Command, "setchatstyle"))
		{
			IDWriteTextFormat* Tmp = ChatInfo.D2DTextFormat;
			ID2D1Brush* BrushTmp = ChatInfo.D2DBrush;
			fscanf(FileHandle, "(%[^,],%f,%d,%d,%d,%d)", ArgStr[0],&ArgFloat[0],&ArgInt[0], &ArgInt[1], &ArgInt[2], &ArgInt[3]);
			mbstowcs(WCharTMP, ArgStr[0], 1000);
			int Siz;
			GetClientRect(MainHwnd, &WindowRect);
			if (WindowRect.right - WindowRect.left > WindowRect.bottom - WindowRect.top)
				Siz = WindowRect.bottom - WindowRect.top;
			else
				Siz = WindowRect.right - WindowRect.left;
			DWriteFactory->CreateTextFormat(WCharTMP, NULL, DWRITE_FONT_WEIGHT_REGULAR, (DWRITE_FONT_STYLE)((BOOL)(ArgInt[0]) * 2), DWRITE_FONT_STRETCH_NORMAL, ArgFloat[0]* ((float)Siz / (float)StretchRuler), TEXT("zh-cn"), &ChatInfo.D2DTextFormat);
			if (ChatInfo.D2DTextFormat)
			{
				Tmp->Release();
				wcscpy(ChatInfo.ChatStyle.FontName, WCharTMP);
				ChatInfo.ChatStyle.FontSize = ArgFloat[0];
				ChatInfo.ChatStyle.ItalicStyle = ArgInt[0];
			}
			else
				ChatInfo.D2DTextFormat = Tmp;
			D2D1HwndRenderTarget->CreateSolidColorBrush(D2D1::ColorF(RGB(ArgInt[1], ArgInt[2], ArgInt[3])), (ID2D1SolidColorBrush**)&ChatInfo.D2DBrush);
			if (ChatInfo.D2DBrush)
			{
				BrushTmp->Release();
				ChatInfo.ChatStyle.ColorR = ArgInt[1];
				ChatInfo.ChatStyle.ColorG = ArgInt[2];
				ChatInfo.ChatStyle.ColorB = ArgInt[3];
			}
			else
				ChatInfo.D2DBrush = BrushTmp;
		}
		else if (!strcmp(Command, "playbgm"))
		{
			fscanf(FileHandle, "(%[^,],%d)", ArgStr[0],&ArgInt[0]);
			for (int i = 0; i < 10; i++)
			{
				sprintf(ArgStr[1], "str[%d]", i);
				StrReplace(ArgStr[0], ArgStr[1], StrFlag[i]);
			}
			sprintf(ArgStr[1], "open %s alias bgm", ArgStr[0]);
			mciSendString(TEXT("close bgm"), NULL, 0, NULL);
			mciSendStringA(ArgStr[1], NULL, 0, NULL);
			mciSendString(TEXT("play bgm repeat"), NULL, 0, NULL);
			strcpy(BGMPath, ArgStr[0]);
		} 
		else if (!strcmp(Command, "playvideo"))
		{
			long VHeight, VWidth;
			LONGLONG DuratioTmp;
			fscanf(FileHandle, "(%[^)])", ArgStr[0]);
			for (int i = 0; i < 10; i++)
			{
				sprintf(ArgStr[1], "str[%d]", i);
				StrReplace(ArgStr[0], ArgStr[1], StrFlag[i]);
			}
			mbstowcs(WCharTMP, ArgStr[0], 200);
			pGraph->RenderFile(WCharTMP, NULL);
			pWindowLessControl->GetNativeVideoSize(&VWidth, &VHeight, NULL, NULL);
			RECT SourceR;
			pWindowLessControl->GetNativeVideoSize(&VWidth, &VHeight, NULL, NULL);
			SourceR = { 0,0,VWidth,VHeight };
			GetClientRect(MainHwnd, &WindowRect);
			pWindowLessControl->SetVideoPosition(&SourceR, &WindowRect);
			pControl->Run();
			Videoing = TRUE;
			break;
		}
		else if (!strcmp(Command, "stopvideo"))
		{
			fscanf(FileHandle, "%*s");
			Videoing = FALSE;
			pControl->Stop();
		}
		else if (!strcmp(Command, "playvoice"))
		{
			fscanf(FileHandle, "(%[^)])", ArgStr[0]);
			for (int i = 0; i < 10; i++)
			{
				sprintf(ArgStr[1], "str[%d]", i);
				StrReplace(ArgStr[0], ArgStr[1], StrFlag[i]);
			}
			//PlaySoundA(NULL, NULL, NULL);
			PlaySoundA(ArgStr[0], NULL, SND_FILENAME| SND_ASYNC);
		}
		else if (!strcmp(Command, "stopvoice"))
		{
			fscanf(FileHandle, "%*s");
			PlaySoundA(NULL,NULL,NULL);
		}
		else if (!strcmp(Command, "stopbgm"))
		{
			fscanf(FileHandle, "%*s");
			mciSendString(TEXT("close bgm"), NULL, 0, NULL);
			strcpy(BGMPath, "");
		} 
		else if (!strcmp(Command, "waitclick"))
		{
			fscanf(FileHandle, "(%d)", &ArgInt[0]);
			if (ArgInt[0] != 1)
				SendMessage(MainHwnd, WM_PAINT, 0, 0);
			DelayFrameRemain = -1;
			break;
		} 
		else if (!strcmp(Command, "button"))
		{
			fscanf(FileHandle, "(%f,%f,%f,%f,%d)", &ArgFloat[0], &ArgFloat[1], &ArgFloat[2], &ArgFloat[3], &ArgInt[0]);
			if (!(ArgInt[0]<30&& ArgInt[0]>=0))
				continue;
			ButtonInfo[ArgInt[0]].StreamPosition = ftell(FileHandle);
			ButtonInfo[ArgInt[0]].Rect = D2D1::RectF(ArgFloat[0], ArgFloat[1], ArgFloat[2], ArgFloat[3]);
			FindCloseCase("button", "endbutton");
		}
		else if (!strcmp(Command, "endbutton"))		
		{
			fscanf(FileHandle, "%*[^\n]");
			if (Stack_IsEmpty(&buttonStack))
				continue;
			fseek(FileHandle, *(long*)Stack_top(&buttonStack), SEEK_SET);
			Stack_pop(&buttonStack);
		}
		else if (!strcmp(Command, "removebutton"))
		{
			fscanf(FileHandle, "(%d)", &ArgInt[0]);
			if (!(ArgInt[0] < 30 && ArgInt[0] >= 0))
				continue;
			ButtonInfo[ArgInt[0]].StreamPosition = 0;
		}
		else if (!strcmp(Command, "clearbutton"))
		{
			fscanf(FileHandle, "%*[^\n]");
			for (BTNInfo& BI : ButtonInfo)
				BI.StreamPosition = 0;
		}
		else if (!strcmp(Command, "save"))
		{
			fscanf(FileHandle, "(%d)",&ArgInt[0]);
			sprintf(ArgStr[0], "%d.save", ArgInt[0]);
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
			fscanf(FileHandle, "(%d)", &ArgInt[0]);
			sprintf(ArgStr[0], "%d.save", ArgInt[0]);
			LoadGame(ArgStr[0]);
		}
		else if (!strcmp(Command, "setwindowsize"))
		{
			fscanf(FileHandle, "(%d,%d)", &ArgInt[0], &ArgInt[1]);
			SetWindowPos(MainHwnd, HWND_TOP, 0, 0, ArgInt[0], ArgInt[1], SWP_NOMOVE);
			WindowWidth = ArgInt[0];
			WindowHeight = ArgInt[1];
			if (ArgInt[0] > ArgInt[1])
				StretchRuler = ArgInt[1];
			else
				StretchRuler = ArgInt[0];
		}
		else if (!strcmp(Command, "setflag"))////////////////////////////////////////////////////////////////////非功能逻辑控制语句///////////////////////////////////////////////////////////////////////////
		{
			fscanf(FileHandle, "(%d,%[^)])", &ArgInt[0],ArgStr[0]);
			if (ArgInt[0] < 100 && ArgInt[0] >= 0)
				GameFlag[ArgInt[0]] = CaculateExpression(ArgStr[0]);
		}
		else if (!strcmp(Command, "setstrflag"))
		{
			fscanf(FileHandle, "(%d,%[^)])", &ArgInt[0], ArgLongStr);
			for (int i = 0; i < 10; i++)
			{
				sprintf(ArgStr[1], "str[%d]", i);
				StrReplace(ArgLongStr, ArgStr[1], StrFlag[i]);
			}
			if (ArgInt[0] < 10 && ArgInt[0] >= 0)
				strcpy(StrFlag[ArgInt[0]], ArgLongStr);
		}
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
				WhileInfo* WI=(WhileInfo*)malloc(sizeof(WhileInfo));
				WI->StreamPosition = ftell(FileHandle);
				strcpy(WI->expression, ArgStr[0]);
				Stack_push(&whileStack, WI);
			}
			else
				FindCloseCase("while","loop");
		}
		else if (!strcmp(Command, "loop"))
		{
			fscanf(FileHandle, "%*[^\n]");
			if (Stack_IsEmpty(&whileStack))
				continue;
			if (CaculateExpression(((WhileInfo*)Stack_top(&whileStack))->expression))
				fseek(FileHandle, ((WhileInfo*)Stack_top(&whileStack))->StreamPosition, SEEK_SET);
			else
				Stack_pop(&whileStack);
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
				fscanf(FileHandle, "%s", &ArgStr[1]);
				if (!strcmp(ArgStr[0],ArgStr[1]))
				{
					while (!Stack_IsEmpty(&whileStack))
						Stack_pop(&whileStack);
					while (!Stack_IsEmpty(&buttonStack))
						Stack_pop(&buttonStack);
					while (!Stack_IsEmpty(&subStack))
						Stack_pop(&subStack);
					SteamCurrent = ftell(FileHandle);
					break;
				}
			}
			fseek(FileHandle, SteamCurrent, SEEK_SET);
		}
		else if (!strcmp(Command, "delay"))
		{
			fscanf(FileHandle, "(%f)", &ArgFloat[0]);
			DelayFrameRemain = (int)(ArgFloat[0] * 30);
			break;
		}
		else if (!strcmp(Command, "end"))
		{
			exit(0);
		}
		else if (!strcmp(Command, "endsub"))
		{
			fscanf(FileHandle, "%*s");
			fseek(FileHandle, *(long*)Stack_top(&subStack), SEEK_SET);
			Stack_pop(&subStack);
		}
		else 
		{
			long FileCurrent, SubCurrent;
			//fscanf(FileHandle, "%*s");
			FileCurrent = ftell(FileHandle);
			//strcpy(ArgStr[0], Command);
			bool Findway = false;
			fseek(FileHandle, 0, SEEK_SET);
			while (!feof(FileHandle))
			{
				if ((ArgStr[0][0] = fgetc(FileHandle)) != '\n'&&ArgStr[0][0] != ' '&&ArgStr[0][0] != '\t')
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
				fscanf(FileHandle, "%s", ArgStr[0]);
				if (!strcmp(ArgStr[0], "sub"))
				{
					while (fgetc(FileHandle) == ' ');
					fseek(FileHandle, -1, SEEK_CUR);
					fscanf(FileHandle, "%[^(]", ArgStr[0]);
					if (!strcmp(ArgStr[0], Command))
					{
						fscanf(FileHandle, "%*s");
						SubCurrent = ftell(FileHandle);
						Findway = true;
						break;
					}
				}
			}
			fseek(FileHandle, FileCurrent, SEEK_SET);
			if (!Findway)
			{
				fscanf(FileHandle, "%*s");
				continue;
			}
			fscanf(FileHandle, "(%d", &ArgInt[0]);
			while (fgetc(FileHandle) != ')')
			{
				fscanf(FileHandle, "%[^,)]", ArgLongStr);
				strcpy(StrFlag[ArgInt[0]], ArgLongStr);
				ArgInt[0]++;
			}
			Stack_push(&subStack, malloc(sizeof(long)));
			*(long*)Stack_top(&subStack) = ftell(FileHandle);
			//subStack.push(ftell(FileHandle));
			fseek(FileHandle, SubCurrent, SEEK_SET);
		}
	} while (!ReadLine);
}

int StrReplace(char strRes[], char from[], char to[]) 
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

int CaculateExpression(char Originexpression[])
{
	char expression[200];
	int A, B;
	char C;
	char* FindPoint = 0;
	int FlagNumber = 0;
	char TmpStr[20] = { NULL };
	strcpy(expression, Originexpression);
	while (FindPoint = strstr(expression, "flag["))
	{
		sscanf(FindPoint, "flag[%d]", &FlagNumber);
		sprintf(TmpStr, "flag[%d]", FlagNumber);
		sprintf(&TmpStr[10], "%d", GameFlag[FlagNumber]);
		StrReplace(expression, TmpStr, &TmpStr[10]);
	}
	while (FindPoint = (char*)GetMinPoint(strchr(expression, '*'), strchr(expression, '/')))
	{
		while ((long)(--FindPoint) >= (long)expression && ('0' <= *FindPoint&&*FindPoint <= '9'));
		FindPoint++;
		sscanf(FindPoint, "%d%c%d", &A, &C, &B);
		sprintf(TmpStr, "%d%c%d", A, C, B);
		switch (C)
		{
		case '*':
			sprintf(&TmpStr[15], "%d", A * B);
			break;
		case '/':
			sprintf(&TmpStr[15], "%d", A / B);
			break;
		}
		StrReplace(expression, TmpStr, &TmpStr[15]);
	}
	while (FindPoint = (char*)GetMinPoint(strchr(expression, '+'), strchr(expression, '-')))
	{
		while ((long)(--FindPoint) >= (long)expression && ('0' <= *FindPoint&&*FindPoint <= '9'));
		FindPoint++;
		sscanf(FindPoint, "%d%c%d", &A, &C, &B);
		sprintf(TmpStr, "%d%c%d", A, C, B);
		switch (C)
		{
		case '+':
			sprintf(&TmpStr[15], "%d", A+B);
			break;
		case '-':
			sprintf(&TmpStr[15], "%d", A - B);
			break;
		}
		StrReplace(expression, TmpStr, &TmpStr[15]);
	}
	FindPoint = expression;
	do 
	{
		if(!(strchr(expression, '<')|| strchr(expression, '>')|| strchr(expression, '=')))
			break;
		if (*FindPoint == '&')
			*FindPoint = '*';
		if (*FindPoint == '|')
			*FindPoint = '/';
		if (*FindPoint == '/' || *FindPoint == '*')
			FindPoint++;
		sscanf(FindPoint, "%d%c%d", &A, &C, &B);
		sprintf(TmpStr, "%d%c%d", A,C,B);
		switch (C)
		{
		case '<':
			StrReplace(expression, TmpStr, itoa((int)(A < B), &TmpStr[15], 4));
			break;
		case '>':
			StrReplace(expression, TmpStr, itoa((int)(A > B), &TmpStr[15], 4));
			break;
		case '=':
			StrReplace(expression, TmpStr, itoa((int)(A == B), &TmpStr[15], 4));
			break;
		}
	} while (FindPoint = (char*)GetMinPoint(strchr(expression, '&'),strchr(expression, '|')));
	while (strchr(expression, '/') || strchr(expression, '*'))
	{
		sscanf(expression, "%d%c%d", &A, &C, &B);
		sprintf(TmpStr, "%d%c%d", A, C, B);
		switch (C)
		{
		case '*':
			StrReplace(expression, TmpStr, itoa((int)(A && B), &TmpStr[15], 4));
			break;
		case '/':
			StrReplace(expression, TmpStr, itoa((int)(A || B), &TmpStr[15], 4));
			break;
		}
	} 
	return atoi(expression);
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

BOOL LoadD2DImage(WCHAR ImagePath[],ID2D1Bitmap** BitMapMayClear)
{
	IWICBitmap*    m_pWicBitmap = NULL;
	IWICBitmapDecoder* m_pWicDecoder = NULL;
	IWICBitmapFrameDecode* m_pWicFrameDecoder = NULL;
	if (FAILED(WICImagingFactory->CreateDecoderFromFilename(ImagePath, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &m_pWicDecoder)))
		return FALSE;
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
	return TRUE;
}

void SaveGame(char FilePath[])
{
	FILE* SaveGameFile = fopen(FilePath, "wb+");
	if (!SaveGameFile)
		return;
	WhileInfo* TmpWhileinfo = (WhileInfo*)malloc(sizeof(WhileInfo) *Stack_size(&whileStack));
	WhileInfo* TmpW=NULL;
	int Size = Stack_size(&whileStack);
	fprintf(SaveGameFile, "%c", (char)Size);
	for (int i=0;i<Size;i++)
	{
		TmpWhileinfo[i] = *(WhileInfo*)Stack_top(&whileStack);
		Stack_pop(&whileStack);
	}
	fwrite(TmpWhileinfo, sizeof(WhileInfo), Size, SaveGameFile);
	for (int i = 0; i < Size; i++)
	{
		TmpW = (WhileInfo*)malloc(sizeof(WhileInfo));
		*TmpW = TmpWhileinfo[Size - i - 1];
		Stack_push(&whileStack, TmpW);
		//whileStack.push(TmpWhileinfo[Size - i - 1]);
	}
	free(TmpWhileinfo);
	Size = Stack_size(&buttonStack);
	fprintf(SaveGameFile, "%c", (char)Size);
	long* BTNSTmp = (long*)malloc(sizeof(long)*Size);
	long* TmpBTN=NULL;
	for (int i = 0; i < Size; i++)
	{
		BTNSTmp[i] = *(long*)Stack_top(&buttonStack);
		Stack_pop(&buttonStack);
	}
	fwrite(BTNSTmp, sizeof(long), Size, SaveGameFile);
	for (int i = 0; i < Size; i++)
	{
		TmpBTN = (long*)malloc(sizeof(long));
		*TmpBTN = BTNSTmp[Size - i - 1];
		Stack_push(&buttonStack, TmpBTN);
	}
	free(BTNSTmp);
	fwrite(ButtonInfo, sizeof(BTNInfo), 30, SaveGameFile);
	fwrite(GameFlag, sizeof(int), 100, SaveGameFile);
	fwrite(StrFlag, 1000, 10, SaveGameFile);
	fwrite(BackgroundPath,200, 1, SaveGameFile);
	fwrite(ImageInfo, sizeof(IMGInfo), 30, SaveGameFile);
	fwrite(BGMPath, 200, 1, SaveGameFile);
	long CurrentF = ftell(FileHandle);
	fwrite(&CurrentF, sizeof(long), 1, SaveGameFile);
	fwrite(&ChatInfo, sizeof(TextInfo), 1, SaveGameFile);
	fclose(SaveGameFile);
}

void LoadGame(char FilePath[])
{
	FILE* SaveGameFile = fopen(FilePath, "rb");
	if (!SaveGameFile)
		return;
	ClearGameState();
	char CSize;
	WhileInfo WI = { NULL };
	fscanf(SaveGameFile, "%c", &CSize);
	WhileInfo* TmpWhileInfo = NULL;
	for (int i=0;i<CSize;i++)
	{
		TmpWhileInfo = (WhileInfo*)malloc(sizeof(WhileInfo));
		fread(&WI, sizeof(WhileInfo), 1, SaveGameFile);
		*TmpWhileInfo = WI;
		Stack_push(&whileStack, TmpWhileInfo);
	}
	long BTNSTmp = 0;
	long* TmpBTN = NULL;
	fscanf(SaveGameFile, "%c", &CSize);
	for (int i = 0; i < CSize; i++)
	{
		TmpBTN = (long*)malloc(sizeof(long));
		fread(&BTNSTmp, sizeof(long), 1, SaveGameFile);
		*TmpBTN = BTNSTmp;
		Stack_push(&buttonStack, TmpBTN);
	}
	fread(ButtonInfo, sizeof(BTNInfo), 30, SaveGameFile);
	fread(GameFlag, sizeof(int), 100, SaveGameFile);
	fread(StrFlag, 1000, 10, SaveGameFile);
	fread(BackgroundPath, 200, 1, SaveGameFile);
	fread(ImageInfo, sizeof(IMGInfo), 30, SaveGameFile);
	fread(BGMPath, 200, 1, SaveGameFile);
	long CurrentF;
	fread(&CurrentF, sizeof(long), 1, SaveGameFile);
	fread(&ChatInfo, sizeof(TextInfo), 1, SaveGameFile);
	//重建资源
	WCHAR WCHARTmp[200];
	mbstowcs(WCHARTmp, BackgroundPath, 200);
	BackgroundBitmap = NULL;
	LoadD2DImage(WCHARTmp, &BackgroundBitmap);
	for (int i=0;i<30;i++)
	{
		if (ImageInfo[i].D2D1Bitmap)
		{
			mbstowcs(WCHARTmp, ImageInfo[i].ImagePath, 200);
			ImageInfo[i].D2D1Bitmap = NULL;
			LoadD2DImage(WCHARTmp, &ImageInfo[i].D2D1Bitmap);
		}
	}
	DWriteFactory->CreateTextFormat(ChatInfo.ChatStyle.FontName, NULL, DWRITE_FONT_WEIGHT_REGULAR, 
		(DWRITE_FONT_STYLE)((BOOL)(ChatInfo.ChatStyle.ItalicStyle) * 2), DWRITE_FONT_STRETCH_NORMAL, ChatInfo.ChatStyle.FontSize, TEXT("zh-cn"), &ChatInfo.D2DTextFormat);
	D2D1HwndRenderTarget->CreateSolidColorBrush(D2D1::ColorF(RGB(ChatInfo.ChatStyle.ColorR, ChatInfo.ChatStyle.ColorG, ChatInfo.ChatStyle.ColorB)), 
		(ID2D1SolidColorBrush**)&ChatInfo.D2DBrush);
	char BGMP[200];
	sprintf(BGMP, "open %s alias bgm", BGMPath);
	mciSendStringA(BGMP, NULL, 0, NULL);
	mciSendString(TEXT("play bgm repeat"), NULL, 0, NULL);
	fseek(FileHandle, CurrentF, SEEK_SET);
	fclose(SaveGameFile);
}

void ClearGameState()
{
	while (!Stack_IsEmpty(&whileStack))
		Stack_pop(&whileStack);
	while (!Stack_IsEmpty(&buttonStack))
		Stack_pop(&buttonStack);
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
	memset(BackgroundPath, 0, 200);
	mciSendString(TEXT("close bgm"), NULL, 0, NULL);
	PlaySoundA(NULL, NULL, NULL);
	for (int& F : GameFlag)
	{
		F = 0;
	}
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

inline void Stack_Init(StackData* SD)
{
	SD->pIndex = 0;
}

inline void Stack_push(StackData* SD, void* pOint)
{
	SD->pData[SD->pIndex] = pOint;
	SD->pIndex++;
}

inline BOOL Stack_IsEmpty(StackData* SD)
{
	if (SD->pIndex > 0)
		return FALSE;
	else
		return TRUE;
}

inline void* Stack_top(StackData* SD)
{
	return SD->pData[SD->pIndex - 1];
}

inline void Stack_pop(StackData* SD)
{
	free(SD->pData[SD->pIndex - 1]);
	SD->pData[SD->pIndex - 1] = NULL;
	SD->pIndex--;
}

inline int Stack_size(StackData* SD)
{
	return SD->pIndex;
}

inline float abs(float Value)
{
	if (Value > 0)
		return Value;
	else
		return -Value;
}