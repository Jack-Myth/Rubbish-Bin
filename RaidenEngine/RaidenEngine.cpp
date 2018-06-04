#define  _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <locale.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>
#include <stack>
#include <dshow.h>
#include <vector>
#include <map>
#include <xaudio2.h>
#include "BaseInformation.h"
#include "InlineFunction.h"
#include "FunctionLib.h"
#include <string>
#include <time.h>
#pragma comment(lib, "WINMM.LIB")
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "dwrite.lib")
#pragma comment(lib,"strmiids.lib")
#pragma comment(lib,"XAudio2.lib")

#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)

void InitEngine(HINSTANCE hInstance, int nShowCmd);
void InitGame();
void MessageLoop();
void ProcessGameLogic();
void RenderGame();
void SpawnAmmo(const char[], D2D1_POINT_2F,GameLogic::PawnBase* AmmoFrom = NULL);
void SpawnEnemy(const char[], D2D1_POINT_2F);
void SpawnItem(const char[], D2D1_POINT_2F);
template<typename T>
void DestroySelf(T* pp);
double func_rand(double minv,double maxv);
void ProcessPlayerInput();
void TerminateThread(std::vector<bool*> ThreadCollection, _In_ DWORD dwExitCode);
template<typename TSelf = GameLogic::VoidType, typename TTarget = GameLogic::VoidType>
std::thread* ScriptExecuter(char ScriptPath[], long ExecuteLocation, void* Self, void* Target, bool Async, std::vector<bool*>* ThreadSlot = NULL);
HRESULT InitWindowlessVMR(HWND hwndApp,IGraphBuilder* pGraph,IVMRWindowlessControl** ppWc);
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

BaseInformation Sys = { NULL };
std::map<std::string, GameLogic::AmmoBase*> AmmoTemplate;
std::vector<GameLogic::AmmoBase*> AmmoInstance;
std::map<std::string, GameLogic::ItemBase*> ItemTemplate;
std::vector<GameLogic::ItemBase*> ItemInstance;
std::map<std::string, GameLogic::PawnBase*> PawnTemplate;
std::vector<GameLogic::PawnBase*> PawnInstance;
std::vector<D2DFrameAnimation*> FrameAnimationCollection;
HANDLE EnemyClearedEvent=NULL;
exprtk::symbol_table<double> GlobalGameFlag;
std::vector<double*> GlobalGameFlagInstance;
int wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	InitEngine(hInstance, nShowCmd);
	InitGame();
	ScriptExecuter(".\\Script", 0, NULL, NULL, true,NULL);
	MessageLoop();
	return 0;
}

void InitEngine(HINSTANCE hInstance,int nShowCmd)
{
	//Begin Init Wndow for Game Viewport
	RECT WndRC = { 0,0,384,768 };
	WNDCLASSEX wnd = { NULL };
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.hCursor = LoadCursor(0, IDC_ARROW);
	wnd.hIcon = LoadIcon(0, IDC_ICON);
	wnd.hInstance = hInstance;
	wnd.lpfnWndProc = MainWndProc;
	wnd.lpszClassName = TEXT("RAIDEN ENGINE VIEWPORT");
	RegisterClassEx(&wnd);
	AdjustWindowRect(&WndRC, WS_OVERLAPPEDWINDOW, FALSE);
	Sys.ViewportHWND = CreateWindow(wnd.lpszClassName, TEXT("RAIDEN ENGINE"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WndRC.right- WndRC.left, WndRC.bottom- WndRC.top, NULL, NULL, hInstance, NULL);
	CheckAndPostErrorMsg(!Sys.ViewportHWND, TEXT("Create Window as Viewport Failed,Exit..."));
	ShowWindow(Sys.ViewportHWND, nShowCmd);
	//End Init Window For Game Viewport
	//Begin Init Direct2D
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &Sys.D2D1Factory);
	CheckAndPostErrorMsg(!Sys.D2D1Factory, TEXT("D2D1Factory Failed to Create,Exit..."));
	Sys.D2D1Factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT), D2D1::HwndRenderTargetProperties(Sys.ViewportHWND, D2D1::SizeU(WndRC.right- WndRC.left, WndRC.bottom- WndRC.top)), &Sys.D2D1HwndRenderTarget);
	CheckAndPostErrorMsg(!Sys.D2D1HwndRenderTarget, TEXT("D2D1 RenderTarget Failed to Create,Exit..."));
	//End Init Direct2D
	//Begin Init WICImageComponent
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&Sys.WICImagingFactory));
	CheckAndPostErrorMsg(!Sys.WICImagingFactory, TEXT("WICImageComponent Failed to Create,Exit..."));
	//End Init WICImageComponent
	//Begin Init DWrite
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&Sys.DWriteFactory));
	Sys.DWriteFactory->CreateTextFormat(TEXT("微软雅黑"), NULL,
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, 20, TEXT("zh-cn"), &Sys.BaseTextFormat);
	CheckAndPostErrorMsg(!Sys.BaseTextFormat, TEXT("Base Text Format Failed to Create,Exit..."));
	//End Init DWrite
	//Begin Init XAudio2
	CheckAndPostErrorMsg(FAILED(XAudio2Create(&Sys.XAudio2Engine)), TEXT("XAudio2 Component Failed To Create,Exit..."));
	CheckAndPostErrorMsg(FAILED(Sys.XAudio2Engine->CreateMasteringVoice(&Sys.XAudio2MasteringVoice)), TEXT("XAudio2 MasteringVoice Failed To Create,Exit..."));
	//End Init XAudio2
	//Begin Init DShow
	CoCreateInstance(CLSID_FilterGraph, NULL, CLSCTX_INPROC_SERVER, IID_IGraphBuilder, (void **)&Sys.DirectShowInfo.pGraph);
	Sys.DirectShowInfo.pGraph->QueryInterface(IID_IMediaControl, (void **)&Sys.DirectShowInfo.pControl);
	Sys.DirectShowInfo.pGraph->QueryInterface(IID_IMediaEvent, (void **)&Sys.DirectShowInfo.pEvent);
	Sys.DirectShowInfo.pGraph->QueryInterface(IID_IMediaSeeking, (void **)&Sys.DirectShowInfo.pSeeking);
	InitWindowlessVMR(Sys.ViewportHWND, Sys.DirectShowInfo.pGraph, &Sys.DirectShowInfo.pWindowLessControl);
	//End Init DShow
	//Set Timer For FrameTick
	SetTimer(Sys.ViewportHWND, 0, 10, NULL);
}

void InitGame()
{
	EnemyClearedEvent=CreateEvent(NULL, TRUE, FALSE, NULL);
	srand(time(NULL));
	GlobalGameFlag.add_function(std::string("rand"), func_rand);
}

LRESULT  CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		return DefWindowProc(hwnd, message, wParam, lParam);
	case WM_TIMER:
		ProcessGameLogic();
		RenderGame();
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}


