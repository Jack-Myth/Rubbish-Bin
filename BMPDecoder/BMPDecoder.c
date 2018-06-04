#include <windows.h>
#include <stdio.h>
#include <locale.h>
#include <math.h>

/*
typedef struct tagBITMAPFILEHEADER
{
	UINT16 bfType;
	DWORD bfSize;
	UINT16 bfReserved1;
	UINT16 bfReserved2;
	DWORD bfOffBits;
} BITMAPFILEHEADER;

typedef struct tagBITMAPINFOHEADER
{
	DWORD biSize;
	LONG biWidth;
	LONG biHeight;
	WORD biPlanes;
	WORD biBitCount;
	DWORD biCompression;
	DWORD biSizeImage;
	LONG biXPelsPerMeter;
	LONG biYPelsPerMeter;
	DWORD biClrUsed;
	DWORD biClrImportant;
} BITMAPINFOHEADER;*/

typedef struct tagARGB
{
	unsigned char Blue;
	unsigned char Green;
	unsigned char Red;
	unsigned char Alpha;
} tARGB;

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
//void DecodeRLE8(FILE* File, BITMAPFILEHEADER BitmapFileHeader, BITMAPINFOHEADER BitmapInfoHeader, tARGB ColorPlane[]);
void Decode();
inline void SetTitle(int Now, int Max);


HWND MainHwnd = NULL;
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
	wnd.style = CS_DBLCLKS;
	RegisterClassEx(&wnd);
	MainHwnd = CreateWindow(wnd.lpszClassName, TEXT("BMPDecoder--Double Click To Choose File"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1280, 720, NULL, NULL, hInstance, NULL);
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
	case WM_LBUTTONDBLCLK:
		Decode();
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
		break;
	}
	return 0;
}

