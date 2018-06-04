#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <locale.h>
#include <time.h>
#include <math.h>
#include <stdio.h>
#pragma comment(lib,"d2d1.lib")//D2D1链接库
#pragma comment (lib, "dwrite.lib")//DWrite链接库
#define MOVESPEED 20

enum ANIMSTAGE
{
	AnimStage_None,
	AnimStage_Bigger,
	AnimStage_MoveLeft,
	AnimStage_MoveRight,
	AnimStage_MoveUp,
	AnimStage_MoveDown,
};

class GameCube
{
	int CubeSize = 0;
	GameCube** GameCubeREF;
public:
	int CubeLevel = 0;
	ANIMSTAGE AnimStage = AnimStage_Bigger;
	D2D1_POINT_2F CubePosition;
	GameCube(D2D1_POINT_2F,int, GameCube**);
	void FrameNotify();
	void Move(ANIMSTAGE);
	void Merge();
};

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void InitMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd);
void InitGame();
void GameRender();
GameCube* FindCube(const GameCube* GameC, ANIMSTAGE DirectionToFind);
GameCube* FindNearCube(const GameCube* GameC, ANIMSTAGE DirectionToFind);
GameCube* FindCubeOnEdge(ANIMSTAGE DirectionToFind, float CubeAxis);
bool IsPositionHaveCube(D2D1_POINT_2F Position);
bool IsAnyCubeMoving();
void SpawnCube();
inline bool IsPointEqual(D2D1_POINT_2F A, D2D1_POINT_2F B);
bool IsMovableLocal(ANIMSTAGE AS);
bool IsMovableOneLine(ANIMSTAGE AS, int MoveAxis);
inline bool IsMovableGlobal();
inline bool Have2048();

ID2D1Factory* D2D1Factory = NULL;
IDWriteFactory* DWriteFactory = NULL;
ID2D1HwndRenderTarget* D2D1HwndRenderTarget = NULL;
HWND MainHwnd = NULL;
RECT WindowRect = { 0,0,405,455};
IDWriteTextFormat* D2DTextFormat = NULL;
GameCube* CubeCollection[16] = { NULL };
int Score = 0;
ID2D1SolidColorBrush* LevelBrushCollection[10];
bool Spawned =true;
ID2D1SolidColorBrush* BlackBrush = NULL;
bool WinOrFaild = false;
WCHAR Hint[10] = TEXT("双击重新游戏");
int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	InitMain(hInstance, hPrevInstance, lpCmdLine, nShowCmd);
	MSG msg = { NULL };
	while (msg.message != WM_QUIT)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		GetMessage(&msg, 0, 0, 0);
	}
	return 0;
}

void InitMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	WNDCLASSEX wnd = { NULL };
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wnd.hCursor = LoadCursor(0, IDC_ARROW);
	wnd.hIcon = LoadIcon(0, IDC_ICON);
	wnd.hInstance = hInstance;
	wnd.lpfnWndProc = MainWndProc;
	wnd.lpszClassName = TEXT("Win32 Window");
	wnd.style = CS_DBLCLKS;
	RegisterClassEx(&wnd);
	AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, false);
	MainHwnd = CreateWindow(wnd.lpszClassName, TEXT("2048 C++Ver By JackMyth"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, NULL, NULL, hInstance, NULL);
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &D2D1Factory);
	D2D1Factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(MainHwnd, D2D1::SizeU(405, 455)), &D2D1HwndRenderTarget);
	setlocale(LC_ALL, "");
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&DWriteFactory));
	DWriteFactory->CreateTextFormat(TEXT("微软雅黑"), NULL,
		DWRITE_FONT_WEIGHT_REGULAR,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL, 30, TEXT("zh-cn"), &D2DTextFormat);
	for (int i = 0; i < 10; i++)
		D2D1HwndRenderTarget->CreateSolidColorBrush(D2D1::ColorF(RGB(255, 255, 255 - (i + 1) * 25), 1.0F), &LevelBrushCollection[i]);
	D2D1HwndRenderTarget->CreateSolidColorBrush(D2D1::ColorF(RGB(0, 0, 0), 1.0F), &BlackBrush);
	InitGame();
	SetTimer(MainHwnd, 0, 17, NULL);
	ShowWindow(MainHwnd, nShowCmd);
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_TIMER:
		if (Spawned == false && !IsAnyCubeMoving())
		{
			SpawnCube();
			Spawned = true;
			if (Have2048())
			{
				wcscpy(Hint, TEXT("你获得了胜利"));
				WinOrFaild = true;
			}
			else if (!IsMovableGlobal())
				wcscpy(Hint, TEXT("游戏结束"));
		}
		GameRender();
		break;
	case WM_KEYDOWN:
		if (IsAnyCubeMoving()||WinOrFaild)
			break;
		switch (wParam)
		{
		case VK_LEFT:
			if (IsMovableLocal(AnimStage_MoveLeft))
			{
				Spawned = false;
				for (int i = 5; i < 400; i += 100)
				{
					if (IsMovableOneLine(AnimStage_MoveLeft,i)&&FindCubeOnEdge(AnimStage_MoveRight, i))
						FindCubeOnEdge(AnimStage_MoveRight, i)->Move(AnimStage_MoveLeft);
				}
			}
			break;
		case VK_RIGHT:
			if (IsMovableLocal(AnimStage_MoveRight))
			{
				Spawned = false;
				for (int i = 5; i < 400; i += 100)
				{
					if (IsMovableOneLine(AnimStage_MoveRight,i) && FindCubeOnEdge(AnimStage_MoveLeft, i))
						FindCubeOnEdge(AnimStage_MoveLeft, i)->Move(AnimStage_MoveRight);
				}
			}
			break;
		case VK_UP:
			if (IsMovableLocal(AnimStage_MoveUp))
			{
				Spawned = false;
				for (int i = 5; i < 400; i += 100)
				{
					if (IsMovableOneLine(AnimStage_MoveUp,i) && FindCubeOnEdge(AnimStage_MoveDown, i))
						FindCubeOnEdge(AnimStage_MoveDown, i)->Move(AnimStage_MoveUp);
				}
			}
			break;
		case VK_DOWN:
			if(IsMovableLocal(AnimStage_MoveDown))
			{
				Spawned = false;
				for (int i = 5; i < 400; i += 100)
				{
					if (IsMovableOneLine(AnimStage_MoveDown,i) && FindCubeOnEdge(AnimStage_MoveUp, i))
						FindCubeOnEdge(AnimStage_MoveUp, i)->Move(AnimStage_MoveDown);
				}
			}
			break;
		default:
			break;
		}
		break;
	case WM_SIZE:
		break;
	case WM_LBUTTONDBLCLK:
		if (LOWORD(lParam)>205&& HIWORD(lParam)>410)
		{
			for (int i=0;i<16;i++)
			{
				if (CubeCollection[i])
					delete CubeCollection[i];
				CubeCollection[i] = NULL;
			}
			wcscpy(Hint, TEXT("双击重新开始"));
			Score = 0;
			SpawnCube();
			SpawnCube();
		}
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
		break;
	}
	return 0;
}

void InitGame()
{
	srand((unsigned int)time(NULL));
	D2D1_POINT_2F DP2F = D2D1::Point2F(rand() % 4 * 100 + 5, rand() % 4 * 100 + 5);
	CubeCollection[0] = new GameCube(DP2F, 0, &CubeCollection[0]);
	DP2F = D2D1::Point2F(rand() % 4 * 100 + 5, rand() % 4 * 100 + 5);
	while (DP2F.x== CubeCollection[0]->CubePosition.x&&DP2F.y == CubeCollection[0]->CubePosition.y)
	{
		DP2F=D2D1::Point2F(rand() % 4 * 100 + 5, rand() % 4 * 100 + 5);
	}
	CubeCollection[1] = new GameCube(DP2F, 0, &CubeCollection[1]);
}