void MessageLoop()
{
	MSG msg = { NULL };
	while (msg.message != WM_QUIT)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		GetMessage(&msg, 0, 0, 0);
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

template<typename TSelf, typename TTarget>
std::thread* ScriptExecuter(char ScriptPath[],long ExecuteLocation,void* Self,void* Target,bool Async,std::vector<bool*>* ThreadSlot)
{
	auto ExecuterLamdba=[ScriptPath, ExecuteLocation, Self,Target, Async, ThreadSlot]()
	{
		int CurrentSlot = 0;
		bool ExitSignal=false;
		if (Async&&ThreadSlot)
		{
			CurrentSlot = (int)(ThreadSlot->size() - 1);
			(*ThreadSlot)[CurrentSlot] = &ExitSignal;
		}
		FILE* FileHandle = fopen(ScriptPath, "r+");
		if (!FileHandle)
		{
			printf("Script Load Error, Executer Will Exit");
			return;
		}
		fseek(FileHandle, ExecuteLocation, SEEK_SET);
		std::stack<WhileInfo> whileStack;
		std::stack<long> subStack;
		char Command[50] = { NULL };
		float ArgFloat[5] = { 0 };
		int ArgInt[5] = { 0 };
		char ArgStr[10][1024] = { NULL };
		char ArgLongStr[1000] = { NULL };
		WCHAR WCharTMP[100] = { NULL };
		//int GameFlag[100] = { 0 };
		exprtk::symbol_table<double> GameFlag;
		std::vector<double*> VariablesInstance;
		char StrFlag[20][1000] = { 0 };
		long LastStreamIndex;
		do
		{
			if (ExitSignal)
			{
				//资源释放
				for (auto it=VariablesInstance.begin();it<VariablesInstance.end();++it)
					delete *it;
				return;
			}
			if ((Command[0] = fgetc(FileHandle)) != '\n'&&Command[0] != ' '&&Command[0] != '\t')
				fseek(FileHandle, -1, SEEK_CUR);
			else
				continue;
			if (fgetc(FileHandle) != '#')
				fseek(FileHandle, -1, SEEK_CUR);
			else
			{
				fseek(FileHandle, -1, SEEK_CUR);
				fscanf(FileHandle, "%*[^\n]");
				continue;
			}
			LastStreamIndex = ftell(FileHandle);
			fscanf(FileHandle, "%[^(]", Command);
			if (!strcmp(Command, "AddAmmoType"))
			{
				fscanf(FileHandle, "(%[^)])", ArgStr[0]);
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[1], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[1], StrFlag[i]);
				}
				AmmoTemplate.insert_or_assign(std::string(ArgStr[0]),new GameLogic::AmmoBase());
			}
			//开始处理Ammo的相关语句
			else if (!strcmp(Command, "SetAmmoInfo"))	
			{//Ammo名称，Ammo攻击力，大小，移动速度，图片
				fscanf(FileHandle, "(%[^,],%[^,],%[^,],%[^,],%[^)])",ArgStr[0], ArgStr[1], ArgStr[2], ArgStr[3], ArgStr[4]);
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[5], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[5], StrFlag[i]);
					StrReplace(ArgStr[4], ArgStr[5], StrFlag[i]);
				}
				auto TmpAmmoTemplate = AmmoTemplate.find(std::string(ArgStr[0]))->second;
				TmpAmmoTemplate->Attack = CaculateExpression(ArgStr[1],GameFlag);
				TmpAmmoTemplate->AttackRadius = (float)CaculateExpression(ArgStr[2],GameFlag)/1000;
				TmpAmmoTemplate->Speed = (float)CaculateExpression(ArgStr[3],GameFlag) / 1000;
				LoadD2DImage(ArgStr[4], &(TmpAmmoTemplate->AmmoImage));
			}
			else if (!strcmp(Command, "SetAmmoEffect"))
			{
				fscanf(FileHandle, "(%[^,],%[^,],%[^,],%[^,]", ArgStr[0], ArgStr[1], ArgStr[2], ArgStr[3]); //第一遍输入,名称，序列帧路径,序列帧XY轴帧数
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[4], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[4], StrFlag[i]);
					StrReplace(ArgStr[1], ArgStr[4], StrFlag[i]);
				}
				auto TmpAmmoTemplate = AmmoTemplate.find(std::string(ArgStr[0]))->second;
				int XC = CaculateExpression(ArgStr[2], GameFlag), YC = CaculateExpression(ArgStr[3], GameFlag);
				LoadD2DImage(ArgStr[1], &TmpAmmoTemplate->HittedAnimation.AnimationBitmap);
				TmpAmmoTemplate->HittedAnimation.XCount = XC;
				TmpAmmoTemplate->HittedAnimation.YCount = YC;
				fscanf(FileHandle, ",%[^,],%[^,],%[^,],%[^)])", ArgStr[0], ArgStr[1], ArgStr[2], ArgStr[3]);//第二遍输入，序列帧渲染大小(屏幕千分比)，每秒帧数,音频文件的路径
				TmpAmmoTemplate->HittedAnimation.RenderSize = { (float)CaculateExpression(ArgStr[0],GameFlag) / 1000,(float)CaculateExpression(ArgStr[1],GameFlag) / 1000 };
				TmpAmmoTemplate->HittedAnimation.FramePerSecond = CaculateExpression(ArgStr[2], GameFlag);
				TmpAmmoTemplate->HittedAnimation.MaxDelay = 1000 / TmpAmmoTemplate->HittedAnimation.FramePerSecond / 10;
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[4], "str[%d]", i);
					StrReplace(ArgStr[3], ArgStr[4], StrFlag[i]);
				}
				LoadSound(ArgStr[3], &TmpAmmoTemplate->HittedSound);
			}
			else if (!strcmp(Command, "SetAmmoRootScript"))
			{
				fscanf(FileHandle, "(%[^,],%[^,],%[^,],%[^)])", ArgStr[0], ArgStr[1],ArgStr[2], ArgStr[3]);
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[4], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[4], StrFlag[i]);
					StrReplace(ArgStr[1], ArgStr[4], StrFlag[i]);
					StrReplace(ArgStr[2], ArgStr[4], StrFlag[i]);
				}
				auto TmpAmmoTemplate = AmmoTemplate.find(std::string(ArgStr[0]))->second;
				TmpAmmoTemplate->RootScript = GetExecutePoint(ArgStr[1], ArgStr[2]);
				TmpAmmoTemplate->RootScript.ShouldAsync = (bool)CaculateExpression(ArgStr[3], GameFlag);
			}
			else if (!strcmp(Command, "SetAmmoOnHited"))
			{
				fscanf(FileHandle, "(%[^,],%[^,],%[^,],%[^)])", ArgStr[0], ArgStr[1], ArgStr[2], ArgStr[3]);
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[4], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[4], StrFlag[i]);
					StrReplace(ArgStr[1], ArgStr[4], StrFlag[i]);
					StrReplace(ArgStr[2], ArgStr[4], StrFlag[i]);
				}
				auto TmpAmmoTemplate = AmmoTemplate.find(std::string(ArgStr[0]))->second;
				TmpAmmoTemplate->OnHited = GetExecutePoint(ArgStr[1], ArgStr[2]);
				TmpAmmoTemplate->OnHited.ShouldAsync= (bool)CaculateExpression(ArgStr[3], GameFlag);
			}
			else if (!strcmp(Command, "SpawnAmmo"))
			{
				fscanf(FileHandle, "(%[^,],%[^,],%[^)])", ArgStr[0], ArgStr[1], ArgStr[2]);
				int SA_X= CaculateExpression(ArgStr[1], GameFlag);
				int SA_Y=CaculateExpression(ArgStr[2], GameFlag);
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[3], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[3], StrFlag[i]);
				}
				SpawnAmmo(ArgStr[0],D2D1::Point2F((float)SA_X/1000,(float)SA_Y/1000),(GameLogic::PawnBase*)Self);
			}
			//开始处理Pawn的相关语句
			else if (!strcmp(Command, "AddPawnType"))
			{
				fscanf(FileHandle, "(%[^)])", ArgStr[0]);
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[1], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[1], StrFlag[i]);
				}
				PawnTemplate.insert_or_assign(std::string(ArgStr[0]), new GameLogic::PawnBase());
			}
			else if (!strcmp(Command, "SetPawnInfo"))
			{//Pawn名称，射击延迟（感觉放这里好像不太对），Pawn体积，移动速度，最大生命值,图片
				fscanf(FileHandle, "(%[^,],%[^,],%[^,],%[^,],%[^,],%[^)])", ArgStr[0], ArgStr[1], ArgStr[2], ArgStr[3], ArgStr[4],ArgStr[5]);
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[6], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[6], StrFlag[i]);
					StrReplace(ArgStr[5], ArgStr[6], StrFlag[i]);
				}
				auto TmpPawnTemplate = PawnTemplate.find(std::string(ArgStr[0]))->second;
				TmpPawnTemplate->ShootDelayMax = CaculateExpression(ArgStr[1],GameFlag);
				TmpPawnTemplate->BodyRadius = (float)CaculateExpression(ArgStr[2], GameFlag)/1000;
				TmpPawnTemplate->Speed = (float)CaculateExpression(ArgStr[3], GameFlag)/1000;
				TmpPawnTemplate->Health = CaculateExpression(ArgStr[4],GameFlag);
				LoadD2DImage(ArgStr[5], &(TmpPawnTemplate->PawnImage));
			}
			else if (!strcmp(Command, "SetPawnAmmo"))
			{//Pawn名称，Ammo的名称
				fscanf(FileHandle, "(%[^,],%[^)])", ArgStr[0], ArgStr[1]);
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[5], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[5], StrFlag[i]);
					StrReplace(ArgStr[1], ArgStr[5], StrFlag[i]);
				}
				auto TmpPawnTemplate = PawnTemplate.find(std::string(ArgStr[0]))->second;
				TmpPawnTemplate->WeaponAmmo = std::string(ArgStr[1]);
			}
			else if (!strcmp(Command, "SetPawnEffect"))
			{
				fscanf(FileHandle, "(%[^,],%[^,],%[^,],%[^,]", ArgStr[0], ArgStr[1], ArgStr[2], ArgStr[3]); //第一遍输入,名称，序列帧路径,序列帧XY轴帧数
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[4], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[4], StrFlag[i]);
					StrReplace(ArgStr[1], ArgStr[4], StrFlag[i]);
				}
				auto TmpPawnTemplate = PawnTemplate.find(std::string(ArgStr[0]))->second;
				int XC = CaculateExpression(ArgStr[2], GameFlag), YC = CaculateExpression(ArgStr[3], GameFlag);
				LoadD2DImage(ArgStr[1], &TmpPawnTemplate->DeadAnimation.AnimationBitmap);
				TmpPawnTemplate->DeadAnimation.XCount = XC;
				TmpPawnTemplate->DeadAnimation.YCount = YC;
				fscanf(FileHandle, ",%[^,],%[^,],%[^,],%[^)])", ArgStr[0], ArgStr[1], ArgStr[2], ArgStr[3]);//第二遍输入，序列帧渲染大小(屏幕千分比)，每秒帧数,音频文件的路径
				TmpPawnTemplate->DeadAnimation.RenderSize = { (float)CaculateExpression(ArgStr[0],GameFlag) / 1000,(float)CaculateExpression(ArgStr[1],GameFlag) / 1000 };
				TmpPawnTemplate->DeadAnimation.FramePerSecond = CaculateExpression(ArgStr[2], GameFlag);
				TmpPawnTemplate->DeadAnimation.MaxDelay = 1000 / TmpPawnTemplate->DeadAnimation.FramePerSecond / 10;
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[4], "str[%d]", i);
					StrReplace(ArgStr[3], ArgStr[4], StrFlag[i]);
				}
				LoadSound(ArgStr[3], &TmpPawnTemplate->DeadSound);
			}
			else if (!strcmp(Command, "SetPawnRootScript"))
			{
				fscanf(FileHandle, "(%[^,],%[^,],%[^,],%[^)])", ArgStr[0], ArgStr[1], ArgStr[2], ArgStr[3]);
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[4], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[4], StrFlag[i]);
					StrReplace(ArgStr[1], ArgStr[4], StrFlag[i]);
					StrReplace(ArgStr[2], ArgStr[4], StrFlag[i]);
				}
				auto TmpPawnTemplate = PawnTemplate.find(std::string(ArgStr[0]))->second;
				TmpPawnTemplate->RootScript = GetExecutePoint(ArgStr[1], ArgStr[2]);
				TmpPawnTemplate->RootScript.ShouldAsync= (bool)CaculateExpression(ArgStr[3], GameFlag);
			}
			else if (!strcmp(Command, "SetPawnOnDamaged"))
			{
				fscanf(FileHandle, "(%[^,],%[^,],%[^,],%[^)])", ArgStr[0], ArgStr[1], ArgStr[2], ArgStr[3]);
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[4], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[4], StrFlag[i]);
					StrReplace(ArgStr[1], ArgStr[4], StrFlag[i]);
					StrReplace(ArgStr[2], ArgStr[4], StrFlag[i]);
				}
				auto TmpPawnTemplate = PawnTemplate.find(std::string(ArgStr[0]))->second;
				TmpPawnTemplate->OnDamaged = GetExecutePoint(ArgStr[1], ArgStr[2]);
				TmpPawnTemplate->OnDamaged.ShouldAsync = (bool)CaculateExpression(ArgStr[3], GameFlag);
			}
			else if (!strcmp(Command, "SetPawnOnDead"))
			{
				fscanf(FileHandle, "(%[^,],%[^,],%[^,],%[^)])", ArgStr[0], ArgStr[1], ArgStr[2], ArgStr[3]);
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[4], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[4], StrFlag[i]);
					StrReplace(ArgStr[1], ArgStr[4], StrFlag[i]);
					StrReplace(ArgStr[2], ArgStr[4], StrFlag[i]);
				}
				auto TmpPawnTemplate = PawnTemplate.find(std::string(ArgStr[0]))->second;
				TmpPawnTemplate->OnDead = GetExecutePoint(ArgStr[1], ArgStr[2]);
				TmpPawnTemplate->OnDead.ShouldAsync = (bool)CaculateExpression(ArgStr[3], GameFlag);
			}
			else if (!strcmp(Command, "SetPawnWeaponAmmo"))
			{
				fscanf(FileHandle, "(%[^,],%[^)])", ArgStr[0], ArgStr[1]);
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[2], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[2], StrFlag[i]);
					StrReplace(ArgStr[1], ArgStr[2], StrFlag[i]);
				}
				auto TmpPawnTemplate = PawnTemplate.find(std::string(ArgStr[0]))->second;
				TmpPawnTemplate->WeaponAmmo = std::string(ArgStr[1]);
			}
			else if (!strcmp(Command, "SpawnEnemy"))
			{
				fscanf(FileHandle, "(%[^,],%[^,],%[^)])", ArgStr[0], ArgStr[1], ArgStr[2]);
				int SA_X = CaculateExpression(ArgStr[1], GameFlag);
				int SA_Y = CaculateExpression(ArgStr[2], GameFlag);
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[3], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[3], StrFlag[i]);
				}
				SpawnEnemy(ArgStr[0], D2D1::Point2F((float)SA_X / 1000, (float)SA_Y / 1000));
			}
			//开始处理Item的相关语句
			else if (!strcmp(Command, "AddItemType"))
			{
				fscanf(FileHandle, "(%[^)])", ArgStr[0]);
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[1], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[1], StrFlag[i]);
				}
				ItemTemplate.insert_or_assign(std::string(ArgStr[0]), new GameLogic::ItemBase());
			}
			else if (!strcmp(Command, "SetItemInfo"))
			{
				fscanf(FileHandle, "(%[^,],%[^,],%[^)])", ArgStr[0],ArgStr[1], ArgStr[2]);
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[3], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[3], StrFlag[i]);
					StrReplace(ArgStr[2], ArgStr[3], StrFlag[i]);
				}
				auto TmpItemTemplate = ItemTemplate.find(std::string(ArgStr[0]))->second;
				TmpItemTemplate->Speed = (float)CaculateExpression(ArgStr[1], GameFlag) / 1000;
				LoadD2DImage(ArgStr[1], &(TmpItemTemplate->ItemImage));
			}
			else if (!strcmp(Command, "SetItemEffect"))
			{
				fscanf(FileHandle, "(%[^,],%[^,],%[^,],%[^,]", ArgStr[0], ArgStr[1], ArgStr[2], ArgStr[3]); //第一遍输入,名称，序列帧路径,序列帧XY轴帧数
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[4], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[4], StrFlag[i]);
					StrReplace(ArgStr[1], ArgStr[4], StrFlag[i]);
				}
				auto TmpItemTemplate = ItemTemplate.find(std::string(ArgStr[0]))->second;
				int XC = CaculateExpression(ArgStr[2],GameFlag), YC = CaculateExpression(ArgStr[3],GameFlag);
				LoadD2DImage(ArgStr[1], &TmpItemTemplate->PickedAnimation.AnimationBitmap);
				TmpItemTemplate->PickedAnimation.XCount = XC;
				TmpItemTemplate->PickedAnimation.YCount = YC;
				fscanf(FileHandle, ",%[^,],%[^,],%[^,],%[^)])", ArgStr[0], ArgStr[1], ArgStr[2], ArgStr[3]);//第二遍输入，序列帧渲染大小(屏幕千分比)，每秒帧数,音频文件的路径
				TmpItemTemplate->PickedAnimation.RenderSize = { (float)CaculateExpression(ArgStr[0],GameFlag) / 1000,(float)CaculateExpression(ArgStr[1],GameFlag) / 1000 };
				TmpItemTemplate->PickedAnimation.FramePerSecond = CaculateExpression(ArgStr[2], GameFlag);
				TmpItemTemplate->PickedAnimation.MaxDelay = 1000 / TmpItemTemplate->PickedAnimation.FramePerSecond / 10;
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[4], "str[%d]", i);
					StrReplace(ArgStr[3], ArgStr[4], StrFlag[i]);
				}
				LoadSound(ArgStr[3], &TmpItemTemplate->PickedSound);
			}
			else if (!strcmp(Command, "SetItemRootScript"))
			{
				fscanf(FileHandle, "(%[^,],%[^,],%[^,],%[^)])", ArgStr[0], ArgStr[1], ArgStr[2], ArgStr[3]);
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[4], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[4], StrFlag[i]);
					StrReplace(ArgStr[1], ArgStr[4], StrFlag[i]);
					StrReplace(ArgStr[2], ArgStr[4], StrFlag[i]);
				}
				auto TmpItemTemplate =ItemTemplate.find(std::string(ArgStr[0]))->second;
				TmpItemTemplate->RootScript = GetExecutePoint(ArgStr[1], ArgStr[2]);
				TmpItemTemplate->RootScript.ShouldAsync= (bool)CaculateExpression(ArgStr[3], GameFlag);
			}
			else if (!strcmp(Command, "SetItemOnPickedUp"))
			{
				fscanf(FileHandle, "(%[^,],%[^,],%[^,],%[^)])", ArgStr[0], ArgStr[1], ArgStr[2], ArgStr[3]);
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[3], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[3], StrFlag[i]);
					StrReplace(ArgStr[1], ArgStr[3], StrFlag[i]);
					StrReplace(ArgStr[2], ArgStr[3], StrFlag[i]);
				}
				auto TmpItemTemplate = ItemTemplate.find(std::string(ArgStr[0]))->second;
				TmpItemTemplate->OnPickedUp = GetExecutePoint(ArgStr[1], ArgStr[2]);
				TmpItemTemplate->OnPickedUp.ShouldAsync= (bool)CaculateExpression(ArgStr[3], GameFlag);
			}
			else if (!strcmp(Command, "SpawnItem"))
			{
				fscanf(FileHandle, "(%[^,],%[^,],%[^)])", ArgStr[0], ArgStr[1], ArgStr[2]);
				int SA_X = CaculateExpression(ArgStr[1], GameFlag);
				int SA_Y = CaculateExpression(ArgStr[2], GameFlag);
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[3], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[3], StrFlag[i]);
				}
				SpawnItem(ArgStr[0], D2D1::Point2F((float)SA_X / 1000, (float)SA_Y / 1000));
			}
			//游戏相关
			else if (!strcmp(Command, "SetBackground"))
			{
				fscanf(FileHandle, "(%[^)])", ArgStr[0]);
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[3], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[3], StrFlag[i]);
				}
				LoadD2DImage(ArgStr[0],&GameLogic::GameBackground);
			}
			else if (!strcmp(Command, "SetFlySpeed"))
			{
				fscanf(FileHandle, "(%[^)])", ArgStr[0]);
				GameLogic::FlySpeed = (float)CaculateExpression(ArgStr[0],GameFlag)/1000;
			}
			else if (!strcmp(Command, "SetDefPlayerPawn"))
			{
				fscanf(FileHandle, "(%[^)])", &ArgStr[0]);
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[3], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[3], StrFlag[i]);
				}
				GameLogic::DefPlayerPawn= PawnTemplate.find(std::string(ArgStr[0]))->second;
				if (!GameLogic::Players[0].PlayerPawn)
				{
					GameLogic::Players[0].PlayerPawn = new GameLogic::PawnBase(*GameLogic::DefPlayerPawn);
					GameLogic::Players[0].PlayerPawn->RootScript.Execute<GameLogic::PawnBase, GameLogic::VoidType>(GameLogic::Players[0].PlayerPawn, NULL);
				}
			}
			else if (!strcmp(Command, "SetHealth"))
			{
				fscanf(FileHandle, "(%[^,],%[^)])", ArgStr[0], ArgStr[1]);
				if (CaculateExpression(ArgStr[0], GameFlag) && Self)
					((GameLogic::PawnBase*)Self)->Health = CaculateExpression(ArgStr[1], GameFlag);
				else if (Target)
					((GameLogic::PawnBase*)Target)->Health = CaculateExpression(ArgStr[1],GameFlag);
			}
			else if (!strcmp(Command, "GetHealth"))
			{
				fscanf(FileHandle, "(%[^,],%[^,])", ArgStr[0],ArgStr[1]);
				if (CaculateExpression(ArgStr[0], GameFlag) && Self)
					GameFlag.get_variable(std::string(ArgStr[0]))->ref() = ((GameLogic::PawnBase*)Self)->Health;
				else if (Target)
					GameFlag.get_variable(std::string(ArgStr[0]))->ref() =((GameLogic::PawnBase*)Target)->Health;
			}
			else if (!strcmp(Command,"SetEnabled"))
			{
				fscanf(FileHandle, "(%[^,],%[^)])", ArgStr[0], ArgStr[1]);
				if (CaculateExpression(ArgStr[0], GameFlag) && Self)
					((TSelf*)Self)->Enabled= CaculateExpression(ArgStr[1], GameFlag);
				else if (Target)
					((TTarget*)Target)->Enabled=CaculateExpression(ArgStr[1],GameFlag);
			}
			else if (!strcmp(Command, "SetLocation"))
			{
				fscanf(FileHandle, "(%[^,],%[^,],%[^)])",ArgStr[0], ArgStr[1], ArgStr[2]);
				LogicLock.lock();
				if (CaculateExpression(ArgStr[0], GameFlag) && Self)
					((TSelf*)Self)->Location = { (float)CaculateExpression(ArgStr[1],GameFlag) / 1000.f,(float)CaculateExpression(ArgStr[2],GameFlag) / 1000.f };
				else if (Target)
					//{
					//	bool IsPlayer = false;
					//	for (auto it = GameLogic::PlayerCollection.begin(); it != GameLogic::PlayerCollection.end(); it++)
					//		if (&*it==Target)
					//		{
					//			((GameLogic::PlayerInfo*)Target)->PlayerPawn->Location= { (float)CaculateExpression(ArgStr[1],GameFlag) / 1000.f,(float)CaculateExpression(ArgStr[2],GameFlag) / 1000.f };
					//			IsPlayer = true;
					//		}
					//	if (!IsPlayer)
					((TTarget*)Target)->Location = { (float)CaculateExpression(ArgStr[1],GameFlag) / 1000.f,(float)CaculateExpression(ArgStr[2],GameFlag) / 1000.f };
				LogicLock.unlock();
				//}
			}
			else if (!strcmp(Command, "GetLocation"))
			{
				fscanf(FileHandle, "(%[^,],%[^,],%[^)])", ArgStr[0], ArgStr[1], ArgStr[2]);
				if (CaculateExpression(ArgStr[0], GameFlag) && Self)
				{
					//LogicLock.lock();
					GameFlag.get_variable(std::string(ArgStr[1]))->ref()=((TSelf*)Self)->Location.x * 1000;
					GameFlag.get_variable(std::string(ArgStr[2]))->ref() = ((TSelf*)Self)->Location.y * 1000;
					//LogicLock.unlock();
				}
				else if (Target)
				{
					//bool IsPlayer = false;
					//for (auto it = GameLogic::PlayerCollection.begin(); it != GameLogic::PlayerCollection.end(); it++)
					//	if (&*it == Target)
					//	{
					//		GameFlag[ArgInt[0]] = (int)(((GameLogic::PlayerInfo*)Target)->PlayerPawn->Location.x * 1000);
					//		GameFlag[ArgInt[1]] = (int)(((GameLogic::PlayerInfo*)Target)->PlayerPawn->Location.y * 1000);
					//		IsPlayer = true;
					//	}
					//if (!IsPlayer)
					//{
					//LogicLock.lock();
					GameFlag.get_variable(std::string(ArgStr[1]))->ref() = ((TTarget*)Target)->Location.x * 1000;
					GameFlag.get_variable(std::string(ArgStr[2]))->ref() = ((TTarget*)Target)->Location.y * 1000;
					//LogicLock.unlock();
					//}
				}
			}
			else if (!strcmp(Command, "SetAmmoDirection"))
			{
				fscanf(FileHandle, "(%[^,],%[^)])", ArgStr[0], ArgStr[1]);
				if (Self)
					((GameLogic::AmmoBase*)Self)->MoveDirection = GetUnitVector(D2D1::Point2F(CaculateExpression(ArgStr[0], GameFlag), CaculateExpression(ArgStr[1], GameFlag)));
			}
			else if (!strcmp(Command, "WaitForEnemyCleared"))
			{
				fscanf(FileHandle, "%*s");
				WaitForSingleObject(EnemyClearedEvent, INFINITE);
			}
			else if (!strcmp(Command, "DestroySelf"))
			{
				fscanf(FileHandle, "%*s");
				DestroySelf<TSelf>((TSelf*)Self);
			}
			else if (!strcmp(Command, "PlayBGM"))
			{
				fscanf(FileHandle, "(%[^,],%[^)])", ArgStr[0], ArgStr[1]);
				for (int i = 0; i < 10; i++)
				{
					sprintf(ArgStr[2], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[2], StrFlag[i]);
				}
				sprintf(ArgStr[2], "open %s alias bgm", ArgStr[0]);
				mciSendString(TEXT("close bgm"), NULL, 0, NULL);
				mciSendStringA(ArgStr[2], NULL, 0, NULL);
				if (CaculateExpression(ArgStr[1],GameFlag))
					mciSendString(TEXT("play bgm"), NULL, 0, NULL);
				else
					mciSendString(TEXT("play bgm repeat"), NULL, 0, NULL);
			}
			else if (!strcmp(Command, "StopBGM"))
			{
				fscanf(FileHandle, "%*s");
				mciSendString(TEXT("close bgm"), NULL, 0,NULL);
			}
			else if (!strcmp(Command, "PlayVideo"))
			{
				long VHeight, VWidth;
				fscanf(FileHandle, "(%[^)])", ArgStr[0]);
				for (int i = 0; i < 10; i++)
				{
					sprintf(ArgStr[1], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[1], StrFlag[i]);
				}
				mbstowcs(WCharTMP, ArgStr[0], 200);
				Sys.DirectShowInfo.pGraph->RenderFile(WCharTMP, NULL);
				RECT SourceR;
				Sys.DirectShowInfo.pWindowLessControl->GetNativeVideoSize(&VWidth, &VHeight, NULL, NULL);
				SourceR = { 0,0,VWidth,VHeight };
				RECT WindowRect = { NULL };
				GetClientRect(Sys.ViewportHWND, &WindowRect);
				Sys.DirectShowInfo.pWindowLessControl->SetVideoPosition(&SourceR, &WindowRect);
				Sys.DirectShowInfo.pControl->Run();
			}
			else if (!strcmp(Command, "StopVideo"))
			{
				fscanf(FileHandle, "%*s");
				Sys.DirectShowInfo.pControl->Stop();
			}
			else if (!strcmp(Command, "ExecuteScript"))
			{//Script路径，标签，是否应当以异步方式执行，对象销毁时是否销毁此执行器
				fscanf(FileHandle, "(%[^,],%[^,],%[^,],%[^)])", ArgStr[0], ArgStr[1], ArgStr[2],ArgStr[3]);
				for (int i = 0; i < 10; i++)
				{
					sprintf(ArgStr[4], "str[%d]", i);
					StrReplace(ArgStr[0], ArgStr[4], StrFlag[i]);
					StrReplace(ArgStr[1], ArgStr[4], StrFlag[i]);
				}
				auto EXECPoint = GetExecutePoint(ArgStr[0], ArgStr[1]);
				if (CaculateExpression(ArgStr[2], GameFlag))
				{
					if (CaculateExpression(ArgStr[2], GameFlag))
					{
						ThreadSlot->push_back(NULL);
						ScriptExecuter<TSelf, TTarget>(ArgStr[0], EXECPoint.ScriptPoint, Self, Target, true, ThreadSlot) -> detach();
					}
					else
						ScriptExecuter<TSelf, TTarget>(ArgStr[0], EXECPoint.ScriptPoint, Self, Target, true, NULL) -> detach();
				}
				else
					ScriptExecuter<TSelf, TTarget>(ArgStr[0], EXECPoint.ScriptPoint, Self, Target, false);
			}
			/*else if (!strcmp(Command, "setflag"))////////////////////////////////////////////////////////////////////非功能逻辑控制语句///////////////////////////////////////////////////////////////////////////
			{
				fscanf(FileHandle, "(%d,%[^)])", &ArgInt[0], ArgStr[0]);
				if (ArgInt[0] < 100 && ArgInt[0] >= 0)
					GameFlag[ArgInt[0]] = CaculateExpression(ArgStr[0],GameFlag);
			}
			else if (!strcmp(Command, "setglobalflag"))
			{
				fscanf(FileHandle, "(%d,%[^)])", &ArgInt[0], ArgStr[0]);
				if (ArgInt[0] < 1000 && ArgInt[0] >= 0)
					GlobalGameFlag[ArgInt[0]] = CaculateExpression(ArgStr[0], GameFlag);
			}
			else if (!strcmp(Command, "getglobalflag"))
			{
				fscanf(FileHandle, "(%d,%d)", &ArgInt[0], &ArgInt[1]);
				if (ArgInt[0] < 1000 && ArgInt[0] >= 0 && ArgInt[1] >= 0 && ArgInt[1] < 100)
					GameFlag[ArgInt[1]] = GlobalGameFlag[ArgInt[0]];
			}*/
			else if (!strcmp(Command, "val"))
			{
				fscanf(FileHandle, "(%[^)])", ArgStr[0]);
				if (!GameFlag.symbol_exists(std::string(ArgStr[0])))
				{
					GlobalGameFlagInstance.push_back(&(*new double=0));
					GlobalGameFlag.add_variable(std::string(ArgStr[0]), *GlobalGameFlagInstance.back());
				}
				//GlobalGameFlag.add_variable(std::string(ArgStr[0]), GlobalGameFlagInstance[GlobalGameFlagInstance.size() - 1]);
			}
			else if (!strcmp(Command, "var"))
			{
				fgetc(FileHandle);
				do 
				{
					fscanf(FileHandle, "%[^,)]", ArgStr[0]);
					//VariablesInstance.push_back(0);
					if (!GameFlag.symbol_exists(std::string(ArgStr[0])))
					{
						VariablesInstance.push_back(&(*new double = 0));
						GameFlag.add_variable(std::string(ArgStr[0]), *VariablesInstance.back());
					}
				} while (fgetc(FileHandle)!=')');
			}
			else if (!strcmp(Command, "e"))
			{
				fscanf(FileHandle, "(%[^)])", ArgStr[0]);
				CaculateExpression(ArgStr[0], GameFlag);
			}
			else if (!strcmp(Command, "setstrflag"))
			{
				fscanf(FileHandle, "(%d,%[^)])", &ArgInt[0], ArgLongStr);
				for (int i = 0; i < 20; i++)
				{
					sprintf(ArgStr[1], "str[%d]", i);
					StrReplace(ArgLongStr, ArgStr[1], StrFlag[i]);
				}
				if (ArgInt[0] < 20 && ArgInt[0] >= 0)
					strcpy(StrFlag[ArgInt[0]], ArgLongStr);
			}
			else if (!strcmp(Command, "if"))
			{
				fscanf(FileHandle, "(%[^)])", ArgStr[0]);
				if (!CaculateExpression(ArgStr[0],GameFlag))
				{
					FindElsePlace(FileHandle);
				}
			}
			else if (!strcmp(Command, "else"))
			{
				fscanf(FileHandle, "%*[^\n]");
				FindElsePlace(FileHandle);
			}
			else if (!strcmp(Command, "while"))
			{
				fscanf(FileHandle, "(%[^)])", ArgStr[0]);
				if (CaculateExpression(ArgStr[0], GameFlag))
				{
					WhileInfo WI;
					WI.StreamPosition = ftell(FileHandle);
					strcpy(WI.expression, ArgStr[0]);
					whileStack.push(WI);
				}
				else
					FindCloseCase(FileHandle,"while", "loop");
			}
			else if (!strcmp(Command, "loop"))
			{
				fscanf(FileHandle, "%*[^\n]");
				if (whileStack.empty())
					continue;
				if (CaculateExpression(whileStack.top().expression, GameFlag))
					fseek(FileHandle, whileStack.top().StreamPosition, SEEK_SET);
				else
					whileStack.pop();
			}
			else if (!strcmp(Command, "goto"))
			{
				fscanf(FileHandle, "(%[^)])", ArgStr[0]);
				long SteamCurrent = ftell(FileHandle);
				fseek(FileHandle, 0, SEEK_SET);
				while (!feof(FileHandle))
				{
					if (fgetc(FileHandle) != '#')
						continue;
					fscanf(FileHandle, "%s", ArgStr[1]);
					if (!strcmp(ArgStr[0], ArgStr[1]))
					{
						while (!whileStack.empty())
							whileStack.pop();
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
				fscanf(FileHandle, "(%[^)])", ArgStr[0]);
				Sleep(CaculateExpression(ArgStr[0],GameFlag));
			}
			else if (!strcmp(Command, "endsub"))
			{
				fscanf(FileHandle, "%*s");
				fseek(FileHandle, subStack.top(), SEEK_SET);
				subStack.pop();
			}
			else if (!strcmp(Command, "return"))
			{
				fclose(FileHandle);
				if (Async&&ThreadSlot)
					(*ThreadSlot)[CurrentSlot] = NULL;
				return;
			}
			else if (!strcmp(Command, "end"))
			{
				exit(0);
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
				subStack.push(ftell(FileHandle));
				fseek(FileHandle, SubCurrent, SEEK_SET);
			}
		} while (!feof(FileHandle));
		fclose(FileHandle);
		if (Async&&ThreadSlot)
			(*ThreadSlot)[CurrentSlot] = NULL;
		return;
	};
	if (Async)
	{
		std::thread* TmpThread;
		if (ThreadSlot)
		{
			ThreadSlot->push_back(NULL);
			TmpThread = new std::thread(ExecuterLamdba);
			TmpThread->detach();
		}else
			(TmpThread = new std::thread(ExecuterLamdba))->detach();
		return TmpThread;
	}
	else
		return ExecuterLamdba(),NULL;
}

