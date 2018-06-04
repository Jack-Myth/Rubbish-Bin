#include <windows.h>
#include <stdio.h>
#include <d2d1.h>
#include <wincodec.h>
#include <time.h>
#include <vector>
#pragma comment(lib,"d2d1.lib")

#define MAPSIZE 12
#define BLOCK_KIND 40

HWND MainHwnd = NULL;
ID2D1Factory* D2D1Factory = NULL;
ID2D1HwndRenderTarget* D2D1HwndRenderTarget = NULL;
IWICImagingFactory* WICImagingFactory = NULL;
RECT Windowrc = { 0,0,800,800 };
int Map[MAPSIZE][MAPSIZE] = { -1 };
std::vector<ID2D1SolidColorBrush*> BrushCollection;
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void InitD2D();
void InitGame();
void Render();
void BuildColorCache();
void CreateMap(int Mapsize = MAPSIZE);
int GetRandomBetween(int A, int B);
bool CheckIsLinkable(int AX, int AY, int BX, int BY, POINT* p);
bool CheckIsLinkableBuilding(int AX, int AY, int BX, int BY);
POINT WindowPositionToMapPosition(int X, int Y);
D2D1_POINT_2F MapPositionToPointF(int X, int Y);
D2D1_RECT_F MapPositionToRectF(int X, int Y);
ID2D1SolidColorBrush* tmpBlack = nullptr;
ID2D1SolidColorBrush* ActivedBrush = nullptr;
struct LineInfo
{
	D2D1_POINT_2U XY[4];
	ID2D1SolidColorBrush* LineColor = nullptr;
	int msRemain = 0;
} LineToDraw;
POINT ActivedBlock = {-1,-1};

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	WNDCLASSEX wnd = { NULL };
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wnd.hCursor = LoadCursor(0, IDC_ARROW);
	wnd.hIcon = LoadIcon(0, IDC_ICON);
	wnd.hInstance = hInstance;
	wnd.lpfnWndProc = MainWndProc;
	wnd.lpszClassName = TEXT("D2D Blank Window");
	RegisterClassEx(&wnd);
	AdjustWindowRect(&Windowrc, WS_OVERLAPPEDWINDOW, false);
	MainHwnd = CreateWindow(wnd.lpszClassName, TEXT("Mahjong"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, Windowrc.right - Windowrc.left, Windowrc.bottom - Windowrc.top, NULL, NULL, hInstance, NULL);
	ShowWindow(MainHwnd, nShowCmd);
	InitD2D();
	InitGame();
	SetTimer(MainHwnd, 0, 10, 0);
	MSG Msg = { NULL };
	while (GetMessage(&Msg, NULL, 0, 0) > 0)
	{
		TranslateMessage(&Msg);
		DispatchMessage(&Msg);
	}
	return 0;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_TIMER:
			{
				Render();
			}
			break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		case WM_LBUTTONDOWN:
			{
				POINT ScreenP;
				ScreenP.x = LOWORD(lParam);
				ScreenP.y = HIWORD(lParam);
				ScreenP.y -= Windowrc.top/2.f;
				//ScreenToClient(MainHwnd, &ScreenP);
				POINT MapPosition = WindowPositionToMapPosition(ScreenP.x, ScreenP.y);
				if (Map[MapPosition.x][MapPosition.y] < 0)
				{
					ActivedBlock.x = ActivedBlock.y = -1;
					break;;
				}
				if (ActivedBlock.x < 0 || ActivedBlock.y < 0)
					ActivedBlock = MapPosition;
				else
				{
					POINT LinkP[4];
					if (Map[ActivedBlock.x][ActivedBlock.y]==Map[MapPosition.x][MapPosition.y]&&CheckIsLinkable(ActivedBlock.x, ActivedBlock.y, MapPosition.x, MapPosition.y,LinkP))
					{
						for (int i=0;i<4;i++)
						{
							LineToDraw.XY[i].x = LinkP[i].x;
							LineToDraw.XY[i].y = LinkP[i].y;
						}
						LineToDraw.msRemain = 100;
						Map[ActivedBlock.x][ActivedBlock.y] = Map[MapPosition.x][MapPosition.y] = -1;
						ActivedBlock.x = ActivedBlock.y= -1;
					}
					else
					{
						ActivedBlock.x = ActivedBlock.y = -1;
					}
				}
				break;
			}
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

void Render()
{
	D2D1HwndRenderTarget->BeginDraw();
	D2D1HwndRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &tmpBlack);
	D2D1HwndRenderTarget->FillRectangle(D2D1::RectF(0, 0, Windowrc.right - Windowrc.left, Windowrc.bottom - Windowrc.top), tmpBlack);
	for (int x = 0; x < MAPSIZE; x++)
		for (int y = 0; y < MAPSIZE; y++)
		{
			if (Map[x][y]>=0)
				D2D1HwndRenderTarget->FillRectangle(MapPositionToRectF(x, y), BrushCollection[Map[x][y]]);
		}
	if (ActivedBlock.x >= 0 && ActivedBlock.y >= 0)
	{
		D2D1HwndRenderTarget->DrawRectangle(MapPositionToRectF(ActivedBlock.x, ActivedBlock.y), tmpBlack,10);
	}
	if (LineToDraw.msRemain > 0)
	{
		D2D1HwndRenderTarget->DrawLine(MapPositionToPointF(LineToDraw.XY[0].x, LineToDraw.XY[0].y), MapPositionToPointF(LineToDraw.XY[1].x, LineToDraw.XY[1].y), LineToDraw.LineColor,3);
		D2D1HwndRenderTarget->DrawLine(MapPositionToPointF(LineToDraw.XY[1].x, LineToDraw.XY[1].y), MapPositionToPointF(LineToDraw.XY[2].x, LineToDraw.XY[2].y), LineToDraw.LineColor,3);
		D2D1HwndRenderTarget->DrawLine(MapPositionToPointF(LineToDraw.XY[2].x, LineToDraw.XY[2].y), MapPositionToPointF(LineToDraw.XY[3].x, LineToDraw.XY[3].y), LineToDraw.LineColor,3);
		LineToDraw.msRemain --;
	}
	D2D1HwndRenderTarget->EndDraw();
}

