#pragma once
#include <windows.h>
#include <d2d1.h>

class GameUtility
{
	static float TimeValue;
public:
	static void UpdateTime();
	static float GetGameTime();
	static char GetUninitedCard(CardSuit& Suit);
	static char GetUninitedCardRandom(CardSuit& Suit);
	static char CardSuitToChar(CardSuit Suit);
	static bool LoadD2DImage(WCHAR ImagePath[], ID2D1Bitmap ** BitMapMayClear);
	static bool IsPointInRect(D2D1_POINT_2F Point, D2D1_RECT_F Rect);
	static char GetParentCard(Card* ChildCard);
	//-12~-9:四个目标位置
	//-8:牌堆底
	//-7~-1:桌面上从左至右7个位置
	//0:鼠标没有在任何东西上（桌面空白处或不在窗口内）
	//1-52:牌的REF索引
	static char GetObjectUnderCursor();
};