void ProcessGameLogic()
{
	LogicLock.lock();
	using namespace GameLogic;
	//处理固定的敌人
	for (int i = 0; i < PawnInstance.size(); i++)
		if (PawnInstance[i]->Speed < 0&&PawnInstance[i]->Enabled)
			PawnInstance[i]->Location.y += GameLogic::FlySpeed;
	//处理Item的拾取事件
	for (int i = 0; i < ItemInstance.size(); i++)
	{
		if (!ItemInstance[i]->Enabled)
			continue;
		bool PaddingKill = false;
		for (int j = 0; j <Players.size(); j++)
		{
			if ((Players[j].PlayerPawn->BodyRadius + ItemInstance[i]->PickRadius)/2 > GetPointDistance(Players[j].PlayerPawn->Location, ItemInstance[i]->Location))
			{
				if (!Players[j].PlayerPawn->Enabled)
					continue;
				ItemInstance[i]->OnPickedUp.Execute<ItemBase,VoidType>(ItemInstance[i], &Players[j]);
				PaddingKill = true;
			}
		}
		ItemInstance[i]->Location.x += ItemInstance[i]->MoveDirection.x*ItemInstance[i]->Speed;
		ItemInstance[i]->Location.y += ItemInstance[i]->MoveDirection.y*ItemInstance[i]->Speed/2;
		if (ItemInstance[i]->Location.x + ItemInstance[i]->PickRadius < 0 || ItemInstance[i]->Location.x - ItemInstance[i]->PickRadius > 1)
			ItemInstance[i]->MoveDirection.x *= -1;
		else if (ItemInstance[i]->Location.y + ItemInstance[i]->PickRadius / 2 < 0 || ItemInstance[i]->Location.y - ItemInstance[i]->PickRadius / 2 > 1)
			ItemInstance[i]->MoveDirection.y *= -1;
		if (PaddingKill)
		{
			TerminateThread(ItemInstance[i]->RootScript.Thread_ExitSignal, 0);
			//效果
			if (ItemInstance[i]->PickedAnimation.AnimationBitmap)
				ItemInstance[i]->PickedAnimation.PlayAtLocation(ItemInstance[i]->Location);
			if (ItemInstance[i]->PickedSound)
				PlayXAudio2Sound(ItemInstance[i]->PickedSound);
			delete ItemInstance[i];
			ItemInstance.erase(ItemInstance.begin()+i);
			i--;
			continue; 
		}
		//D2D1_POINT_2F SpeedTmpx = { ItemInstance[i]->MoveDirection.x*ItemInstance[i]->Speed,ItemInstance[i]->MoveDirection.y*ItemInstance[i]->Speed };
	}
	//检查Pawn数量并广播事件信号
	if (!PawnInstance.size())
		SetEvent(EnemyClearedEvent);
	else 
		ResetEvent(EnemyClearedEvent);
	//处理Ammo的碰撞事件
	for (int i = 0; i < AmmoInstance.size(); i++)
	{
		if (!AmmoInstance[i]->Enabled)
			continue;
		if (AmmoInstance[i]->FromPlayer)//检查是不是来自玩家的子弹
		{
			//处理敌人
			bool PaddingKill = false;
			for (int j = 0; j < PawnInstance.size(); j++)
			{
				if ((PawnInstance[j]->BodyRadius + AmmoInstance[i]->AttackRadius) / 2 > GetPointDistance(PawnInstance[j]->Location, AmmoInstance[i]->Location))
				{
					if (!PawnInstance[j]->Enabled)
						continue;
					PaddingKill = true;
					AmmoInstance[i]->OnHited.Execute<AmmoBase, PawnBase>(AmmoInstance[i], PawnInstance[j]);
					PawnInstance[j]->Health--;
					PawnInstance[j]->OnDamaged.Execute<PawnBase, AmmoBase>(PawnInstance[j], AmmoInstance[i]);
					if (PawnInstance[j]->Health <= 0)
					{
						PawnInstance[j]->OnDead.Execute<PawnBase, VoidType>(PawnInstance[j], NULL);
						TerminateThread(PawnInstance[j]->RootScript.Thread_ExitSignal, 0);
						if (PawnInstance[j]->DeadAnimation.AnimationBitmap)
							PawnInstance[j]->DeadAnimation.PlayAtLocation(PawnInstance[j]->Location);
						if (PawnInstance[j]->DeadSound)
							PlayXAudio2Sound(PawnInstance[j]->DeadSound);
						delete PawnInstance[j];
						PawnInstance.erase(PawnInstance.begin() + j);
					}
				}
			}
			AmmoInstance[i]->Location.x += AmmoInstance[i]->MoveDirection.x*AmmoInstance[i]->Speed;
			AmmoInstance[i]->Location.y += AmmoInstance[i]->MoveDirection.y*AmmoInstance[i]->Speed / 2;
			if (PaddingKill)
			{
				TerminateThread(AmmoInstance[i]->RootScript.Thread_ExitSignal, 0);
				if (AmmoInstance[i]->HittedAnimation.AnimationBitmap)
					AmmoInstance[i]->HittedAnimation.PlayAtLocation(AmmoInstance[i]->Location);
				if (AmmoInstance[i]->HittedSound)
					PlayXAudio2Sound(AmmoInstance[i]->HittedSound);
				delete AmmoInstance[i];
				AmmoInstance.erase(AmmoInstance.begin() + i);
				i--;
				continue;//Continue For(Ammo)
			}
			if (IsOutofBound(AmmoInstance[i]->Location, AmmoInstance[i]->AttackRadius))
			{
				delete AmmoInstance[i];
				AmmoInstance.erase(AmmoInstance.begin() + i);
				i--;
			}
			//D2D1_POINT_2F SpeedTmpx = { ItemInstance[i]->MoveDirection.x*ItemInstance[i]->Speed,ItemInstance[i]->MoveDirection.y*ItemInstance[i]->Speed };
		}
		else
		{
			//处理玩家
			bool PaddingKill = false;
			for (int j = 0; j < Players.size(); j++)
			{
				if ((Players[j].PlayerPawn->BodyRadius + AmmoInstance[i]->AttackRadius) / 2 > GetPointDistance(Players[j].PlayerPawn->Location, AmmoInstance[i]->Location))
				{
					if (!Players[j].PlayerPawn->Enabled)
						continue;
					PaddingKill = true;
					AmmoInstance[i]->OnHited.Execute<AmmoBase, VoidType>(AmmoInstance[i], &Players[j]);
					Players[j].PlayerPawn->Health--;
					Players[j].PlayerPawn->OnDamaged.Execute<PawnBase, AmmoBase>(Players[j].PlayerPawn, AmmoInstance[i]);
					if (Players[j].PlayerPawn->Health <= 0)
					{
						Players[j].PlayerPawn->OnDead.Execute<PawnBase, VoidType>(Players[j].PlayerPawn, NULL);
						TerminateThread(Players[j].PlayerPawn->RootScript.Thread_ExitSignal, 0);
						if (Players[j].PlayerPawn->DeadAnimation.AnimationBitmap)
							Players[j].PlayerPawn->DeadAnimation.PlayAtLocation(Players[j].PlayerPawn->Location);
						if (Players[j].PlayerPawn->DeadSound)
							PlayXAudio2Sound(Players[j].PlayerPawn->DeadSound);
						delete Players[j].PlayerPawn;
						Players[j].PlayerPawn = NULL;
						if (Players[j].Life > 0)
						{
							Players[j].Life--;
							Players[j].PlayerPawn = new PawnBase(*GameLogic::DefPlayerPawn);
							Players[j].PlayerPawn->RootScript.Execute<PawnBase, VoidType>(Players[j].PlayerPawn, NULL);
						}
					}
				}
			}
			AmmoInstance[i]->Location.x += AmmoInstance[i]->MoveDirection.x*AmmoInstance[i]->Speed;
			AmmoInstance[i]->Location.y += AmmoInstance[i]->MoveDirection.y*AmmoInstance[i]->Speed / 2;
			if (PaddingKill)
			{
				TerminateThread(AmmoInstance[i]->RootScript.Thread_ExitSignal, 0);
				if (AmmoInstance[i]->HittedAnimation.AnimationBitmap)
					AmmoInstance[i]->HittedAnimation.PlayAtLocation(AmmoInstance[i]->Location);
				if (AmmoInstance[i]->HittedSound)
					PlayXAudio2Sound(AmmoInstance[i]->HittedSound);
				delete AmmoInstance[i];
				AmmoInstance.erase(AmmoInstance.begin() + i);
				i--;
				continue; //Continue For(Ammo)
			}
			if (IsOutofBound(AmmoInstance[i]->Location, AmmoInstance[i]->AttackRadius))
			{
				delete AmmoInstance[i];
				AmmoInstance.erase(AmmoInstance.begin() + i);
				i--;
			}
			//D2D1_POINT_2F SpeedTmpx = { ItemInstance[i]->MoveDirection.x*ItemInstance[i]->Speed,ItemInstance[i]->MoveDirection.y*ItemInstance[i]->Speed };
		}
	}
	//射击
	for (int i = 0; i<GameLogic::Players.size(); i++)
		if (GameLogic::Players[i].PlayerPawn)
			GameLogic::Players[i].PlayerPawn->ShootDelay - 20 > 0 ? GameLogic::Players[i].PlayerPawn->ShootDelay -= 20 : GameLogic::Players[i].PlayerPawn->ShootDelay = 0;
	for (int i=0;i<PawnInstance.size();i++)
		PawnInstance[i]->ShootDelay - 20 > 0 ? PawnInstance[i]->ShootDelay -= 20 : PawnInstance[i]->ShootDelay = 0;
	//处理玩家输入
	LogicLock.unlock();
	ProcessPlayerInput();
}

