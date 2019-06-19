// TestMFC.cpp : Defines the entry point for the application.
//

#include "windows.h"
#include "framework.h"
#include "TGraphics.h"
#include "Resource.h"
#include <vector>
#include <commdlg.h>
#include <gdiplus.h>
#include <CommCtrl.h>
#pragma comment (lib,"GdiPlus.lib")

#define MAX_LOADSTRING 100

#define SHAPE_RECTANGLE 0
#define SHAPE_CIRCLE 1
#define DRAW_MODE_FILL 0
#define DRAW_MODE_OPAQUE 1

using namespace Gdiplus;

struct Shape
{
	int L=0, T=0, R=0, D=0;
	COLORREF Color;
	int LineWidth;
	UINT TargetShape = SHAPE_RECTANGLE;
	UINT TargetRenderMode = DRAW_MODE_FILL;
};

BOOL IsPointInShape(Shape* mShape, int X, int Y)
{
	return X >= mShape->L && X <= mShape->R && Y >= mShape->T && Y <= mShape->D;
}

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK	Width(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

// Render
UINT RenderShape = SHAPE_RECTANGLE;
UINT RenderMode = DRAW_MODE_FILL;

//Shape Array
std::vector<Shape> Shapes;
Shape* tmpShape = NULL;
int LineWidth = 1;
int tmpLineWidth = 1;
COLORREF TemplateColor=RGB(0,0,255);
BOOL NeedReverse = FALSE;
Shape* curShape = NULL;
BOOL MovingShape = FALSE;
POINT OriginalPoint;
Shape tmpOriginalShape;
LPWSTR CursorID=IDC_ARROW;
float Scale = 1.f;

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

	GdiplusStartupInput gdiplusStartupInput;
	ULONG_PTR pGdiToken;
	GdiplusStartup(&pGdiToken, &gdiplusStartupInput, NULL);//Init GDI+

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TGRAPHICS, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TGRAPHICS));

    MSG msg;

    // Main message loop:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}


