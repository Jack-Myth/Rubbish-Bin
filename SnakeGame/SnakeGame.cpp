#include <windows.h>
#include <d2d1.h>
#include <stdio.h>
#include <time.h>
#include <wincodec.h>
#include <vector>
#include <winuser.h>
#pragma comment(lib,"d2d1.lib")

#define MAP_HEIGHT 24
#define MAP_WIDTH 32
#define PIXEL_PERUNIT 25.5

enum class MoveDirection
{
	MoveUp,
	MoveDown,
	MoveLeft,
	MoveRight
};

namespace Images
{
	ID2D1Bitmap* Background=NULL;
	ID2D1Bitmap* Wall=NULL;
	ID2D1Bitmap* SnakeBody=NULL;
	ID2D1Bitmap* SnakeHead=NULL;
	ID2D1Bitmap* Food=NULL;
}

class Snake
{
	std::vector<POINT> Coordinate;
	int TickClock=0;
	MoveDirection NextMoveDirection=MoveDirection::MoveLeft;
	POINT ShadowTail = {0,0};
	int MovePerTick=10; //移动速度,越小越快
public:
	Snake(std::vector<POINT> InitSnakeCoordinate);
	void SetNextMoveDirection(MoveDirection TargetMoveDirection);
	std::vector<POINT> GetSnakeCoordinate();
	void RenderFrame();
	void Tick();
};

void InitD2D();
void InitGame();
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void ProcessGameLogic();
void ProcessRender();
bool LoadD2DImage(const WCHAR ImagePath[], ID2D1Bitmap** BitMapMayClear);
POINT GetValidFoodPosition();