void RenderGame()
{
	RenderLock.lock(); //线程锁
	//开始渲染
	Sys.D2D1HwndRenderTarget->BeginDraw();
	auto RenderTargetSize = Sys.D2D1HwndRenderTarget->GetPixelSize();
#pragma warning( disable: 4244 )  //忽略精度丢失警告
	//图像Z序：
	//1.背景
	//2.敌人
	//3.道具
	//4.玩家
	//5.子弹
	//6.序列帧动画
	//画背景
	if (GameLogic::GameBackground)
	{
		float BGHeight = (float)RenderTargetSize.width / (float)GameLogic::GameBackground->D2DBitmap->GetPixelSize().width* (float)GameLogic::GameBackground->D2DBitmap->GetPixelSize().height;
		GameLogic::BackgroundState = fmodf(GameLogic::BackgroundState + GameLogic::FlySpeed, BGHeight / RenderTargetSize.height);
		for (int i = -1; i*BGHeight <= RenderTargetSize.height; i++)
			Sys.D2D1HwndRenderTarget->DrawBitmap(GameLogic::GameBackground->D2DBitmap,
				D2D1::RectF(0, RenderTargetSize.height*GameLogic::BackgroundState + i*BGHeight, RenderTargetSize.width, RenderTargetSize.height*GameLogic::BackgroundState + (i + 1)*BGHeight), 1, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
				D2D1::RectF(0, 0, GameLogic::GameBackground->D2DBitmap->GetPixelSize().width, GameLogic::GameBackground->D2DBitmap->GetPixelSize().height));
	}
	//画敌人
	for (auto it=PawnInstance.begin();it<PawnInstance.end();++it)
	{
		if ((*it)->PawnImage)
			Sys.D2D1HwndRenderTarget->DrawBitmap((*it)->PawnImage->D2DBitmap,
				D2D1::RectF(((*it)->Location.x - (*it)->BodyRadius / 2) * RenderTargetSize.width, ((*it)->Location.y - (*it)->BodyRadius / 4) * RenderTargetSize.height,
				((*it)->Location.x + (*it)->BodyRadius / 2) * RenderTargetSize.width, ((*it)->Location.y + (*it)->BodyRadius / 4) * RenderTargetSize.height), 1, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
				D2D1::RectF(0, 0, (*it)->PawnImage->D2DBitmap->GetPixelSize().width, (*it)->PawnImage->D2DBitmap->GetPixelSize().height));
	}
	//画道具
	for (auto it = ItemInstance.begin(); it < ItemInstance.end(); ++it)
	{
		if ((*it)->ItemImage)
			Sys.D2D1HwndRenderTarget->DrawBitmap((*it)->ItemImage->D2DBitmap,
				D2D1::RectF(((*it)->Location.x - (*it)->PickRadius / 2) * RenderTargetSize.width, ((*it)->Location.y - (*it)->PickRadius / 4) * RenderTargetSize.height,
				((*it)->Location.x + (*it)->PickRadius / 2) * RenderTargetSize.width, ((*it)->Location.y + (*it)->PickRadius / 4) * RenderTargetSize.height), 1, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
				D2D1::RectF(0, 0, (*it)->ItemImage->D2DBitmap->GetPixelSize().width, (*it)->ItemImage->D2DBitmap->GetPixelSize().height));
	}
	//画玩家
	for (auto it = GameLogic::Players.begin(); it < GameLogic::Players.end(); ++it)
	{
		if (it->PlayerPawn&&it->PlayerPawn->PawnImage)
			Sys.D2D1HwndRenderTarget->DrawBitmap(it->PlayerPawn->PawnImage->D2DBitmap,
				D2D1::RectF((it->PlayerPawn->Location.x - it->PlayerPawn->BodyRadius / 2) * RenderTargetSize.width, (it->PlayerPawn->Location.y - it->PlayerPawn->BodyRadius / 4) * RenderTargetSize.height,
				(it->PlayerPawn->Location.x + it->PlayerPawn->BodyRadius / 2) * RenderTargetSize.width, (it->PlayerPawn->Location.y + it->PlayerPawn->BodyRadius / 4) * RenderTargetSize.height), 1, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
				D2D1::RectF(0, 0, it->PlayerPawn->PawnImage->D2DBitmap->GetPixelSize().width, it->PlayerPawn->PawnImage->D2DBitmap->GetPixelSize().height));
	}
	//画子弹
	for (auto it = AmmoInstance.begin(); it < AmmoInstance.end(); ++it)
	{
		if ((*it)->AmmoImage)
			Sys.D2D1HwndRenderTarget->DrawBitmap((*it)->AmmoImage->D2DBitmap,
				D2D1::RectF(((*it)->Location.x - (*it)->AttackRadius / 2) * RenderTargetSize.width, ((*it)->Location.y - (*it)->AttackRadius / 4) * RenderTargetSize.height,
				((*it)->Location.x + (*it)->AttackRadius / 2) * RenderTargetSize.width, ((*it)->Location.y + (*it)->AttackRadius / 4) * RenderTargetSize.height), 1, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
				D2D1::RectF(0, 0, (*it)->AmmoImage->D2DBitmap->GetPixelSize().width, (*it)->AmmoImage->D2DBitmap->GetPixelSize().height));
	}
	//处理序列帧
	for (auto it=FrameAnimationCollection.begin();it<FrameAnimationCollection.end();++it)
	{
		if ((*it)->RenderOneTick())
		{
			delete *it;
			it = FrameAnimationCollection.erase(it);
			if (it>=FrameAnimationCollection.end())
				break;
		}
	}
#pragma warning( default: 4244 )  //重启精度丢失警告
	//结束渲染
	Sys.D2D1HwndRenderTarget->EndDraw();
	RenderLock.unlock(); //释放锁
}