void GameRender()
{
	WCHAR TextTD[100] = { NULL };
	/**************************描画窗体背景和游戏底色***************************/
	ID2D1SolidColorBrush* D2D1Brush;
	D2D1HwndRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White),&D2D1Brush);
	D2D1HwndRenderTarget->BeginDraw();
	D2D1HwndRenderTarget->FillRectangle(D2D1::RectF(0,0,(float)D2D1HwndRenderTarget->GetPixelSize().width, (float)D2D1HwndRenderTarget->GetPixelSize().height), (ID2D1Brush*)D2D1Brush);
	D2D1Brush->Release();
	D2D1HwndRenderTarget->CreateSolidColorBrush(D2D1::ColorF(RGB(100,100,100)), &D2D1Brush);
	D2D1HwndRenderTarget->FillRectangle(D2D1::RectF(0, 0,405, 405), (ID2D1Brush*)D2D1Brush);
	//D2D1HwndRenderTarget->FillRectangle(D2D1::RectF(0,0,405,720), (ID2D1Brush*)D2D1Brush);
	for (int i = 0; i < 16; i++)
	{
		if (CubeCollection[i])
			CubeCollection[i]->FrameNotify();
	}
	/************************************************************************/
	/*                                      描画得分框                                                   */
	/************************************************************************/
	D2D1HwndRenderTarget->FillRectangle(D2D1::RectF(5,410,200,450),D2D1Brush);
	//sprintf(TextTDA, "分数:%d", Score);
	wsprintf(TextTD, TEXT("分数:%d"), Score);
	D2D1HwndRenderTarget->DrawTextW(TextTD,wcslen(TextTD),D2DTextFormat, D2D1::RectF(10, 410, 200, 450),BlackBrush);
	D2D1Brush->Release();
	D2D1HwndRenderTarget->DrawTextW(Hint,wcslen(Hint),D2DTextFormat, D2D1::RectF(205, 410, 405, 450), BlackBrush);
	D2D1HwndRenderTarget->EndDraw();
}

GameCube::GameCube(D2D1_POINT_2F _CubePosition, int _CubeLevel, GameCube** _GameCubeCollection)
{
	CubePosition = _CubePosition;
	CubeLevel = _CubeLevel;
	*_GameCubeCollection = this;
	GameCubeREF = _GameCubeCollection;
}