void Decode()
{
	WCHAR  FileName[255] = { NULL };
	char FilePath[255] = { NULL };
	OPENFILENAME OpenFN = { NULL };
	OpenFN.Flags = OFN_FILEMUSTEXIST;
	OpenFN.lpstrFile = FileName;
	OpenFN.lpstrTitle = TEXT("Pick BMP File");
	OpenFN.lStructSize = sizeof(OPENFILENAME);
	OpenFN.lpstrFilter = TEXT("BMPFile\0*.bmp\0\0");
	OpenFN.nMaxFile = MAX_PATH;
	if (!GetOpenFileName(&OpenFN))
		return;
	FILE* File = NULL;
	setlocale(LC_ALL, "");
	wcstombs(FilePath, OpenFN.lpstrFile, 255);
	File= fopen(FilePath,"rb");
	BITMAPFILEHEADER BitmapFileHeader = {NULL};
	BITMAPINFOHEADER BitmapInfoHeader = { NULL };
	//fgets((char*)&BitmapFileHeader, sizeof(BITMAPFILEHEADER), File);
	fread(&BitmapFileHeader, sizeof(BITMAPFILEHEADER), 1, File);
	fread(&BitmapInfoHeader, sizeof(BITMAPINFOHEADER), 1, File);
	BitmapInfoHeader.biHeight = abs(BitmapInfoHeader.biHeight);
	if (BitmapInfoHeader.biBitCount<24)
	{
		tARGB* BGRA = (tARGB*)malloc(sizeof(tARGB)*(unsigned int)pow(2, BitmapInfoHeader.biBitCount));
		for (int i = 0; i < pow(2, BitmapInfoHeader.biBitCount); i++)
		{
			fread(&BGRA[i], 4, 1, File);
		}
		fseek(File, BitmapFileHeader.bfOffBits, SEEK_SET);
		if (BitmapInfoHeader.biCompression== BI_RLE8)
		{
			MessageBox(MainHwnd, TEXT("暂不支持RLE压缩算法"), TEXT("!"), MB_OK);
			return;
			//DecodeRLE8(File, BitmapFileHeader, BitmapInfoHeader, BGRA);
		} 
		else
		{
			HDC MainHDC = GetDC(MainHwnd);
			unsigned char PixelData = NULL;
			for (int Height = 0; Height < BitmapInfoHeader.biHeight; Height++)
			{
				for (int Width = 0; Width < BitmapInfoHeader.biWidth; Width++)
				{
					fread(&PixelData, 1, 1, File);
					SetPixel(MainHDC, Width, BitmapInfoHeader.biHeight - Height, RGB(BGRA[PixelData].Red, BGRA[PixelData].Green, BGRA[PixelData].Blue));
				}
				if ((BitmapInfoHeader.biWidth*BitmapInfoHeader.biBitCount) % 32 != 0)
					fseek(File, 4 - (BitmapInfoHeader.biWidth * BitmapInfoHeader.biBitCount / 8 % 4), SEEK_CUR);
				SetTitle(Height, BitmapInfoHeader.biHeight);
			}
			SetTitle(1, 1);
			ReleaseDC(MainHwnd, MainHDC);
			free(BGRA);
		}
	} 
	else
	{
		HDC MainHDC = GetDC(MainHwnd);
		tARGB BGRA = { NULL };
		COLORREF OriColor=0;
		fseek(File, BitmapFileHeader.bfOffBits, SEEK_SET);
		if (BitmapInfoHeader.biBitCount < 32)
		{
			for (int Height = 0; Height < BitmapInfoHeader.biHeight; Height++)
			{
				for (int Width = 0; Width < BitmapInfoHeader.biWidth; Width++)
				{
					fread(&BGRA, BitmapInfoHeader.biBitCount / 8, 1, File);
					SetPixel(MainHDC, Width, BitmapInfoHeader.biHeight - Height, RGB(BGRA.Red, BGRA.Green, BGRA.Blue));
				}
				if ((BitmapInfoHeader.biWidth*BitmapInfoHeader.biBitCount) % 32 != 0)
					fseek(File, 4 - (BitmapInfoHeader.biWidth * BitmapInfoHeader.biBitCount / 8 % 4), SEEK_CUR);
				SetTitle(Height, BitmapInfoHeader.biHeight);
			}
		}
		else
		{
			RECT Rc;
			GetClientRect(MainHwnd, &Rc);
			BITMAPINFO bmpInfo; //创建位图 
			bmpInfo.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
			bmpInfo.bmiHeader.biWidth = Rc.right- Rc.left;//宽度
			bmpInfo.bmiHeader.biHeight = Rc.bottom- Rc.top;//高度
			bmpInfo.bmiHeader.biPlanes = 1;
			bmpInfo.bmiHeader.biBitCount = 24;
			bmpInfo.bmiHeader.biCompression = BI_RGB;
			int uiTotalBytes = bmpInfo.bmiHeader.biWidth * bmpInfo.bmiHeader.biHeight * 3;
			char *pArray = (char*)malloc(uiTotalBytes);
			free(pArray);
			HBITMAP hbmp = CreateDIBSection(NULL, &bmpInfo, DIB_RGB_COLORS, &pArray, NULL, 0);//创建DIB
			HDC MemDC= CreateCompatibleDC(MainHDC);
			SelectObject(MemDC, hbmp);
			BitBlt(MemDC, 0, 0, bmpInfo.bmiHeader.biWidth, bmpInfo.bmiHeader.biHeight, MainHDC,0,0, SRCCOPY);
			int OriOffset = BitmapInfoHeader.biHeight - bmpInfo.bmiHeader.biHeight;
			for (int Height = 0; Height < BitmapInfoHeader.biHeight; Height++)
			{
				for (int Width = 0; Width < BitmapInfoHeader.biWidth; Width++)
				{
					fread(&BGRA, BitmapInfoHeader.biBitCount / 8, 1, File);
					//rgbQ = bmpInf->bmiColors[buf[Height*bmpInf->bmiHeader.biWidth + Width]];
					if (Height - OriOffset-1>0 &&((Height - OriOffset-1)*bmpInfo.bmiHeader.biWidth + Width) * 3<uiTotalBytes)
						OriColor = RGB(*(pArray + ((Height- OriOffset-1)*bmpInfo.bmiHeader.biWidth + Width) * 3+2)
							, *(pArray + ((Height - OriOffset-1)*bmpInfo.bmiHeader.biWidth + Width) * 3 + 1)
							, *(pArray + ((Height - OriOffset-1)*bmpInfo.bmiHeader.biWidth + Width) * 3 ));
					SetPixel(MainHDC, Width, BitmapInfoHeader.biHeight - Height, 
						RGB(BGRA.Red*(float)BGRA.Alpha / 255 + GetRValue(OriColor) * ((float)(255 - BGRA.Alpha) / 255)
						, BGRA.Green*(float)BGRA.Alpha / 255 + GetGValue(OriColor) * ((float)(255 - BGRA.Alpha) / 255)
						, BGRA.Blue*(float)BGRA.Alpha / 255 + GetBValue(OriColor) * ((float)(255 - BGRA.Alpha) / 255)));
				}
				if ((BitmapInfoHeader.biWidth*BitmapInfoHeader.biBitCount) % 32 != 0)
					fseek(File, 4 - (BitmapInfoHeader.biWidth * BitmapInfoHeader.biBitCount / 8 % 4), SEEK_CUR);
				SetTitle(Height, BitmapInfoHeader.biHeight);
			}
			DeleteDC(MemDC);
			DeleteObject(hbmp);
		}
		SetTitle(1,1);
		ReleaseDC(MainHwnd, MainHDC);
	}
	fclose(File);
}