void SpawnAmmo(const char AmmoTelemplateName[],D2D1_POINT_2F LocationByPermill,GameLogic::PawnBase* AmmoFrom)
{
	if (!AmmoFrom || AmmoFrom->ShootDelay)
		return;
	LogicLock.lock();
	auto TmpAmmoTemplate = AmmoTemplate.find(std::string(AmmoTelemplateName))->second;
	auto NewAmmoBaseInstance = new GameLogic::AmmoBase(*TmpAmmoTemplate);
	NewAmmoBaseInstance->Location = LocationByPermill;
	for (int i = 0; i < GameLogic::Players.size(); i++)
		if (AmmoFrom == GameLogic::Players[i].PlayerPawn)
		{
			NewAmmoBaseInstance->MoveDirection = { 0,-1 };
			NewAmmoBaseInstance->FromPlayer = true;
		}
	AmmoInstance.push_back(NewAmmoBaseInstance);
	LogicLock.unlock();
	NewAmmoBaseInstance->RootScript.Execute<GameLogic::AmmoBase,GameLogic::PawnBase>(NewAmmoBaseInstance, AmmoFrom);   //Execute函数应当会检查执行点是否有效，所以此处不需要进行检查
	if (AmmoFrom)
		AmmoFrom->ShootDelay = AmmoFrom->ShootDelayMax;
}