void GameCube::FrameNotify()
{
	GameCube* GameCubeTmp=NULL;
	switch (AnimStage)
	{
	case AnimStage_None:
		break;
	case AnimStage_Bigger:
		if (CubeSize < 95)
			CubeSize += 15;
		else
		{
			CubeSize = 95;
			AnimStage = AnimStage_None;
			//RepeatMoveFinished();
		}
		break;
	case AnimStage_MoveLeft: 
		GameCubeTmp = FindNearCube(this, AnimStage_MoveLeft);
		if (!GameCubeTmp) ///左边没有方块
		{
			if (CubePosition.x>5)
				this->CubePosition.x -= MOVESPEED;
			else
			{
				this->CubePosition.x = 5;
				AnimStage = AnimStage_None;
			}
		}
		else if (GameCubeTmp->CubeLevel == this->CubeLevel) //有方块，判断是否满足融合条件
		{
			this->CubePosition.x -= MOVESPEED;
			if (IsPointEqual(GameCubeTmp->CubePosition, CubePosition))//判断是否在同一位置
			{
				GameCubeTmp->Merge();
				*GameCubeREF = new GameCube(CubePosition, CubeLevel+1, GameCubeREF);
				delete this;
				return;
			}
		}
		else if (GameCubeTmp->AnimStage!=AnimStage_Bigger&& GameCubeTmp->AnimStage != AnimStage_None) //有方块，不满足融合条件，判断是否在移动
		{
			if (CubePosition.x > 5)
				this->CubePosition.x -= MOVESPEED;
			else
			{
				this->CubePosition.x = 5;
				AnimStage = AnimStage_None;
			}
		} else //有方块，且没有在移动
		{
			CubePosition.x = GameCubeTmp->CubePosition.x + 100;
			AnimStage = AnimStage_None;
		}
		break;
	case AnimStage_MoveRight:
		GameCubeTmp = FindNearCube(this, AnimStage_MoveRight);
		if (!GameCubeTmp) ///左边没有方块
		{
			if (CubePosition.x < 305)
				this->CubePosition.x += MOVESPEED;
			else
			{
				this->CubePosition.x = 305;
				AnimStage = AnimStage_None;
			}
		}
		else if (GameCubeTmp->CubeLevel == this->CubeLevel) //有方块，判断是否满足融合条件
		{
			this->CubePosition.x += MOVESPEED;
			if (IsPointEqual(GameCubeTmp->CubePosition, CubePosition))//判断是否在同一位置
			{
				GameCubeTmp->Merge();
				*GameCubeREF = new GameCube(CubePosition, CubeLevel + 1, GameCubeREF);
				delete this;
				return;
			}
		}
		else if (GameCubeTmp->AnimStage != AnimStage_Bigger&& GameCubeTmp->AnimStage != AnimStage_None) //有方块，不满足融合条件，判断是否在移动
		{
			if (CubePosition.x < 305)
				this->CubePosition.x += MOVESPEED;
			else
			{
				this->CubePosition.x = 305;
				AnimStage = AnimStage_None;
			}
		}
		else //有方块，且没有在移动
		{
			CubePosition.x = GameCubeTmp->CubePosition.x - 100;
			AnimStage = AnimStage_None;
		}
		break;
	case AnimStage_MoveUp:
		GameCubeTmp = FindNearCube(this, AnimStage_MoveUp);
		if (!GameCubeTmp) ///上边没有方块
		{
			if (CubePosition.y> 5)
				this->CubePosition.y -= MOVESPEED;
			else
			{
				this->CubePosition.y = 5;
				AnimStage = AnimStage_None;
			}
		}
		else if (GameCubeTmp->CubeLevel == this->CubeLevel) //有方块，判断是否满足融合条件
		{
			this->CubePosition.y -= MOVESPEED;
			if (IsPointEqual(GameCubeTmp->CubePosition, CubePosition))//判断是否在同一位置
			{
				GameCubeTmp->Merge();
				*GameCubeREF = new GameCube(CubePosition, CubeLevel + 1, GameCubeREF);
				delete this;
				return;
			}
		}
		else if (GameCubeTmp->AnimStage != AnimStage_Bigger&& GameCubeTmp->AnimStage != AnimStage_None) //有方块，不满足融合条件，判断是否在移动
		{
			if (CubePosition.y > 5)
				this->CubePosition.y -= MOVESPEED;
			else
			{
				this->CubePosition.y = 5;
				AnimStage = AnimStage_None;
			}
		}
		else //有方块，且没有在移动
		{
			CubePosition.y= GameCubeTmp->CubePosition.y + 100;
			AnimStage = AnimStage_None;
		}
		break;
	case AnimStage_MoveDown:
		GameCubeTmp = FindNearCube(this, AnimStage_MoveDown);
		if (!GameCubeTmp) ///左边没有方块
		{
			if (CubePosition.y < 305)
				this->CubePosition.y+= MOVESPEED;
			else
			{
				this->CubePosition.y = 305;
				AnimStage = AnimStage_None;
			}
		}
		else if (GameCubeTmp->CubeLevel == this->CubeLevel) //有方块，判断是否满足融合条件
		{
			this->CubePosition.y += MOVESPEED;
			if (IsPointEqual(GameCubeTmp->CubePosition, CubePosition))//判断是否在同一位置
			{
				GameCubeTmp->Merge();
				*GameCubeREF = new GameCube(CubePosition, CubeLevel + 1, GameCubeREF);
				delete this;
				return;
			}
		}
		else if (GameCubeTmp->AnimStage != AnimStage_Bigger&& GameCubeTmp->AnimStage != AnimStage_None) //有方块，不满足融合条件，判断是否在移动
		{
			if (CubePosition.y < 305)
				this->CubePosition.y += MOVESPEED;
			else
			{
				this->CubePosition.y = 305;
				AnimStage = AnimStage_None;
			}
		}
		else //有方块，且没有在移动
		{
			CubePosition.y = GameCubeTmp->CubePosition.y- 100;
			AnimStage = AnimStage_None;
		}
		break;
	default:
		break;
	}
	D2D1HwndRenderTarget->FillRectangle(D2D1::RectF(this->CubePosition.x + (95 - CubeSize) / 2, this->CubePosition.y + (95 - CubeSize) / 2, this->CubePosition.x + (95 - CubeSize) / 2 + CubeSize, this->CubePosition.y + (95 - CubeSize) / 2 + CubeSize),(ID2D1Brush*)LevelBrushCollection[CubeLevel]);
	WCHAR Level[5] = { NULL };
	char LevelA[5] = { NULL };
	itoa(pow(2, CubeLevel+1), LevelA, 10);
	mbstowcs(Level, LevelA, 5);
	D2D1HwndRenderTarget->DrawTextW(Level, 5, D2DTextFormat, D2D1::RectF(this->CubePosition.x + (95 - CubeSize) / 2, this->CubePosition.y + (95 - CubeSize) / 2, this->CubePosition.x + (95 - CubeSize) / 2 + CubeSize, this->CubePosition.y + (95 - CubeSize) / 2 + CubeSize), BlackBrush);
}

