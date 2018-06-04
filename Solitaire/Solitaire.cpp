#include <stdio.h>
#include <windows.h>
#include <d2d1.h>
#include <wincodec.h>
#include "Solitaire.h"
#include "Utility.h"
#include <vector>
#include <algorithm>
#include <time.h>
#pragma comment(lib, "d2d1.lib")

using std::vector;

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void INITGameEngine();
void InitGame();
void ProcessGameLogic();
void ProcessRender();

HWND MainHwnd;
IWICImagingFactory* WICImagingFactory;
ID2D1Factory* D2D1Factory;
ID2D1HwndRenderTarget* D2D1HwndRenderTarget;
PlayerController PlayerControllerInstance;
Card* CardRef[52];
vector<char> CardRefCopy(52);
ID2D1Bitmap* DeskBackground, *CardBackground;
vector<char> CardSlots::CardStack;
char CardSlots::ActivingCardNumber;
char CardSlots::TargetSlots[4];
char CardSlots::DeskSlots[7];
RECT Windowrc = { 0,0,800,800 };
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	WNDCLASSEX wnd = { NULL };
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wnd.hCursor = LoadCursor(0, IDC_ARROW);
	wnd.hIcon = LoadIcon(0, IDC_ICON);
	wnd.hInstance = hInstance;
	wnd.lpfnWndProc = MainWndProc;
	wnd.lpszClassName = TEXT("My Solitaire");
	RegisterClassEx(&wnd);
	AdjustWindowRect(&Windowrc, WS_OVERLAPPEDWINDOW, false);
	MainHwnd = CreateWindow(wnd.lpszClassName, TEXT("My Solitaire"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, Windowrc.right - Windowrc.left, Windowrc.bottom - Windowrc.top, NULL, NULL, hInstance, NULL);
	INITGameEngine();
	SetTimer(MainHwnd, 0, 20, NULL);
	ShowWindow(MainHwnd, nShowCmd);
	InitGame();
	MSG msg = { NULL };
	while (msg.message != WM_QUIT)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		GetMessage(&msg, 0, 0, 0);
	}
	return 0;
}

void INITGameEngine()
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

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_TIMER:
		ProcessGameLogic();
		ProcessRender();
		break;
	case  WM_MOUSEMOVE:
		PlayerControllerInstance.UpdatePosition(D2D1::Point2F(LOWORD(lParam), HIWORD(lParam)));
		break;
	case WM_LBUTTONDOWN:
		PlayerControllerInstance.MouseDown();
		break;
	case WM_LBUTTONUP:
		PlayerControllerInstance.MouseUp();
		break;
	case  WM_DESTROY:
		PostQuitMessage(0);
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

void InitGame()
{
	WCHAR ImagePath[255] = { 0 };
	CardSuit Suit;
	char Number;
	char LastIndex;
	for (int i=0;i<24;i++) //分配牌堆里的牌
	{
		Number= GameUtility::GetUninitedCardRandom(Suit);
		wsprintf(ImagePath, TEXT("Image\\%c%d.png"), GameUtility::CardSuitToChar(Suit), (int)(Number % 13 + 1));
		CardRef[Number] = new Card(Suit, Number % 13 + 1,ImagePath,0,i);
		CardRef[Number]->SetPosition(D2D1::Point2F(CARDSTACK_X, CARDSTACK_Y));
		CardRef[Number]->RecordPosition();
		CardSlots::CardStack.push_back(Number);
	}
	for(int i=1;i<=7;i++) //分配桌面上的牌
		for (int k = 0; k < i; k++)
		{
			Number = GameUtility::GetUninitedCardRandom(Suit);
			wsprintf(ImagePath, TEXT("Image\\%c%d.png"), GameUtility::CardSuitToChar(Suit), (int)(Number % 13 + 1));
			CardRef[Number] = new Card(Suit, Number % 13 + 1, ImagePath, i, k);
			CardRef[Number]->SetPosition(D2D1::Point2F(DESKSLOT_X + DESKSLOT_SPACE*(i-1), DESKSLOT_Y));
			CardRef[Number]->RecordPosition();
			if (k > 0)
				CardRef[LastIndex]->SetChild(CardRef[Number]);
			if (k == i - 1)
				CardRef[Number]->TryShow();
			LastIndex = Number;
		}
	for (int i = 0; i < 52; i++)
		CardRefCopy[i] = i;
	//读取桌面的图片
	GameUtility::LoadD2DImage(TEXT("Image\\DeskBG.png"), &DeskBackground);
	//读取卡牌背景 
	GameUtility::LoadD2DImage(TEXT("Image\\CardBG.png"), &CardBackground);
}

