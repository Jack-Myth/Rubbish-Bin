/*
2017-5-26
实验报告8、题目1、2016061520、田明
*/
#include <stdio.h>
#include <windows.h>

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
HWND MainHwnd=NULL, EditHwnd=NULL;
int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd )
{
	RECT rc = { 0,0,1024,768 };
	WNDCLASSEX wnd = {NULL};
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wnd.hCursor = LoadCursor(0, IDC_ARROW);
	wnd.hIcon = LoadIcon(0, IDC_ICON);
	wnd.hInstance = hInstance;
	wnd.lpszClassName = TEXT("Win32 Blank Window");
	wnd.lpfnWndProc = MainWndProc;
	wnd.style = CS_DBLCLKS;
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	RegisterClassEx(&wnd);
	MainHwnd=CreateWindow(wnd.lpszClassName, TEXT("点击最大化按钮保存文件"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right-rc.left, rc.bottom-rc.top, NULL, NULL, hInstance, NULL);
	ShowWindow(MainHwnd,nShowCmd);
	MSG msg = { NULL };
	while (msg.message!=WM_QUIT)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		GetMessage(&msg, NULL, NULL, NULL);
	}
	return 0;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		EditHwnd= CreateWindow(TEXT("Edit"), TEXT(""), WS_CHILDWINDOW | WS_VISIBLE | WS_VSCROLL | WS_EX_CLIENTEDGE | WS_EX_OVERLAPPEDWINDOW | ES_MULTILINE | ES_NOHIDESEL, 0, 0,1024 , 768, hwnd, NULL, NULL, NULL);
		break;
	case WM_SYSCOMMAND:
		if (wParam== SC_MAXIMIZE)
		{
			OPENFILENAMEA ofn = { 0 };
			char FileName[255] = { NULL };
			ofn.lStructSize = sizeof(OPENFILENAMEA);
			ofn.lpstrFilter = "文本文件\0*.txt\0\0";
			ofn.lpstrTitle = "选择保存位置";
			ofn.hwndOwner = hwnd;
			ofn.nMaxFile = MAX_PATH;
			ofn.lpstrFile = FileName;
			if (GetSaveFileNameA(&ofn))
			{
				strcat(FileName, ".txt");
				char TextToSave[10240] = { NULL };
				FILE* FileHandle = fopen(FileName, "w+");
				GetWindowTextA(EditHwnd, TextToSave, 10240);
				fprintf(FileHandle, "%s", TextToSave);
				fclose(FileHandle);
			}
			return 0;
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}