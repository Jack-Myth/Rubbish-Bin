#include <stdio.h>
#include <windows.h>
#include <atlimage.h>

LRESULT CALLBACK NewProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

HBITMAP  hb;
HDC memDC;
HDC DC;
LONG cxSource, cySource;
CImage CI;
LONG OldProc;
int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd )
{
	long InputTmp;
	HWND Target;
	WCHAR BGPath[1024] = TEXT("E:\\JackMyth\\Documents\\Visual Studio 2017\\Projects\\TestForDirectX\\TestForDirect2D\\MikuPic.bmp");
	printf("Input hWnd:");
	InputTmp = 1247234;
	//scanf("%ld", &InputTmp);
	Target = (HWND)InputTmp;
	//printf("Input BGPath:");
	//scanf("%[^\n]", &BGPath);
	//hb = (HBITMAP)LoadImageA(NULL, BGPath, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
	DC = GetDC(Target);
	//memDC=CreateCompatibleDC(DC);
	//SelectObject(memDC, hb);
	CI.Load(BGPath);
	RECT rc;
	GetWindowRect(Target,&rc);
	rc = { 0,0,rc.right - rc.left,rc.bottom - rc.top };
	while (1)
	{
		CI.StretchBlt(DC, rc);
		Sleep(100);
	}
	/*
	OldProc = SetWindowLong(Target, GWL_WNDPROC, (LONG)NewProc);
	MSG msg = {NULL};
	while (1)
	{
		TranslateMessage(&msg);
		GetMessage(&msg,0,0,0);
		DispatchMessage(&msg);
	}*/
	return 0;
}

LRESULT CALLBACK NewProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT:
	{
		RECT rc = { 0, 0, GetDeviceCaps(DC, HORZRES), GetDeviceCaps(DC, VERTRES) };
		CI.StretchBlt(DC, rc);
		//BitBlt(DC,0,0, GetDeviceCaps(DC, HORZRES),GetDeviceCaps(DC, VERTRES),memDC,cxSource,cySource,)
	}
	default:
		break;
	}
	return 0;
	//return CallWindowProc((WNDPROC)OldProc,hWnd,message,wParam,lParam);
}