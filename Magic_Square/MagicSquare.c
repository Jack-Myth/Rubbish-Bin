#ifndef _UNICODE
#define  _UNICODE
#endif
#include <windows.h>
#include <stdio.h>

HRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
DWORD WINAPI MagicSquareProcessor(LPVOID lpParameter);
void ApplyNumber(POINT TargetPosition, POINT* LastPosition, int NumberToApply);
void RenderViewer();
inline BOOL IsOutOfRange(POINT TargetToCheck);

RECT rc = { 0,0,500,500 };
HWND MainHwnd = NULL;
int** MagicSquare = NULL;
int MagicSquareSize = 3;
HDC MainDC=NULL, MemoryDC=NULL;//GDI双缓冲
HPEN gPen ;
HBRUSH BlackBrush;
HBRUSH WhiteBrush;
HFONT DefFont;
HBRUSH FinishedColor;
int Finished=0;
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	if (!AllocConsole())
	{
		MessageBoxA(NULL, "控制台分配失败", "MagicSquare", MB_OK);
		return -1;
	}
	freopen("CONOUT$", "w", stdout);
	freopen("CONIN$", "r", stdin);
	printf("输入你想要创建的魔方阵行列数:");
	scanf("%d", &MagicSquareSize);
	if (MagicSquareSize<3||!(MagicSquareSize%2))
	{
		MessageBoxA(NULL, "不正确的魔方阵参数","MagicSquare_Viewer",MB_OK);
		return -1;
	}
	HWND ConsoleHwnd = GetConsoleWindow();
	FreeConsole();
	SendMessage(ConsoleHwnd, WM_CLOSE, 0, 0);
	MagicSquare = (int**)malloc(sizeof(int*)*MagicSquareSize);
	for (int i = 0; i < MagicSquareSize; i++)
	{
		MagicSquare[i] = (int*)malloc(MagicSquareSize*sizeof(int));
		memset(MagicSquare[i], 0, MagicSquareSize*sizeof(int));
	}
	WNDCLASSEX wnd = {0};
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wnd.hCursor = LoadCursor(0, IDC_ARROW);
	wnd.hInstance = hInstance;
	wnd.lpfnWndProc = MainWndProc;
	wnd.lpszClassName = TEXT("MagicSquare");
	RegisterClassEx(&wnd);
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	MainHwnd=CreateWindow(wnd.lpszClassName, TEXT("MagicSquare_Viewer"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
	ShowWindow(MainHwnd, nShowCmd);
	//Init GDI
	gPen=CreatePen(PS_SOLID, 2, RGB(0, 0, 0));
	BlackBrush = CreateSolidBrush(RGB(0, 0, 0));
	WhiteBrush = CreateSolidBrush(RGB(255, 255, 255));
	FinishedColor = CreateSolidBrush(RGB(102,204,255));
	MainDC = GetDC(MainHwnd);
	MemoryDC=CreateCompatibleDC(MainDC);
	SelectObject(MemoryDC, CreateCompatibleBitmap(MainDC, 500, 500));
	char NumSize[5] = {0};
	sprintf(NumSize, "%d", MagicSquareSize*MagicSquareSize);
	DefFont = CreateFont(400 / MagicSquareSize, 400/strlen(NumSize) / MagicSquareSize, 0, 0, FW_NORMAL, FALSE, FALSE, 0, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, TEXT("宋体"));
	SelectObject(MemoryDC,DefFont);
	SetBkMode(MemoryDC, TRANSPARENT);
	//Init GDI Finished
	CreateThread(NULL, 0, MagicSquareProcessor, NULL, 0, NULL);
	SetTimer(MainHwnd, 0, 10, 0);
	MSG msg = {NULL};
	while (msg.message != WM_QUIT)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		GetMessage(&msg, NULL, 0, 0);
	}
	return 0;
}

HRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_TIMER:
		RenderViewer();
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

DWORD WINAPI MagicSquareProcessor(LPVOID lpParameter)
{
	POINT LastPostion = { (int)(MagicSquareSize / 2),0 };
	MagicSquare[LastPostion.x][LastPostion.y] = 1;
	for (int CurrentNumber=2;CurrentNumber<=MagicSquareSize*MagicSquareSize;CurrentNumber++)
	{
		POINT TargetPosition;
		if (!LastPostion.y&&LastPostion.x < MagicSquareSize - 1)
		{
			TargetPosition.x = LastPostion.x + 1;
			TargetPosition.y = MagicSquareSize - 1;
			ApplyNumber(TargetPosition, &LastPostion, CurrentNumber);
		}
		else if (LastPostion.x == MagicSquareSize - 1)
		{
			TargetPosition.x = 0;
			TargetPosition.y = LastPostion.y - 1;
			ApplyNumber(TargetPosition, &LastPostion, CurrentNumber);
		}
		else
		{
			TargetPosition.x = LastPostion.x+1;
			TargetPosition.y = LastPostion.y - 1;
			ApplyNumber(TargetPosition, &LastPostion, CurrentNumber);
		}
	}
	Finished = 1;
	return 0;
}

void ApplyNumber(POINT TargetPosition,POINT* LastPosition,int NumberToApply)
{
	if (MagicSquare[TargetPosition.x][TargetPosition.y] || IsOutOfRange(TargetPosition))
		LastPosition->y += 1;
	else
		*LastPosition = TargetPosition;
	for (int i=0;i<7;i++)
	{//原子，不加锁
		MagicSquare[LastPosition->x][LastPosition->y]= rand() % MagicSquareSize*MagicSquareSize + 1;
		Sleep(30);
	}
	MagicSquare[LastPosition->x][LastPosition->y] = NumberToApply;
}

void RenderViewer()
{
	RECT Rc0 = { 0,0,500,500 };
	FillRect(MemoryDC, &Rc0, WhiteBrush);
	if (Finished)
	{
		SelectObject(MemoryDC, FinishedColor);
		Ellipse(MemoryDC, 500 - Finished, 500 - Finished, 500 + Finished, 500 + Finished);
		Finished+=13;
		if (Finished > 1024)
			Finished = 1024;
		SelectObject(MemoryDC, BlackBrush);
	}
	HPEN oPen = (HPEN)SelectObject(MemoryDC, gPen);
	int BlockSize = 460 / MagicSquareSize;
	for (int i=1;i<MagicSquareSize;i++)
	{
		MoveToEx(MemoryDC, 20, 20+i*BlockSize, NULL);
		LineTo(MemoryDC, 480, 20 + i*BlockSize);
	}
	for (int i = 1; i < MagicSquareSize; i++)
	{
		MoveToEx(MemoryDC, 20 + i*BlockSize,20 , NULL);
		LineTo(MemoryDC, 20 + i*BlockSize, 480);
	}
	char OutText[10] = {0};
	for (int x=0;x<MagicSquareSize;x++)
		for (int y = 0; y < MagicSquareSize; y++)
		{
			if (!MagicSquare[x][y])
				continue;
			sprintf(OutText, "%d", MagicSquare[x][y]);
			TextOutA(MemoryDC, 20 + x*BlockSize+2, 20 + y*BlockSize+2, OutText, strlen(OutText));
		}
	BitBlt(MainDC, 0, 0, 500, 500, MemoryDC, 0, 0, SRCCOPY);
}

inline BOOL IsOutOfRange(POINT TargetToCheck)
{
	return TargetToCheck.x < 0 || TargetToCheck.x >= MagicSquareSize || TargetToCheck.y < 0 || TargetToCheck.y >= MagicSquareSize;
}