void ProcessGameLogic()
{
	GameUtility::UpdateTime();
}

void ProcessRender()
{
	D2D1HwndRenderTarget->BeginDraw();
	D2D1HwndRenderTarget->DrawBitmap(DeskBackground, D2D1::RectF(0, 0, Windowrc.right-Windowrc.left,Windowrc.bottom-Windowrc.top), 1, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, 
		D2D1::RectF(0.f,0.f,(float)DeskBackground->GetPixelSize().width,(float)DeskBackground->GetPixelSize().height));
	std::sort(CardRefCopy.begin(), CardRefCopy.end(), [](const char& a,const char& b) {return CardRef[a]->GetZOrder() < CardRef[b]->GetZOrder(); });//排序
	//按Z序渲染
	for (int i=0;i<52;i++)
		CardRef[CardRefCopy[i]]->Render();
	if (PlayerControllerInstance.GetHoldingCard())//渲染当前持有的牌，使当前持有的牌始终在最前
		for (Card* HoldingCard = PlayerControllerInstance.GetHoldingCard(); HoldingCard; HoldingCard = HoldingCard->GetChildCard())
			HoldingCard->Render();
	D2D1HwndRenderTarget->EndDraw();
}

void Card::DoubleClick()
{
}

Card::Card(CardSuit Suit, char Number, WCHAR * ImagePath, char Place, char ZOrder)
{
	Card::Suit = Suit;
	Card::Number = Number;
	GameUtility::LoadD2DImage(ImagePath, &(Card::FrontImage));
	Card::Place = Place;
	Card::ZOrder = ZOrder;
}

void Card::Render()
{
	if (CardAnimation)
	{
		if (CardAnimation->UpdateAnimationAndRender())
		{
			delete CardAnimation;
			CardAnimation = NULL;
		}
	}
	else
	D2D1HwndRenderTarget->DrawBitmap(Showing ? FrontImage : CardBackground,
		D2D1::RectF(Position.x - CARD_WIDTH / 2, Position.y - CARD_HEIGHT / 2, Position.x + CARD_WIDTH / 2, Position.y + CARD_HEIGHT/2),
		1, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
		D2D1::RectF(0.f, 0.f, (float)(Showing ? FrontImage : CardBackground)->GetPixelSize().width, (float)(Showing ? FrontImage : CardBackground)->GetPixelSize().height));
}

void Card::Move(D2D1_POINT_2F Target)
{
	Position.x += Target.x;
	Position.y += Target.y;
	if (ChildCard)
		ChildCard->Move(Target);
}

void Card::SetZOrder(char NewZOrder)
{
	ZOrder = NewZOrder;
	if (ChildCard)
		ChildCard->SetZOrder(ZOrder + 1);
}

void Card::SetShowing(bool NewShowing)
{
	if (Place)
		return;
	Showing = NewShowing;
}

void Card::SetChild(Card * NewChildCard)
{
	ChildCard = NewChildCard;
	if (!ChildCard)
		return;

	ChildCard->Move(D2D1::Point2F(Position.x- ChildCard->GetPosition().x, Position.y- ChildCard->GetPosition().y + CARD_HEIGHT / 4));
	ChildCard->RecordPosition();
	ChildCard->SetZOrder(ZOrder + 1);
	ChildCard->Place = Place;
}

void Card::SetPlace(char NewPlace)
{
	Place = NewPlace;
}

void Card::SetPosition(D2D1_POINT_2F NewPosition)
{
	Position = NewPosition;
}

char Card::GetZOrder()
{
	return ZOrder;
}

D2D1_POINT_2F Card::GetPosition()
{
	return Position;
}

char Card::GetNumber()
{
	return Number;
}

CardSuit Card::GetSuit()
{
	return Suit;
}

bool Card::IsShowing()
{
	return Showing;
}

Card * Card::GetChildCard()
{
	return ChildCard;
}

char Card::GetPlace()
{
	return Place;
}

bool Card::CanDrag()
{
	if (!Showing)
		return false;
	if (!ChildCard)
		return true;
	switch (Suit)
	{
	case CardSuit::Spade:
	case CardSuit::Club:
		if (Number - ChildCard->GetNumber() == 1 && (ChildCard->GetSuit() == CardSuit::Heart || ChildCard->GetSuit() == CardSuit::Diamond))
			return true;
		break;
	case CardSuit::Heart:
	case CardSuit::Diamond:
		if (Number - ChildCard->GetNumber() == 1 && (ChildCard->GetSuit() == CardSuit::Club || ChildCard->GetSuit() == CardSuit::Spade))
			return true;
	default:
		break;
	}
	return false;
}