void SpawnEnemy(const char PawnTelemplateName[], D2D1_POINT_2F LocationByPermill)
{
	LogicLock.lock();
	auto TmpPawnTemplate = PawnTemplate.find(std::string(PawnTelemplateName))->second;
	auto NewPawnBaseInstance = new GameLogic::PawnBase(*TmpPawnTemplate);
	NewPawnBaseInstance->Location = LocationByPermill;
	PawnInstance.push_back(NewPawnBaseInstance);
	LogicLock.unlock();
	NewPawnBaseInstance->RootScript.Execute<GameLogic::PawnBase,GameLogic::VoidType>(NewPawnBaseInstance,NULL);   //Execute函数应当会检查执行点是否有效，所以此处不需要进行检查
}

void SpawnItem(const char ItemTelemplateName[], D2D1_POINT_2F LocationByPermill)
{
	LogicLock.lock();
	auto TmpItemTemplate = ItemTemplate.find(std::string(ItemTelemplateName))->second;
	auto NewItemBaseInstance = new GameLogic::ItemBase(*TmpItemTemplate);
	NewItemBaseInstance->Location = LocationByPermill;
	NewItemBaseInstance->MoveDirection = GetUnitVector(D2D1::Point2F( rand() - RAND_MAX / 2,rand() - RAND_MAX / 2 ));
	ItemInstance.push_back(NewItemBaseInstance);
	LogicLock.unlock();
	NewItemBaseInstance->RootScript.Execute < GameLogic::ItemBase,GameLogic::VoidType > (NewItemBaseInstance, NULL);   //Execute函数应当会检查执行点是否有效，所以此处不需要进行检查
}

