#include <windows.h>
#include <stdio.h>
#include <map>
#include "qrencode.h"
#include <atlimage.h>
#include <locale.h>
#include <Gdiplus.h>
#pragma comment(lib,"GdiPlus.lib")
#define BUFF_SIZE 2048

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void CreateQRCode();
wchar_t * ANSIToUnicode(const char* str);
char * UnicodeToUTF8(const wchar_t* str);
int CL(int Value, int Min, int Max);
void DrawHalfOpacityRect(HDC hdc, RECT rc, COLORREF ColorToDraw, unsigned char Alpha);
int StrReplace(char strRes[], char from[], char to[]);
void SaveQRCode();

HWND MainHwnd,EditHwnd;
RECT rc = { 0,0,700,750 };
std::map<char, int> CharMap;
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	WNDCLASSEX wnd = { NULL };
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.hbrBackground = (HBRUSH)(COLOR_WINDOW+ 1);
	wnd.hCursor = LoadCursor(0, IDC_ARROW);
	wnd.hIcon = LoadIcon(0, IDC_ICON);
	wnd.hInstance = hInstance;
	wnd.lpfnWndProc = MainWndProc;
	wnd.lpszClassName = TEXT("Blank Win32 Window");
	wnd.style = CS_DBLCLKS;
	RegisterClassEx(&wnd);	
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);
	MainHwnd = CreateWindow(wnd.lpszClassName, TEXT("QRCodeEncoder   --By JackMyth    BaseOn libqrencode"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right-rc.left,rc.bottom-rc.top, NULL, NULL, hInstance, NULL);
	Gdiplus::GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR gdiplusToken;
	GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);
	ShowWindow(MainHwnd, nShowCmd);
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
		EditHwnd=CreateWindow(TEXT("EDIT"), TEXT("Type Data Here"), WS_CHILDWINDOW | WS_VISIBLE | WS_EX_CLIENTEDGE | WS_EX_OVERLAPPEDWINDOW | ES_MULTILINE | ES_NOHIDESEL, 0, 700, 750, 50, hwnd, NULL, NULL, NULL);
		return DefWindowProc(hwnd,message,wParam,lParam);
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_LBUTTONDBLCLK:
		CreateQRCode();
		break;
	case WM_RBUTTONDBLCLK:
		SaveQRCode();
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
	}
	return 0;
}

void CreateQRCode()
{
	//char OriginData[2000];
	char DataToEncode[2000];
	GetWindowTextA(EditHwnd, DataToEncode, 2000);
	char ScanLevel,PositionAlpha,DataAlpha;
	DataAlpha = DataToEncode[strlen(DataToEncode) - 1];
	PositionAlpha= DataToEncode[strlen(DataToEncode) - 2];
	ScanLevel= DataToEncode[strlen(DataToEncode) - 3];
	DataToEncode[strlen(DataToEncode) - 3] = 0;
	//StrReplace(DataToEncode, "...", "\r");
	//sscanf(OriginData, "%[^\r]\r%c%c%c", DataToEncode, &ScanLevel, &PositionAlpha, &DataAlpha);
	StrReplace(DataToEncode, "\\n", "\n");
	QRcode* QR= QRcode_encodeString(UnicodeToUTF8(ANSIToUnicode(DataToEncode)), 0, QR_ECLEVEL_H, QR_MODE_8, 1);
	if (!QR)
		return;
	WCHAR FileP[MAX_PATH] = { NULL };
	OPENFILENAME OpenFN = { NULL };
	OpenFN.lStructSize = sizeof(OPENFILENAME);
	OpenFN.hwndOwner = MainHwnd;
	OpenFN.lpstrFilter = TEXT("Í¼Æ¬ÎÄ¼þ\0*.bmp;*.jpg;*.png\0\0");
	OpenFN.nMaxFile = MAX_PATH;
	OpenFN.lpstrFile = FileP;
	OpenFN.Flags = OFN_FILEMUSTEXIST;
	OpenFN.lpstrTitle = TEXT("Ñ¡Ôñ¶þÎ¬Âë±³¾°Í¼Æ¬");
	HDC MainHDC = GetDC(MainHwnd);
	if (GetOpenFileName(&OpenFN))
	{
		CImage* CI = new CImage;
		RECT rc = { 0,0,700,700 };
		CI->Load(OpenFN.lpstrFile);
		SetStretchBltMode(MainHDC, HALFTONE);
		SetBrushOrgEx(MainHDC, 0, 0, NULL);
		CI->StretchBlt(MainHDC, rc);
		delete CI;
	}
	//SetBkMode(MainHDC, TRANSPARENT);
	HBRUSH BlackBrush = CreateSolidBrush(RGB(0, 0, 0));
	HBRUSH WhiteBrush = CreateSolidBrush(RGB(255, 255, 255));
	float Siz = (float)700.0 / (float)(QR->width);
	//Siz = (float)(int)(Siz + 1);
	if (ScanLevel > '9' || ScanLevel < '0')
		ScanLevel = (char)Siz;
	else
		ScanLevel = (int)((float)(ScanLevel - '0') / 9 * Siz);
	float PointSize =(float)CL((int)(ScanLevel+1),2,(int)(Siz+1));
	RECT QRRC = { 0,0,0,0 };
	for(int y=0;y<QR->width;y++)
		for (int x = 0;x < QR->width; x++)
		{
			if (*(QR->data+y*QR->width+x)&0x1)
			{
				if ((!(*(QR->data + y*QR->width + x) >> 5 & 0x1)) && (!(*(QR->data + y*QR->width + x) >> 6 & 0x1)))
				{
					QRRC = { (int)(x*Siz + (Siz - PointSize) / 2),(int)(y*Siz + (Siz - PointSize) / 2) ,(int)(x*Siz + (Siz - PointSize) / 2 + PointSize),(int)(y*Siz + (Siz - PointSize) / 2 + PointSize) };
					DrawHalfOpacityRect(MainHDC, QRRC, RGB(0, 0, 0), CL((DataAlpha - '0') / 9 * 127,0,127) + 128);
				}
				else
				{
					QRRC = { (int)(x*Siz),(int)(y*Siz) ,(int)(x*Siz + Siz),(int)(y*Siz + Siz) };
					DrawHalfOpacityRect(MainHDC, QRRC, RGB(0, 0, 0), CL((PositionAlpha - '0') / 9 * 127, 0, 127) + 128);
				}
				//FillRect(MainHDC, &QRRC, BlackBrush);
			}
			else
			{
				if ((!(*(QR->data + y*QR->width + x) >> 5 & 0x1)) && (!(*(QR->data + y*QR->width + x) >> 6 & 0x1)))
				{
					QRRC = { (int)(x*Siz + (Siz - PointSize) / 2),(int)(y*Siz + (Siz - PointSize) / 2) ,(int)(x*Siz + (Siz - PointSize) / 2 + PointSize),(int)(y*Siz + (Siz - PointSize) / 2 + PointSize) };
					DrawHalfOpacityRect(MainHDC, QRRC, RGB(255, 255, 255), CL((DataAlpha - '0') / 9 * 127, 0, 127) + 128);
				}
				else
				{
					QRRC = { (int)(x*Siz),(int)(y*Siz) ,(int)(x*Siz + Siz),(int)(y*Siz + Siz) };
					DrawHalfOpacityRect(MainHDC, QRRC, RGB(255, 255, 255), CL((PositionAlpha - '0') / 9 * 127, 0, 127) + 128);
				}
				//FillRect(MainHDC, &QRRC, WhiteBrush);
			}
		}
	ReleaseDC(MainHwnd, MainHDC);
}