HWND MainHwnd=NULL;
ID2D1Factory* D2D1Factory = NULL;
ID2D1HwndRenderTarget* D2D1HwndRenderTarget = NULL;
IWICImagingFactory* WICImagingFactory = NULL;
RECT Windowrc = { 0,0,800,600 };
Snake* PlayerSnake;
bool IsGameOver = false;
POINT FoodCoordinate = {-1,-1};
char MapData[MAP_WIDTH][MAP_HEIGHT];
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
	MainHwnd = CreateWindow(wnd.lpszClassName, TEXT("Snake"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, Windowrc.right - Windowrc.left, Windowrc.bottom - Windowrc.top, NULL, NULL, hInstance, NULL);
	InitD2D();
	InitGame();
	SetTimer(MainHwnd, 0, 20, NULL);
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

void InitGame()
{
	LoadD2DImage(TEXT("Images\\Background.png"), &Images::Background);
	LoadD2DImage(TEXT("Images\\Wall.png"), &Images::Wall);
	LoadD2DImage(TEXT("Images\\SnakeBody.png"), &Images::SnakeBody);
	LoadD2DImage(TEXT("Images\\SnakeHead.png"), &Images::SnakeHead);
	LoadD2DImage(TEXT("Images\\Food.png"), &Images::Food);
	BITMAP MapBitmap;
	HANDLE hMapBitmap = LoadImage(NULL, TEXT("Images\\Map.bmp"), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE | LR_CREATEDIBSECTION);
	if (!hMapBitmap)
	{
		MessageBox(NULL, TEXT("没有有效的地图,退出"), TEXT("Load Map Error"), MB_OK);
		exit(-1);
	}
	GetObject(hMapBitmap, sizeof(BITMAP), &MapBitmap);
	if (MapBitmap.bmHeight!=MAP_HEIGHT||MapBitmap.bmWidth!=MAP_WIDTH||MapBitmap.bmBitsPixel!=24)
	{
		WCHAR errMessage[1024];
		wsprintf(errMessage, TEXT("地图大小不符合标准，应当为%dx%d的24位BMP位图"), MAP_WIDTH, MAP_HEIGHT);
		MessageBox(NULL, errMessage,TEXT("Load Map Error"), MB_OK);
		exit(-1);
	}
	std::vector<POINT> SnakePosition({ { -1,-1 } ,{ -1,-1 } ,{ -1,-1 } });
	for (int x=0;x<MAP_WIDTH;x++)
		for (int y=0;y<MAP_HEIGHT;y++)
		{
			unsigned char* MapElementInfo = ((unsigned char*)MapBitmap.bmBits) + (y*MAP_WIDTH + x)*3;
			if (!(*MapElementInfo || *(MapElementInfo + 1) || *(MapElementInfo + 2)))
			{
				MapData[x][y] = 1;
			}
			else if (*MapElementInfo >100&& *(MapElementInfo + 1)<100 && *(MapElementInfo + 2)<100)
			{
				SnakePosition[0] = { x,y };
			}
			else if (*MapElementInfo<100 && *(MapElementInfo + 1)>100 && *(MapElementInfo + 2)<100)
			{
				SnakePosition[1] = { x,y };
			}
			else if (*MapElementInfo<100 && *(MapElementInfo + 1)<100 && *(MapElementInfo + 2)>100)
			{
				SnakePosition[2] = { x,y };
			}
			else
			{
				MapData[x][y] = 0;
			}
		}
	DeleteObject(hMapBitmap);
	if (SnakePosition[0].x == -1 || SnakePosition[1].x == -1 || !SnakePosition[2].x == -1)
	{
		MessageBox(NULL, TEXT("贪食蛇的初始地点不明确，退出!"), TEXT("Error When Analysis Map"),MB_OK);
		exit(-1);
	}
	PlayerSnake = new Snake(SnakePosition);
	if (SnakePosition[1].x != SnakePosition[0].x)
		PlayerSnake->SetNextMoveDirection(SnakePosition[1].x > SnakePosition[0].x ? MoveDirection::MoveLeft : MoveDirection::MoveRight);
	else
		PlayerSnake->SetNextMoveDirection(SnakePosition[1].y > SnakePosition[0].y ? MoveDirection::MoveUp : MoveDirection::MoveDown);
	FoodCoordinate = GetValidFoodPosition();
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_TIMER:
		if (!IsGameOver)
			ProcessGameLogic();
		ProcessRender();
		break;
	case WM_KEYDOWN:
		{
			auto Cood = PlayerSnake->GetSnakeCoordinate();
			switch (wParam)
			{
			case 'W': case VK_UP:
				if (!(Cood[1].y < Cood[0].y))
					PlayerSnake->SetNextMoveDirection(MoveDirection::MoveUp);
				break;
			case 'S':case VK_DOWN:
				if (!(Cood[1].y > Cood[0].y))
					PlayerSnake->SetNextMoveDirection(MoveDirection::MoveDown);
				break;
			case 'A':case VK_LEFT:
				if (!(Cood[1].x < Cood[0].x))
					PlayerSnake->SetNextMoveDirection(MoveDirection::MoveLeft);
				break;
			case 'D':case VK_RIGHT:
				if (!(Cood[1].x > Cood[0].x))
					PlayerSnake->SetNextMoveDirection(MoveDirection::MoveRight);
				break;
			}
		}
		break;
	default:
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

void ProcessGameLogic()
{
	PlayerSnake->Tick();
}

void ProcessRender()
{
	D2D1HwndRenderTarget->BeginDraw();
	//绘制背景
	D2D1_SIZE_F HwndSizeF = D2D1::SizeF(D2D1HwndRenderTarget->GetPixelSize().width, D2D1HwndRenderTarget->GetPixelSize().height);
	D2D1HwndRenderTarget->DrawBitmap(Images::Background,
		D2D1::RectF(0, 0, HwndSizeF.width, HwndSizeF.height), 1, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, D2D1::RectF(0, 0, Images::Background->GetPixelSize().width, Images::Background->GetPixelSize().height));
	//绘制墙
	for (int x = 0; x < MAP_WIDTH; x++)
		for (int y = 0; y < MAP_HEIGHT; y++)
		{
			if (MapData[x][y])
			{
				D2D1HwndRenderTarget->DrawBitmap(Images::Wall,
					D2D1::RectF(x*PIXEL_PERUNIT, y*PIXEL_PERUNIT, (x+1)*PIXEL_PERUNIT, (y+1)*PIXEL_PERUNIT), 1, 
					D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, D2D1::RectF(0, 0, Images::Wall->GetPixelSize().width, Images::Wall->GetPixelSize().height));
			}
		}
	//绘制食物
	D2D1HwndRenderTarget->DrawBitmap(Images::Food,
		D2D1::RectF(FoodCoordinate.x*PIXEL_PERUNIT, FoodCoordinate.y*PIXEL_PERUNIT, (FoodCoordinate.x+1)*PIXEL_PERUNIT, (FoodCoordinate.y+1)*PIXEL_PERUNIT), 1, 
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, D2D1::RectF(0, 0, Images::Food->GetPixelSize().width, Images::Food->GetPixelSize().height));
	//绘制贪食蛇
	PlayerSnake->RenderFrame();
	D2D1HwndRenderTarget->EndDraw();
}

POINT GetValidFoodPosition()
{
	POINT TmpPosition;
	auto SnakeBody = PlayerSnake->GetSnakeCoordinate();
	while (1)
	{
		bool ShouldContinue=false;
		TmpPosition.x = rand() % MAP_WIDTH;
		TmpPosition.y = rand() % MAP_HEIGHT;
		if(MapData[TmpPosition.x][TmpPosition.y]==1)
			continue;
		for (int i=0;i<SnakeBody.size();i++)
		{
			if (SnakeBody[i].x == TmpPosition.x&&SnakeBody[i].y == TmpPosition.y)
			{
				ShouldContinue = true;
				break;
			}
		}
		if (ShouldContinue)
			continue;
		return TmpPosition;
	}
}

D2D1_POINT_2F LerpPosition(POINT A, POINT B, float LerpAlpha)
{
	D2D1_POINT_2F tmpPoint;
	tmpPoint.x = A.x + (B.x - A.x)*LerpAlpha;
	tmpPoint.y = A.y + (B.y - A.y)*LerpAlpha;
	return tmpPoint;
}

bool LoadD2DImage(const WCHAR ImagePath[], ID2D1Bitmap** BitMapMayClear)
{
	IWICBitmap*    m_pWicBitmap = NULL;
	IWICBitmapDecoder* m_pWicDecoder = NULL;
	IWICBitmapFrameDecode* m_pWicFrameDecoder = NULL;
	if (FAILED(WICImagingFactory->CreateDecoderFromFilename(ImagePath, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &m_pWicDecoder)))
		return false;
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
	return true;
}

Snake::Snake(std::vector<POINT> InitSnakeCoordinate)
{
	Coordinate = InitSnakeCoordinate;
	ShadowTail = InitSnakeCoordinate[2];
}

void Snake::SetNextMoveDirection(MoveDirection TargetMoveDirection)
{
	NextMoveDirection = TargetMoveDirection;
}

std::vector<POINT> Snake::GetSnakeCoordinate()
{
	return Coordinate;
}

void Snake::RenderFrame()
{
	float LerpAlpha = (float)TickClock / MovePerTick;
	D2D1_POINT_2F RenderPosition = LerpPosition(Coordinate[1], Coordinate[0], LerpAlpha);
	D2D1HwndRenderTarget->DrawBitmap(Images::SnakeHead,
		D2D1::RectF(RenderPosition.x*PIXEL_PERUNIT, RenderPosition.y*PIXEL_PERUNIT, (RenderPosition.x+1)*PIXEL_PERUNIT, (RenderPosition.y + 1)*PIXEL_PERUNIT), 1,
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, D2D1::RectF(0, 0, Images::SnakeHead->GetPixelSize().width, Images::SnakeHead->GetPixelSize().height));
	for (int i=2;i<Coordinate.size();i++)
	{
		RenderPosition = LerpPosition(Coordinate[i], Coordinate[i-1], LerpAlpha);
		D2D1HwndRenderTarget->DrawBitmap(Images::SnakeBody,
			D2D1::RectF(RenderPosition.x*PIXEL_PERUNIT, RenderPosition.y*PIXEL_PERUNIT, (RenderPosition.x + 1)*PIXEL_PERUNIT, (RenderPosition.y + 1)*PIXEL_PERUNIT), 1,
			D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, D2D1::RectF(0, 0, Images::SnakeBody->GetPixelSize().width, Images::SnakeBody->GetPixelSize().height));
	}
	RenderPosition = LerpPosition(ShadowTail, Coordinate[Coordinate.size()-1], LerpAlpha);
	D2D1HwndRenderTarget->DrawBitmap(Images::SnakeBody,
		D2D1::RectF(RenderPosition.x*PIXEL_PERUNIT, RenderPosition.y*PIXEL_PERUNIT, (RenderPosition.x + 1)*PIXEL_PERUNIT, (RenderPosition.y + 1)*PIXEL_PERUNIT), 1,
		D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, D2D1::RectF(0, 0, Images::SnakeBody->GetPixelSize().width, Images::SnakeBody->GetPixelSize().height));
}

void Snake::Tick()
{
	TickClock++;
	if (TickClock==MovePerTick)
	{
		POINT tmpLastCoodinate=*(Coordinate.end()-1);
		TickClock = 0;
		for (int i= Coordinate.size()-1;i>0;i--)
		{
			Coordinate[i] = Coordinate[i - 1];
		}
		ShadowTail = tmpLastCoodinate;
		switch (NextMoveDirection)
		{
		case MoveDirection::MoveUp:
			if (Coordinate[0].y == 0)
				Coordinate[0].y = MAP_HEIGHT-1;
			else
				Coordinate[0].y--;
			break;
		case MoveDirection::MoveDown:
			Coordinate[0].y= (Coordinate[0].y+1)%MAP_HEIGHT;
			break;
		case MoveDirection::MoveLeft:
			if (Coordinate[0].x == 0)
				Coordinate[0].x = MAP_WIDTH - 1;
			else
				Coordinate[0].x--;
			break;
		case MoveDirection::MoveRight:
			Coordinate[0].x=(Coordinate[0].x+1)%MAP_WIDTH;
			break;
		}
		if (MapData[Coordinate[0].x][Coordinate[0].y] == 1)
		{
			IsGameOver = true;
			MessageBox(NULL, TEXT("你撞到墙了哦"), TEXT("游戏结束"), MB_OK);
		} 
		else if (FoodCoordinate.x == Coordinate[0].x&&FoodCoordinate.y == Coordinate[0].y)
		{
			Coordinate.push_back(tmpLastCoodinate);
			if(MovePerTick>2)
				MovePerTick--;
			FoodCoordinate = GetValidFoodPosition();
		}
		else
		{
			for (int i=1;i<Coordinate.size();i++)
			{
				if (Coordinate[0].x	==Coordinate[i].x&&Coordinate[0].y==Coordinate[i].y)
				{
					IsGameOver = true;
					MessageBox(NULL, TEXT("你咬到自己了哦"), TEXT("游戏结束"), MB_OK);
					return;
				}
			}
		}
	}
}