void InitD2D()
{
	srand(time(NULL));
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &D2D1Factory);
	D2D1Factory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT),
		D2D1::HwndRenderTargetProperties(MainHwnd, D2D1::SizeU(Windowrc.right - Windowrc.left, Windowrc.bottom - Windowrc.top)),
		&D2D1HwndRenderTarget);
	CoInitialize(NULL);
	CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&WICImagingFactory));
}

D2D1_RECT_F MapPositionToRectF(int X,int Y)
{
	float TargetRectNum = MAPSIZE + 2;
	float left = ((Windowrc.right - Windowrc.left) / TargetRectNum)*(X + 1);
	float right = ((Windowrc.right - Windowrc.left) / TargetRectNum)*(X + 2);
	float top= ((Windowrc.bottom - Windowrc.top) / TargetRectNum)*(Y + 1);
	float buttom = ((Windowrc.bottom - Windowrc.top) / TargetRectNum)*(Y + 2);
	return D2D1::RectF(left, top, right, buttom);
}

POINT WindowPositionToMapPosition(int X, int Y)
{
	float TargetRectNum = MAPSIZE + 2;
	int mx = X / ((Windowrc.right - Windowrc.left) / TargetRectNum) - 1;
	int my = Y / ((Windowrc.bottom - Windowrc.top) / TargetRectNum) - 1;
	return { mx,my };
}

D2D1_POINT_2F MapPositionToPointF(int X, int Y)
{
	auto Recf = MapPositionToRectF(X, Y);
	float x = (Recf.right - Recf.left)*0.5 + Recf.left;
	float y = (Recf.bottom - Recf.top)*0.5 + Recf.top;
	return D2D1::Point2F(x, y);
}

void BuildColorCache()
{
	for (int i=0;i<BLOCK_KIND;i++)
	{
		ID2D1SolidColorBrush* tmp = nullptr;
		D2D1HwndRenderTarget->CreateSolidColorBrush(D2D1::ColorF(rand() / (double)(RAND_MAX), rand() / (double)(RAND_MAX), rand() / (double)(RAND_MAX) ), &tmp);
		BrushCollection.push_back(tmp);
	}
}

void InitGame()
{
	for (int x=0;x<MAPSIZE;x++)
		for (int y = 0; y < MAPSIZE; y++)
		{
			Map[x][y] = -1;
		}
	CreateMap();
	BuildColorCache();
	D2D1HwndRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &LineToDraw.LineColor);
	D2D1HwndRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Yellow), &tmpBlack);
	//D2D1HwndRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1,1,1,0.2f), &ActivedBrush);
	
}

int GetRandomBetween(int A, int B)
{
	return rand() % (B - A) + A;
}

void CreateMap(int Mapsize /* = MAPSIZE */)
{
	int Siz = MAPSIZE * MAPSIZE;
	int X1 = 0;
	int Y1 = 0;
	int X2 = 0;
	int Y2 = 0;
	while (Siz)
	{
		X1= GetRandomBetween(0, MAPSIZE );
		Y1 = GetRandomBetween(0, MAPSIZE );
		do
		{
			X2 = GetRandomBetween(0, MAPSIZE );
			Y2 = GetRandomBetween(0, MAPSIZE );
		} while (X1==X2&&Y1==Y2);
		if (Map[X1][Y1] >= 0 || Map[X2][Y2] >= 0)
			continue;
		if (CheckIsLinkableBuilding(X1, Y1, X2, Y2))
		{
			int Val = GetRandomBetween(0, BLOCK_KIND);
			Map[X1][Y1] = Map[X2][Y2] = Val;
		}
		else
		{
			continue;
		}
		Siz-=2;
	}
}