void ConvertPoint(HWND hWnd,POINT* p)
{
	RECT r;
	GetClientRect(hWnd, &r);
	int m_bufW = r.right - r.left;
	int m_bufH = r.bottom - r.top;
	p->x -= m_bufW / 2;
	p->y -= m_bufH / 2;
	p->x = (LONG)(p->x / Scale);
	p->y = (LONG)(p->y / (NeedReverse ? -Scale : Scale));
	p->x += m_bufW / 2;
	p->y += m_bufH / 2;
}

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TGRAPHICS));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TGRAPHICS);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   SetTimer(hWnd, 1, 10, NULL);
   UpdateWindow(hWnd);
   RegisterHotKey(hWnd, 1, MOD_CONTROL , 'V');
   RegisterHotKey(hWnd, 2, MOD_CONTROL , 'R');
   RegisterHotKey(hWnd, 3, MOD_CONTROL , VK_UP);
   RegisterHotKey(hWnd, 4, MOD_CONTROL , VK_DOWN);
   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_LBUTTONDOWN:
	{
		POINT CurPoint;
		GetCursorPos(&CurPoint);
		ScreenToClient(hWnd, &CurPoint);
		ConvertPoint(hWnd, &CurPoint);
		BOOL AnyShapes = FALSE;
		for (unsigned int i = 0; i < Shapes.size(); i++)
		{
			if (IsPointInShape(&Shapes[i], CurPoint.x, CurPoint.y))
			{
				tmpOriginalShape = Shapes[i];
				OriginalPoint = CurPoint;
				curShape = &Shapes[i];
				MovingShape = TRUE;
				AnyShapes = TRUE;
				SetTimer(hWnd, 0, 10, NULL);
				break;
			}
		}
		if (AnyShapes)
			break;

		if (!tmpShape)
			tmpShape=new Shape();
		tmpShape->Color = TemplateColor;
		tmpShape->LineWidth = LineWidth;
		tmpShape->L = CurPoint.x;
		tmpShape->T = CurPoint.y;
		tmpShape->R = CurPoint.x;
		tmpShape->D = CurPoint.y;
		tmpShape->TargetShape = RenderShape;
		tmpShape->TargetRenderMode = RenderMode;
		SetTimer(hWnd, 0,10,NULL);
		break;
	}
	case WM_MOUSEMOVE:
	{
		POINT CurPoint;
		GetCursorPos(&CurPoint);
		ScreenToClient(hWnd, &CurPoint);
		ConvertPoint(hWnd, &CurPoint);
		BOOL AnyShapes=FALSE;
		for (unsigned int i = 0; i < Shapes.size(); i++)
		{
			if (IsPointInShape(&Shapes[i], CurPoint.x, CurPoint.y))
			{
				if (CursorID != IDC_HAND)
				{
					SetClassLong(hWnd, GCL_HCURSOR, (long)LoadCursor(NULL, IDC_HAND));
					CursorID = IDC_HAND;
				}
				AnyShapes = TRUE;
				break;
			}
		}
		if (!AnyShapes)
			if (CursorID != IDC_ARROW)
			{
				SetClassLong(hWnd, GCL_HCURSOR, (long)LoadCursor(NULL, IDC_ARROW));
				CursorID = IDC_ARROW;
			}
		break;
	}
	case WM_RBUTTONDOWN:
	{
		POINT CurPoint;
		GetCursorPos(&CurPoint);
		ScreenToClient(hWnd, &CurPoint);
		ConvertPoint(hWnd, &CurPoint);
		for (int i= Shapes.size() -1;i>=0;i--)
		{
			if (IsPointInShape(&Shapes[i], CurPoint.x, CurPoint.y))
			{
				curShape = &Shapes[i];
				HMENU PopedMenu = GetSubMenu(LoadMenu(hInst, MAKEINTRESOURCE(IDR_MENU1)),0);
				GetCursorPos(&CurPoint);
				TrackPopupMenu(PopedMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, CurPoint.x, CurPoint.y, 0, hWnd, NULL);
				break;
			}
		}
		break;
	}
	case WM_TIMER:
	{
		switch (wParam)
		{
		case 0:
		{

			if (!GetAsyncKeyState(VK_LBUTTON))
			{
				//Unpressed
				if (MovingShape)
				{
					MovingShape = FALSE;
				}
				else
				{
					if (tmpShape)
					{

						Shapes.push_back(*tmpShape);
						if (tmpShape)
						{
							delete tmpShape;
							tmpShape = NULL;
						}
					}
				}
				KillTimer(hWnd, 0);
			}
			else	
			{
				//Still Pressed
				//Track tmpShape
				if (MovingShape)
				{
					POINT CurPoint;
					GetCursorPos(&CurPoint);
					ScreenToClient(hWnd, &CurPoint);
					ConvertPoint(hWnd, &CurPoint);
					curShape->L = CurPoint.x - OriginalPoint.x + tmpOriginalShape.L;
					curShape->R = CurPoint.x - OriginalPoint.x + tmpOriginalShape.R;
					curShape->T = CurPoint.y - OriginalPoint.y + tmpOriginalShape.T;
					curShape->D = CurPoint.y - OriginalPoint.y + tmpOriginalShape.D;
				}
				else
				{
					if (tmpShape)
					{
						POINT CurPoint;
						GetCursorPos(&CurPoint);
						ScreenToClient(hWnd, &CurPoint);
						ConvertPoint(hWnd, &CurPoint);
						tmpShape->R = CurPoint.x;
						tmpShape->D = CurPoint.y;
					}
				}
			}
			break;
		}
		case 1:
		{
			RECT r;
			GetClientRect(hWnd, &r);
			int m_bufW = r.right-r.left; 
			int m_bufH = r.bottom-r.top;
			Bitmap memBitmap(m_bufW, m_bufH);
			Graphics gp(&memBitmap);
			gp.Clear(Gdiplus::Color(255, 255, 255, 255));
			gp.TranslateTransform(m_bufW / 2.f, m_bufH / 2.f);
			if (NeedReverse)
				gp.ScaleTransform(Scale, -Scale);
			else
				gp.ScaleTransform(Scale, Scale);
			gp.TranslateTransform(-m_bufW / 2.f, -m_bufH / 2.f);
			std::vector<Shape> tmpShapes;
			tmpShapes = Shapes;
			if (tmpShape)
				tmpShapes.push_back(*tmpShape);
			for (unsigned int i = 0; i < tmpShapes.size(); i++)
			{
				BYTE R = GetRValue(tmpShapes[i].Color);
				BYTE G = GetGValue(tmpShapes[i].Color);
				BYTE B = GetBValue(tmpShapes[i].Color);
				Color c(255,R , G, B);
				Pen p(c, (Gdiplus::REAL)tmpShapes[i].LineWidth);
				SolidBrush sb(c);
				switch (tmpShapes[i].TargetShape)
				{
				case SHAPE_CIRCLE:
					if (tmpShapes[i].TargetRenderMode == DRAW_MODE_OPAQUE)
						gp.DrawEllipse(&p, tmpShapes[i].L, tmpShapes[i].T, tmpShapes[i].R - tmpShapes[i].L, tmpShapes[i].D - tmpShapes[i].T);
					else
						gp.FillEllipse(&sb, tmpShapes[i].L, tmpShapes[i].T, tmpShapes[i].R- tmpShapes[i].L, tmpShapes[i].D - tmpShapes[i].T);
					break;
				case SHAPE_RECTANGLE:
					if (tmpShapes[i].TargetRenderMode == DRAW_MODE_OPAQUE)
						gp.DrawRectangle(&p, tmpShapes[i].L, tmpShapes[i].T, tmpShapes[i].R- tmpShapes[i].L, tmpShapes[i].D - tmpShapes[i].T);
					else
						gp.FillRectangle(&sb, tmpShapes[i].L, tmpShapes[i].T, tmpShapes[i].R- tmpShapes[i].L, tmpShapes[i].D - tmpShapes[i].T);
					break;
				default:
					break;
				}
			}
			HDC m_hDC = GetDC(hWnd);
			Graphics gr(m_hDC);
			gr.DrawImage(&memBitmap, 0, 0);
			ReleaseDC(hWnd, m_hDC);
		}
		default:
			break;
		}
	}
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
			case ID_GRAPHICS_CIRCLE:
			{
				HMENU GraphicsMenu = GetSubMenu(GetMenu(hWnd), 1);
				CheckMenuItem(GraphicsMenu, ID_GRAPHICS_CIRCLE, MF_CHECKED);
				CheckMenuItem(GraphicsMenu, ID_GRAPHICS_RECTANGLE, MF_UNCHECKED);
				RenderShape = SHAPE_CIRCLE;
				break;
			}
			case ID_GRAPHICS_RECTANGLE:
			{
				HMENU GraphicsMenu = GetSubMenu(GetMenu(hWnd), 1);
				CheckMenuItem(GraphicsMenu, ID_GRAPHICS_CIRCLE, MF_UNCHECKED);
				CheckMenuItem(GraphicsMenu, ID_GRAPHICS_RECTANGLE, MF_CHECKED);
				RenderShape = SHAPE_RECTANGLE;
				break;
			}
			case ID_OPTION_FILL:
			{
				HMENU GraphicsMenu = GetSubMenu(GetMenu(hWnd), 2);
				CheckMenuItem(GraphicsMenu, ID_OPTION_FILL, MF_CHECKED);
				CheckMenuItem(GraphicsMenu, ID_OPTION_OPAQUE, MF_UNCHECKED);
				RenderMode = DRAW_MODE_FILL;
				break;
			}
			case ID_OPTION_OPAQUE:
			{
				HMENU GraphicsMenu = GetSubMenu(GetMenu(hWnd), 2);
				CheckMenuItem(GraphicsMenu, ID_OPTION_FILL, MF_UNCHECKED);
				CheckMenuItem(GraphicsMenu, ID_OPTION_OPAQUE, MF_CHECKED);
				RenderMode = DRAW_MODE_OPAQUE;
				break;
			}
			case ID_OPTION_COLOR:
			{
				COLORREF ccx[16];
				CHOOSECOLOR cc = {NULL};
				cc.lStructSize = sizeof(CHOOSECOLOR);
				cc.hwndOwner = hWnd;
				cc.rgbResult = TemplateColor;
				cc.Flags = CC_ANYCOLOR;
				cc.lpCustColors = ccx;
				if (ChooseColor(&cc))
					TemplateColor = cc.rgbResult;
				break;
			}
			case ID_OPTION_WIDTH:
			{
				DialogBox(hInst, MAKEINTRESOURCE(IDD_DIALOG1), hWnd, &Width);
				break;
			}
			case ID_OPERATION_REVERSE:
			{
				NeedReverse = !NeedReverse;
				break;
			}
			case ID_OPERATION_RESET:
			{
				NeedReverse = FALSE;
				Scale = 1.f;
				break;
			}
			case ID_BRING_BRINGTOFRONT:
			{
				for (unsigned int i = 0; i < Shapes.size(); i++)
				{
					if (&Shapes[i]==curShape)
					{
						Shape tmpShape = Shapes[i];
						Shapes.erase(Shapes.begin() + i);
						Shapes.push_back(tmpShape);
					}
				}
				break;
			}
			case ID_BRING_BRINGTOBACK:
			{
				for (unsigned int i = 0; i < Shapes.size(); i++)
				{
					if (&Shapes[i] == curShape)
					{
						Shape tmpShape = Shapes[i];
						Shapes.erase(Shapes.begin() + i);
						Shapes.insert(Shapes.begin(), tmpShape);
					}
				}
				break;
			}
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
	case WM_HOTKEY:
		switch (wParam)
		{
		case 1:
		{
			NeedReverse = !NeedReverse;
			break;
		}
		case 2:
		{
			NeedReverse = FALSE;
			Scale = 1.f;
			break;
		}
		case 3:
		{
			Scale += 0.1f;
			break;
		}
		case 4:
		{
			Scale -= 0.05f;
			break;
		}
		default:
			break;
		}
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
			
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

INT_PTR CALLBACK Width(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
	{
		HWND hwndTrack = CreateWindowEx(
			0, // no extended styles
			TRACKBAR_CLASS, // class name
			TEXT("Trackbar Control"), // title (caption)
			WS_CHILD | WS_VISIBLE |TBS_AUTOTICKS |TBS_HORZ, // style
			10, 10, // position
			200, 30, // size
			hDlg, // parent window
			(HMENU)1001, // control identifier
			hInst, // instance
			NULL // no WM_CREATE parameter
		);
		SendMessage(hwndTrack, TBM_SETRANGEMAX, FALSE, 10);
		SendMessage(hwndTrack, TBM_SETRANGEMIN, FALSE, 1);
		SendMessage(hwndTrack, TBM_SETPOS, TRUE, LineWidth);
	}
		return (INT_PTR)TRUE;
	case WM_HSCROLL:
	{
		if (LOWORD(wParam) == SB_THUMBPOSITION || LOWORD(wParam) == SB_THUMBTRACK)
			tmpLineWidth = HIWORD(wParam);
		break;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		else if (LOWORD(wParam) == IDOK)
		{
			LineWidth = tmpLineWidth;
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}