void Card::TryShow()
{
	if (Place>0&&ChildCard==NULL)
		Showing = true;
	else if (Place == 0&&!Showing) //在牌堆被展开的情况
	{
		ApplyAnimator(D2D1::Point2F(CARDSTACK_X+CARDSTACK_SPACE, CARDSTACK_Y));
		Showing = true;
		SetZOrder((CardSlots::ActivingCardNumber++)-27);
	}
}

void Card::Release()
{
	if (GameUtility::GetGameTime() - LastClickTime < 1)
		DoubleClick();
	LastClickTime = GameUtility::GetGameTime();
	Move(D2D1::Point2F(LastPosition.x - Position.x, LastPosition.y - Position.y));
}

bool Card::ApplyChildTo(Card * CardToApply)
{
	if (CardToApply->GetChildCard())
		return false;
	if (!CardToApply->IsShowing())
		return false;
	if (!CardToApply->GetPlace())
		return false;
	if (CardToApply->GetPlace()<0)
	{
		if (Number-CardToApply->GetNumber() == 1 && CardToApply->GetSuit()==Suit&&!ChildCard)
		{
			Position = CardToApply->GetPosition();
			RecordPosition();
			Place = CardToApply->GetPlace();
			SetZOrder(CardToApply->GetZOrder() + 1);
			if (char ParentCard = GameUtility::GetParentCard(this); ParentCard >= 0)
				CardRef[ParentCard]->SetChild(NULL);
			return true;
		}
		return false;
	}
	switch (Suit)
	{
	case CardSuit::Spade:
	case CardSuit::Club:
		if (!(CardToApply->GetNumber() - Number == 1 && (CardToApply->GetSuit() == CardSuit::Heart || CardToApply->GetSuit() == CardSuit::Diamond)))
			return false;
		break;
	case CardSuit::Heart:
	case CardSuit::Diamond:
		if (!(CardToApply->GetNumber() - Number == 1 && (CardToApply->GetSuit() == CardSuit::Club || CardToApply->GetSuit() == CardSuit::Spade)))
			return false;
		break;
	}
	if (char ParentCard = GameUtility::GetParentCard(this); ParentCard >= 0)
		CardRef[ParentCard]->SetChild(NULL);
	CardToApply->SetChild(this);
	return true;
}

void Card::ApplyAnimator(D2D1_POINT_2F Target)
{
	if (CardAnimation)
	{
		CardAnimation->ForceEnd(); //通知动画强制结束
		delete CardAnimation;
	}
	CardAnimation = new CardAnimator(this,D2D1::RectF(Position.x-CARD_WIDTH/2,Position.y-CARD_HEIGHT/2.f,Position.x+CARD_WIDTH/2,Position.y+CARD_HEIGHT/2.f),Target,0.2);
}

void Card::RecordPosition()
{
	LastPosition = Position;
	if (ChildCard)
		ChildCard->RecordPosition();
}

CardAnimator::CardAnimator(Card * TargetCard, D2D1_RECT_F From, D2D1_POINT_2F PositionTarget, float RequestTime = 0.2)
{
	this->From = From;
	this->TargetCard = TargetCard;
	this->PositionTarget = PositionTarget;
	this->RequestTime = RequestTime;
	this->To = D2D1::RectF(PositionTarget.x - CARD_WIDTH / 2, PositionTarget.y - CARD_HEIGHT / 2, PositionTarget.x + CARD_WIDTH / 2, PositionTarget.y + CARD_HEIGHT/2);
}

bool CardAnimator::UpdateAnimationAndRender()
{
	Value+=1.f/(RequestTime/0.02f);
	D2D1_RECT_F DrawTargetRect = { 0 };
	DrawTargetRect.left = (To.left - From.left)*Value + From.left;
	DrawTargetRect.top = (To.top - From.top)*Value + From.top;
	DrawTargetRect.right = (To.right - From.right)*Value + From.right;
	DrawTargetRect.bottom = (To.bottom - From.bottom)*Value + From.bottom;
	D2D1HwndRenderTarget->DrawBitmap((TargetCard->Showing) ? (TargetCard->FrontImage) : CardBackground,
		DrawTargetRect,
		1, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
		D2D1::RectF(0.f, 0.f, (float)((TargetCard->Showing) ? (TargetCard-> FrontImage) : CardBackground)->GetPixelSize().width, (float)((TargetCard->Showing) ? (TargetCard->FrontImage) : CardBackground)->GetPixelSize().height));
	if (Value >= 1)
	{
		TargetCard->SetPosition(PositionTarget);
		TargetCard->RecordPosition();
		return true;
	}
	return false;
}