template<typename TSelf, typename TTarget>
void	ScriptExecutePoint::Execute(void* Self, void* Target)
{
	if (ScriptPoint >= 0 && strlen(FilePath))
		ScriptExecuter<TSelf, TTarget>(FilePath, ScriptPoint, Self, Target, ShouldAsync, &Thread_ExitSignal);
}

template<typename T>
void DestroySelf(T* pp){}

template<>
void DestroySelf<GameLogic::PawnBase>(GameLogic::PawnBase* pPawn)
{
	TerminateThread(pPawn->RootScript.Thread_ExitSignal, 0);
	delete pPawn;
	for (auto it = PawnInstance.begin(); it < PawnInstance.end(); ++it)
		if (*it == pPawn)
		{
			PawnInstance.erase(it);
			return;
		}
}

template<>
void DestroySelf<GameLogic::AmmoBase>(GameLogic::AmmoBase* pAmmo)
{
	TerminateThread(pAmmo->RootScript.Thread_ExitSignal, 0);
	delete pAmmo;
	for (auto it = AmmoInstance.begin(); it < AmmoInstance.end(); ++it)
		if (*it == pAmmo)
		{
			AmmoInstance.erase(it);
			return;
		}
}

template<>
void DestroySelf<GameLogic::ItemBase>(GameLogic::ItemBase* pItem)
{
	TerminateThread(pItem->RootScript.Thread_ExitSignal, 0);
	delete pItem;
	for (auto it = ItemInstance.begin(); it < ItemInstance.end(); ++it)
		if (*it == pItem)
		{
			ItemInstance.erase(it);
			return;
		}
}

