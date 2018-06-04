#include <windows.h>
#include <stdio.h>

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

HWND MainHwnd = NULL,ButtonToProgress=NULL,EditS=NULL,EditR=NULL;
HDC MainHDC = NULL;
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	WNDCLASSEX wnd = { NULL };
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wnd.hCursor = LoadCursor(0, IDC_ARROW);
	wnd.hIcon = LoadIcon(0, IDC_ICON);
	wnd.hInstance = hInstance;
	wnd.lpfnWndProc = MainWndProc;
	wnd.lpszClassName = TEXT("Blank Win32 Window");
	RECT rc = { 0,0,200,100 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);
	RegisterClassEx(&wnd);
	MainHwnd = CreateWindow(wnd.lpszClassName, TEXT("~ _ ~"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right-rc.left, rc.bottom-rc.top, NULL, NULL, hInstance, NULL);
	ShowWindow(MainHwnd, nShowCmd);
	MainHDC = GetDC(MainHwnd);
	MSG msg = { NULL };
	while (msg.message!=WM_QUIT)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		GetMessage(&msg, 0, 0, 0);
	}
	return 0;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		EditS = CreateWindow(TEXT("EDIT"), TEXT("在这里输入数字"), WS_CHILDWINDOW |WS_VISIBLE | WS_EX_CLIENTEDGE | WS_EX_OVERLAPPEDWINDOW , 0, 0, 150, 40, hwnd, NULL, NULL, NULL);
		EditR = CreateWindow(TEXT("EDIT"), TEXT(""), WS_CHILDWINDOW | WS_VISIBLE |WS_EX_CLIENTEDGE | WS_EX_OVERLAPPEDWINDOW , 0, 50, 150, 40, hwnd, NULL, NULL, NULL);
		ButtonToProgress = CreateWindow(TEXT("BUTTON"), TEXT("计算"), WS_CHILDWINDOW | WS_VISIBLE, 150, 0, 50, 100, hwnd, NULL, NULL, NULL);
		return DefWindowProc(hwnd, message, wParam, lParam);
		break;
	case  WM_COMMAND:
		if ((HWND)lParam==ButtonToProgress&&HIWORD(wParam)==BN_CLICKED)
		{
			char Str[256] = {NULL};
			int Length = 0,R=0;
			GetWindowTextA(EditS, Str, 255);
			for (int i:Str)
			{
				if (i == '\0')
					break;
				if (i-'9'>0||i-'0'<0)
				{
					sprintf(Str, "别逗我了，\"%c\"这玩意儿是数字？", i);
					MessageBoxA(hwnd, Str, "你在开玩笑吗？", MB_OK);
					return 0;
				}
				if (Length>=4)
				{
					MessageBox(hwnd, TEXT("你好像输入了不止4个数字，我就一起算了"), TEXT("* _ *"), MB_OK);
					Length = -256;
				}
				R += (i - '0');
				Length++;
			}
			if (Length==0)
				break;
			sprintf(Str, "结果是：%d", R);
			SetWindowTextA(EditR, Str);
			MessageBoxA(hwnd, Str, "算完啦", MB_OK);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
		break;
	}
	return 0;
}