void GameCube::Move(ANIMSTAGE AnimS)
{
	GameCube* GameCubeTmp;
	AnimStage = AnimS;
	GameCubeTmp = FindCube(this, AnimS);
	switch (AnimS)
	{
	case AnimStage_MoveLeft:
		if (CubePosition.x < 100)
			break;
		if (GameCubeTmp = FindCube(this, AnimStage_MoveLeft))
			GameCubeTmp->Move(AnimS);
		break;
	case AnimStage_MoveRight:
		if (CubePosition.x > 350)
			break;
		if (GameCubeTmp = FindCube(this, AnimStage_MoveRight))
			GameCubeTmp->Move(AnimS);
		break;
	case AnimStage_MoveUp:
		if (CubePosition.y < 100)
			break;
		if (GameCubeTmp = FindCube(this, AnimStage_MoveUp))
			GameCubeTmp->Move(AnimS);
		break;
	case AnimStage_MoveDown:
		if (CubePosition.y > 350)
			break;
		if (GameCubeTmp = FindCube(this, AnimStage_MoveDown))
			GameCubeTmp->Move(AnimS);
		break;
	default:
		break;
	}
}

void GameCube::Merge()
{
	*GameCubeREF = NULL;
	Score += pow(2, CubeLevel + 1);
	delete this;
}

GameCube* FindCube(const GameCube* GameC,ANIMSTAGE DirectionToFind)
{
	GameCube* CubeToReturn=NULL;
	for (GameCube* GameCub:CubeCollection)
	{
		if (!GameCub||GameCub==GameC)
			continue;
		switch (DirectionToFind)
		{
		case AnimStage_MoveLeft:
			if (GameCub->CubePosition.y == GameC->CubePosition.y&&GameCub->CubePosition.x<=GameC->CubePosition.x&&(!CubeToReturn||GameCub->CubePosition.x>CubeToReturn->CubePosition.x))
				CubeToReturn = GameCub;
			break;
		case AnimStage_MoveRight:
			if (GameCub->CubePosition.y == GameC->CubePosition.y&& GameCub->CubePosition.x>=GameC->CubePosition.x&&(!CubeToReturn||GameCub->CubePosition.x < CubeToReturn->CubePosition.x))
				CubeToReturn = GameCub;
			break;
		case AnimStage_MoveUp:
			if (GameCub->CubePosition.x == GameC->CubePosition.x && GameCub->CubePosition.y<=GameC->CubePosition.y&&(!CubeToReturn || GameCub->CubePosition.y > CubeToReturn->CubePosition.y))
				CubeToReturn = GameCub;
			break;
		case AnimStage_MoveDown:
			if (GameCub->CubePosition.x == GameC->CubePosition.x && GameCub->CubePosition.y>=GameC->CubePosition.y && (!CubeToReturn || GameCub->CubePosition.y < CubeToReturn->CubePosition.y))
				CubeToReturn = GameCub;
			break;
		default:
			break;
		}
	}
	return CubeToReturn;
}