inline double func_rand(double minv,double maxv)
{
	return ((double)rand() / RAND_MAX)*(maxv - minv) + minv;
}

void ProcessPlayerInput()
{
	if (GameLogic::Players[0].IsLocal&&GameLogic::Players[0].PlayerPawn)
	{
		float TmpValue = (KEY_DOWN(VK_RIGHT) - KEY_DOWN(VK_LEFT))*GameLogic::Players[0].PlayerPawn->Speed;
		if (!(GameLogic::Players[0].PlayerPawn->Location.x + TmpValue*2 > 1 || GameLogic::Players[0].PlayerPawn->Location.x + TmpValue*2<0))
			GameLogic::Players[0].PlayerPawn->Location.x += TmpValue;
		TmpValue = (KEY_DOWN(VK_DOWN) - KEY_DOWN(VK_UP))*GameLogic::Players[0].PlayerPawn->Speed / 2;
		if (!(GameLogic::Players[0].PlayerPawn->Location.y + TmpValue*2 > 1 || GameLogic::Players[0].PlayerPawn->Location.y + TmpValue*2<0))
			GameLogic::Players[0].PlayerPawn->Location.y += TmpValue;
		if (KEY_DOWN(VK_SPACE))
			SpawnAmmo(GameLogic::Players[0].PlayerPawn->WeaponAmmo.c_str(), GameLogic::Players[0].PlayerPawn->Location, GameLogic::Players[0].PlayerPawn);
	}
}

void TerminateThread(std::vector<bool*> ThreadCollection, _In_ DWORD dwExitCode)
{
	for (auto it = ThreadCollection.begin(); it < ThreadCollection.end(); ++it)
		if (*it)
			**it = true;
}