void CardAnimator::ForceEnd()
{
	TargetCard->SetPosition(PositionTarget);
	TargetCard->RecordPosition();
}

void PlayerController::UpdatePosition(D2D1_POINT_2F CurrentPosition)
{
	if (HoldingCard)
		HoldingCard->Move(D2D1::Point2F(CurrentPosition.x-Position.x,CurrentPosition.y-Position.y));
	Position = CurrentPosition;
}

void PlayerController::MouseDown()
{
	char ObjectIndex = GameUtility::GetObjectUnderCursor();
#ifdef _DEBUG
	char DebugMessage[1024];
	OutputDebugStringA("MouseDown\n");
	sprintf(DebugMessage, "MousePosition:X-%.0f,Y-%.0f\nObjectIndex:%d\n", Position.x, Position.y,(int)ObjectIndex);
	OutputDebugStringA(DebugMessage);
#endif
	if (!ObjectIndex)
		return;
	if (ObjectIndex==-8) //鼠标指着牌堆底
	{
		for (int i=0;i<52;i++)
		{
			if (!CardRef[i]->GetPlace())
			{
				CardRef[i]->SetZOrder(CardRef[i]->GetZOrder()*-1);
				CardRef[i]->SetZOrder((char)(CardRef[i]->GetZOrder() + CardSlots::CardStack.size() + 1)); //倒转Z序
				CardRef[i]->ApplyAnimator(D2D1::Point2F(CARDSTACK_X, CARDSTACK_Y));
				CardRef[i]->SetShowing(false);
				CardSlots::ActivingCardNumber = 0;
			}
		}
	}
	else if(ObjectIndex>0) //鼠标在牌上
	{
		if (!CardRef[ObjectIndex-1]->IsShowing()&&!CardRef[ObjectIndex-1]->GetChildCard())
			CardRef[ObjectIndex-1]->TryShow();
		else if (CardRef[ObjectIndex-1]->CanDrag())
			HoldingCard = CardRef[ObjectIndex-1];
	}
}

void PlayerController::MouseUp()
{
#ifdef _DEBUG
	char DebugMessage[1024];
	OutputDebugStringA("MouseUp\n");
	sprintf(DebugMessage, "MousePosition:X-%.0f,Y-%.0f", Position.x, Position.y);
	OutputDebugStringA(DebugMessage);
#endif
	if (HoldingCard)
	{
		HoldingCard->Move(D2D1::Point2F(-300, -300));
		char ObjectIndex = GameUtility::GetObjectUnderCursor();
		HoldingCard->Move(D2D1::Point2F(300, 300));
		if (ObjectIndex > 0)
		{
			if (!HoldingCard->ApplyChildTo(CardRef[ObjectIndex - 1]))
				HoldingCard->Release();
			HoldingCard = NULL;
			return;
		}
		else if (ObjectIndex<=-9)
		{
			if (HoldingCard->GetNumber()==1&&HoldingCard->GetSuit()==(CardSuit)(ObjectIndex+12)&&!HoldingCard->GetChildCard())
			{
				HoldingCard->SetPosition(D2D1::Point2F(TARGETSLOT_X + TARGETSLOT_SPACE*(ObjectIndex + 12), TARGETSLOT_Y));
				if (char ParentCard = GameUtility::GetParentCard(HoldingCard);ParentCard>=0)
					CardRef[ParentCard]->SetChild(NULL);
				HoldingCard->SetPlace(-(ObjectIndex + 13));
				HoldingCard->RecordPosition();
			}
		}
		else if (ObjectIndex < 0)
		{
			if (HoldingCard->GetNumber() == 13)
			{
				HoldingCard->Move(D2D1::Point2F(DESKSLOT_X + DESKSLOT_SPACE*(ObjectIndex + 7)-HoldingCard->GetPosition().x, DESKSLOT_Y- HoldingCard->GetPosition().y));
				if (char ParentCard = GameUtility::GetParentCard(HoldingCard); ParentCard >= 0)
					CardRef[ParentCard]->SetChild(NULL);
				HoldingCard->SetPlace(ObjectIndex+8);
				HoldingCard->RecordPosition();
			}
		}
		HoldingCard->Release();
		HoldingCard = NULL;
	}
}

D2D1_POINT_2F PlayerController::GetPosition()
{
	return Position;
}

Card * PlayerController::GetHoldingCard()
{
	return HoldingCard;
}