wchar_t * ANSIToUnicode(const char* str)
{
	int textlen;
	wchar_t * result;
	textlen = MultiByteToWideChar(CP_ACP, 0, str, -1, NULL, 0);
	result = (wchar_t *)malloc((textlen + 1) * sizeof(wchar_t));
	memset(result, 0, (textlen + 1) * sizeof(wchar_t));
	MultiByteToWideChar(CP_ACP, 0, str, -1, (LPWSTR)result, textlen);
	return result;
}

char * UnicodeToUTF8(const wchar_t* str)
{
	char* result;
	int textlen;
	textlen = WideCharToMultiByte(CP_UTF8, 0, str, -1, NULL, 0, NULL, NULL);
	result = (char *)malloc((textlen + 1) * sizeof(char));
	memset(result, 0, sizeof(char) * (textlen + 1));
	WideCharToMultiByte(CP_UTF8, 0, str, -1, result, textlen, NULL, NULL);
	return result;
}

int CL(int Value, int Min, int Max)
{
	if (Value < Min)
		return Min;
	if (Value > Max)
		return Max;
	return Value;
}

void DrawHalfOpacityRect(HDC hdc, RECT rc, COLORREF ColorToDraw, unsigned char Alpha)
{
	using namespace Gdiplus;
	SolidBrush m_pBrush(Color(Alpha, GetRValue(ColorToDraw), GetGValue(ColorToDraw), GetBValue(ColorToDraw))); // Í¸Ã÷¶È 128
	Point Pts[] = { Point(rc.left,rc.top),Point(rc.right,rc.top),Point(rc.left,rc.bottom),Point(rc.right,rc.bottom) };
	Gdiplus::Graphics graphics(hdc);
	//graphics.FillPolygon(&m_pBrush, Pts, 4, FillModeAlternate);
	Gdiplus::Rect RRect(rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top);
	graphics.FillRectangle(&m_pBrush, RRect);
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

void SaveQRCode()
{
	WCHAR FileP[MAX_PATH] = { NULL };
	OPENFILENAME OpenFN = { NULL };
	OpenFN.lStructSize = sizeof(OPENFILENAME);
	OpenFN.hwndOwner = MainHwnd;
	OpenFN.lpstrFilter = TEXT("PNGÍ¼Æ¬\0*.png\0\0");
	OpenFN.nMaxFile = MAX_PATH;
	OpenFN.lpstrFile = FileP;
	//OpenFN.Flags = OFN_FILEMUSTEXIST;
	OpenFN.lpstrTitle = TEXT("Ñ¡Ôñ±£´æÎ»ÖÃ");
	HDC MainHDC = GetDC(MainHwnd);
	if (GetSaveFileName(&OpenFN))
	{
		wcscat(OpenFN.lpstrFile, TEXT(".png"));
		HDC hDC = ::GetDC(MainHwnd);
		RECT rect = {0,0,700,700};
		//::GetClientRect(MainHwnd, &rect);
		HDC hDCMem = ::CreateCompatibleDC(hDC);
		HBITMAP hBitMap = ::CreateCompatibleBitmap(hDC, rect.right, rect.bottom);
		HBITMAP hOldMap = (HBITMAP)::SelectObject(hDCMem, hBitMap);
		::BitBlt(hDCMem, 0, 0, rect.right, rect.bottom, hDC, 0, 0, SRCCOPY);
		CImage image;
		image.Attach(hBitMap);
		image.Save(OpenFN.lpstrFile, Gdiplus::ImageFormatPNG);
		image.Detach();
		::SelectObject(hDCMem, hOldMap);
		::DeleteObject(hBitMap);
		::DeleteDC(hDCMem);
		::DeleteDC(hDC);
	}
}