GameCube* FindNearCube(const GameCube* GameC, ANIMSTAGE DirectionToFind)
{
	GameCube* CubeToReturn = NULL;
	for (GameCube* GameCub : CubeCollection)
	{
		if (!GameCub || GameCub == GameC)
			continue;
		switch (DirectionToFind)
		{
		case AnimStage_MoveLeft:
			if (GameCub->CubePosition.y == GameC->CubePosition.y&&GameCub->CubePosition.x <= GameC->CubePosition.x && (!CubeToReturn || GameCub->CubePosition.x > CubeToReturn->CubePosition.x))
				CubeToReturn = GameCub;
			break;
		case AnimStage_MoveRight:
			if (GameCub->CubePosition.y == GameC->CubePosition.y&& GameCub->CubePosition.x >= GameC->CubePosition.x && (!CubeToReturn || GameCub->CubePosition.x < CubeToReturn->CubePosition.x))
				CubeToReturn = GameCub;
			break;
		case AnimStage_MoveUp:
			if (GameCub->CubePosition.x == GameC->CubePosition.x && GameCub->CubePosition.y <= GameC->CubePosition.y && (!CubeToReturn || GameCub->CubePosition.y > CubeToReturn->CubePosition.y))
				CubeToReturn = GameCub;
			break;
		case AnimStage_MoveDown:
			if (GameCub->CubePosition.x == GameC->CubePosition.x && GameCub->CubePosition.y >= GameC->CubePosition.y && (!CubeToReturn || GameCub->CubePosition.y < CubeToReturn->CubePosition.y))
				CubeToReturn = GameCub;
			break;
		default:
			break;
		}
	}
	if (CubeToReturn)
		if (abs((int)(CubeToReturn->CubePosition.x - GameC->CubePosition.x)) > 120 || abs((int)(CubeToReturn->CubePosition.y - GameC->CubePosition.y)) > 120)
			CubeToReturn = NULL;
	return CubeToReturn;
}

GameCube* FindCubeOnEdge(ANIMSTAGE DirectionToFind,float CubeAxis)
{
	GameCube* CubeToReturn = NULL;
	for (GameCube* GameCub : CubeCollection)
	{
		if (!GameCub)
			continue;
		switch (DirectionToFind)
		{
		case AnimStage_MoveLeft:
			if (GameCub->CubePosition.y == CubeAxis&&(!CubeToReturn || (GameCub->CubePosition.x<CubeToReturn->CubePosition.x)))
				CubeToReturn = GameCub;
			break;
		case AnimStage_MoveRight:
			if (GameCub->CubePosition.y == CubeAxis && (!CubeToReturn || (GameCub->CubePosition.x > CubeToReturn->CubePosition.x)))
				CubeToReturn = GameCub;
			break;
		case AnimStage_MoveUp:
			if (GameCub->CubePosition.x == CubeAxis && (!CubeToReturn || (GameCub->CubePosition.y < CubeToReturn->CubePosition.y)))
				CubeToReturn = GameCub;
			break;
		case AnimStage_MoveDown:
			if (GameCub->CubePosition.x == CubeAxis && (!CubeToReturn || (GameCub->CubePosition.y > CubeToReturn->CubePosition.y)))
				CubeToReturn = GameCub;
			break;
		default:
			break;
		}
	}
	return CubeToReturn;
}

bool IsPositionHaveCube(D2D1_POINT_2F Position)
{
	for (GameCube* GCube:CubeCollection)
	{
		if (GCube)
			if (GCube->CubePosition.x == Position.x&&GCube->CubePosition.y == Position.y)
				return true;
	}
	return false;
}

int GetCubeCount()
{
	int CubeCount = 0;
	for (GameCube* GCube : CubeCollection)
		if (GCube)
			CubeCount++;
	return CubeCount;
}