inline void SetTitle(int Now, int Max)
{
	char A[50] = { NULL };
	sprintf(A, "BMPDecoder --Processing, %.2f%% Finished", ((float)Now / Max)*100);
	SetWindowTextA(MainHwnd, A);
}

/*void DecodeRLE8(FILE* File, BITMAPFILEHEADER BitmapFileHeader, BITMAPINFOHEADER BitmapInfoHeader, tARGB ColorPlane[])
{
	char RLEFlag[2] = { NULL };
	unsigned char* RLEData = NULL;
	int Width = 0,Height=0;
	HDC MainHDC = GetDC(MainHwnd);
	long FT;
	while (1)
	{
		fread(RLEFlag, 2, 1, File);
		if (RLEFlag[0] == 0)
		{
			switch (RLEFlag[1])
			{
			case 1:
				ReleaseDC(MainHwnd, MainHDC);
				char aa[10];
				FT=ftell(File);
				sprintf(aa, "%ld", FT);
				MessageBoxA(NULL, aa, " ", MB_OK);
				return;
			case 0:
				Height++;
				Width = 0;
				break;
			case 2:
				RLEData = (unsigned char*)malloc(sizeof(char)*2);
				fread(RLEData, 2, 1, File);
				Width += (int)RLEData[0];
				Height += (int)RLEData[1];
				free(RLEData);
				break;
			default:
				//Absolute
				RLEData = (unsigned char*)malloc(sizeof(char) * 2);
				RLEData[1] = 0;
				if (RLEFlag[1]%4!=0)
					RLEData[1] = 2 - (RLEFlag[1] % 2);
				while (1)
				{
					for (; Width < BitmapInfoHeader.biWidth; Width++)
					{
						fread(RLEData, 1, 1, File);
						SetPixel(MainHDC, Width, BitmapInfoHeader.biHeight - Height, RGB(ColorPlane[RLEData[0]].Red, ColorPlane[RLEData[0]].Green, ColorPlane[RLEData[0]].Blue));
						RLEFlag[1]--;
						if (RLEFlag[1] <= 0)
							break;
					}
					if (RLEFlag[1] <= 0)
						break;
					Width = 0;
					Height++;
				
				}
				if (RLEData[1])
					fseek(File, RLEData[1], SEEK_CUR);
				free(RLEData);
				break;
			}
		}
		else
		{
			while (1)
			{
				for (; Width < BitmapInfoHeader.biWidth; Width++)
				{
					SetPixel(MainHDC, Width, BitmapInfoHeader.biHeight - Height, RGB(ColorPlane[((unsigned char*)RLEFlag)[1]].Red, ColorPlane[((unsigned char*)RLEFlag)[1]].Green, ColorPlane[((unsigned char*)RLEFlag)[1]].Blue));
					RLEFlag[0]--;
					if (RLEFlag[0] <= 0)
						break;
				}
				if (RLEFlag[0] <= 0)
					break;
				Width = 0;
				Height++;
			}
		}
	}
}*/