bool CheckIsLinkable(int AX, int AY, int BX, int BY,POINT* p)
{
	int X1, X2, tmp;
	int AO = Map[AX][AY];
	int BO = Map[BX][BY];
	if (AX == BX && AY == BY)
		return false;
	Map[AX][AY] = Map[BX][BY] = -1;
	for (X1 = AX; X1 >= 0; X1--)
	{
		if (Map[X1][AY]>=0)
		{
			X1++;
			break;
		}
	}
	if (!X1)
		X1--;
	for (X2 = BX; X2 >= 0; X2--)
	{
		if (Map[X2][BY]>=0)
		{
			X2++;
			break;
		}
	}
	if (!X2)
		X2++;
	X1 = X1 > X2 ? X1 : X2;
	for (X2 = AX; X2 < MAPSIZE; X2++)
	{
		if (Map[X2][AY]>=0)
		{
			X2--;
			break;
		}
	}
	if (X2 == MAPSIZE - 1)
		X2++;
	for (tmp = BX; tmp < MAPSIZE; tmp++)
	{
		if (Map[tmp][BY]>=0)
		{
			tmp--;
			break;
		}
	}
	if (tmp == MAPSIZE - 1)
		tmp++;
	X2 = X2 < tmp ? X2 : tmp;
	bool IsLinkable = false;
	for (int x = X1; x <= X2; x++)
	{
		if (x < 0 || x == MAPSIZE)
		{
			p[0].x = AX;
			p[0].y = AY;
			p[1].y = AY;
			if (x < 0)
			{
				p[1].x = -1;
				p[2].x = -1;
			}
			else
			{
				p[1].x = MAPSIZE;
				p[2].x = MAPSIZE;
			}
			p[3].x = BX;
			p[3].y = p[2].y= BY;
			return true;
		}
		IsLinkable = true;
		for (int y = AY < BY ? AY : BY; y <= (AY > BY ? AY : BY); y++)
		{
			if (Map[x][y]>=0)
			{
				Map[AX][AY] = AO;
				Map[BX][BY] = BO;
				IsLinkable = false;
				break;
			}
		}
		if (IsLinkable == true)
		{
			p[0].x = AX;
			p[0].y = AY;
			p[1].x = p[2].x = x;
			p[1].y = AY;
			p[2].y = BY;
			p[3].x = BX;
			p[3].y = BY;
			return true;
		}
	}
	Map[AX][AY] = AO;
	Map[BX][BY] = BO;
	return false;
}

bool CheckIsLinkableBuilding(int AX, int AY, int BX, int BY)
{
	int AO = Map[AX][AY];
	int BO = Map[BX][BY];
	Map[AX][AY] = Map[BX][BY] = 0;
	int X1, X2,tmp;
	for (X1=AX;X1>=0;X1--)
	{
		if (Map[X1][AY]<0)
		{
			X1++;
			break;
		}
	}
	if (!X1)
		X1--;
	for (X2 = BX; X2 >= 0; X2--)
	{
		if (Map[X2][BY] < 0)
		{
			X1++;
			break;
		}
	}
	if (!X2)
		X2++;
	X1 = X1 > X2 ? X1 : X2;
	for (X2 = AX; X2 < MAPSIZE; X2++)
	{
		if (Map[X2][AY] < 0)
		{
			X2--;
			break;
		}
	}
	if (X2==MAPSIZE-1)
		X2++;
	for (tmp = BX; tmp< MAPSIZE; tmp++)
	{
		if (Map[tmp][BY] < 0)
		{
			tmp--;
			break;
		}
	}
	if (tmp == MAPSIZE - 1)
		tmp++;
	X2 = X2 < tmp ? X2 : tmp;
	bool IsLinkable = false;
	for (int x = X1; x <= X2; x++)
	{
		if (x < 0 || x == MAPSIZE)
			return true;
		IsLinkable = true;
		for (int y = AY < BY ? AY : BY; y <= (AY > BY ? AY : BY); y++)
		{
			if (Map[x][y]<0)
			{
				Map[AX][AY] = AO;
				Map[BX][BY] = BO;
				IsLinkable = false;
				break;
			}
		}
		if (IsLinkable == true)
			return true;
	}
	Map[AX][AY] = AO;
	Map[BX][BY] = BO;
	return false;
}