void SpawnCube()
{
	GameCube** GCubeSlot=NULL;
	for(int i=0;i<16;i++)
		if (!CubeCollection[i])
		{
			GCubeSlot = &CubeCollection[i];
			break;
		}
	D2D1_POINT_2F DP2F = D2D1::Point2F(rand() % 4 * 100 + 5, rand() % 4 * 100 + 5);
	while (IsPositionHaveCube(DP2F))
	{
		DP2F = D2D1::Point2F(rand() % 4 * 100 + 5, rand() % 4 * 100 + 5);
	}
	*GCubeSlot = new GameCube(DP2F, 0, GCubeSlot);
}

inline bool IsPointEqual(D2D1_POINT_2F A, D2D1_POINT_2F B)
{
	return A.x == B.x&&A.y == B.y;
}

bool IsAnyCubeMoving()
{
	for (GameCube* GCube:CubeCollection)
	{
		if (GCube)
			if (GCube->AnimStage != AnimStage_None)
				return true;
	}
	return false;
}

bool IsMovableOneLine(ANIMSTAGE AS,int MoveAxis)
{
	GameCube* CubeTmp = NULL;
	switch (AS)
	{
	case AnimStage_MoveLeft:
		CubeTmp = FindCubeOnEdge(AnimStage_MoveRight, MoveAxis);
		while (CubeTmp)
		{
			if (!FindNearCube(CubeTmp, AnimStage_MoveLeft) && CubeTmp->CubePosition.x > 10)
				return true;
			else if ((CubeTmp = FindNearCube(CubeTmp, AnimStage_MoveLeft)) && CubeTmp->CubeLevel == FindNearCube(CubeTmp, AnimStage_MoveRight)->CubeLevel)
				return true;
		}
		return false;
		break;
	case AnimStage_MoveRight:
		CubeTmp = FindCubeOnEdge(AnimStage_MoveLeft, MoveAxis);
		while (CubeTmp)
		{
			if (!FindNearCube(CubeTmp, AnimStage_MoveRight) && CubeTmp->CubePosition.x < 290)
				return true;
			else if ((CubeTmp = FindNearCube(CubeTmp, AnimStage_MoveRight)) && CubeTmp->CubeLevel == FindNearCube(CubeTmp, AnimStage_MoveLeft)->CubeLevel)
				return true;
		}
		return false;
		break;
	case AnimStage_MoveUp:
		CubeTmp = FindCubeOnEdge(AnimStage_MoveDown, MoveAxis);
		while (CubeTmp)
		{
			if (!FindNearCube(CubeTmp, AnimStage_MoveUp) && CubeTmp->CubePosition.y > 10)
				return true;
			else if ((CubeTmp = FindNearCube(CubeTmp, AnimStage_MoveUp)) && CubeTmp->CubeLevel == FindNearCube(CubeTmp, AnimStage_MoveDown)->CubeLevel)
				return true;
		}
		return false;
		break;
	case AnimStage_MoveDown:
		CubeTmp = FindCubeOnEdge(AnimStage_MoveUp, MoveAxis);
		while (CubeTmp)
		{
			if (!FindNearCube(CubeTmp, AnimStage_MoveDown) && CubeTmp->CubePosition.y < 290)
				return true;
			else if ((CubeTmp = FindNearCube(CubeTmp, AnimStage_MoveDown)) && CubeTmp->CubeLevel == FindNearCube(CubeTmp, AnimStage_MoveUp)->CubeLevel)
				return true;
		}
		return false;
		break;
	default:
		break;
	}
}

bool IsMovableLocal(ANIMSTAGE AS)
{
	for (int i=5;i<400;i+=100)
	{
		if (IsMovableOneLine(AS,i))
			return true;
	}
	return false;
}

inline bool IsMovableGlobal()
{
	if (IsMovableLocal(AnimStage_MoveLeft))
		return true;
	else if (IsMovableLocal(AnimStage_MoveRight))
		return true;
	else if (IsMovableLocal(AnimStage_MoveUp))
		return true;
	else if (IsMovableLocal(AnimStage_MoveDown))
		return true;
	else
		return false;
}

inline bool Have2048()
{
	for (GameCube* GCube:CubeCollection)
	{
		if (GCube&&GCube->CubeLevel >= 10)
			return true;
	}
	return false;
}