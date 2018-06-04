#include <wincodec.h>
#include "Solitaire.h"
#include "Utility.h"
#include <vector>
#include <algorithm>

using std::vector;

extern IWICImagingFactory* WICImagingFactory;
extern ID2D1HwndRenderTarget* D2D1HwndRenderTarget;
extern Card* CardRef[52];
extern PlayerController PlayerControllerInstance;
float GameUtility::TimeValue = 0;
void GameUtility::UpdateTime()
{
	TimeValue += 0.02f;
}

float GameUtility::GetGameTime()
{
	return TimeValue;
}

char GameUtility::GetUninitedCard(CardSuit& Suit)
{
	for (int i = 0; i < 52; i++)
		if (!CardRef[i])
			switch ((int)i / 13)
			{
			case 0:
				Suit = CardSuit::Spade;
				return i;
			case 1:
				Suit = CardSuit::Heart;
				return i;
			case 2:
				Suit = CardSuit::Diamond;
				return i;
			case 3:
				Suit = CardSuit::Club;
				return i;
			}
	return 0;
}

//返回当前未初始化的牌，Suit为花色，i为REF的索引
char GameUtility::GetUninitedCardRandom(CardSuit & Suit)
{
	int i = 0;
	for (i = 0; i < 52 && CardRef[i]; i++);
	if (i >= 52)
		return -1;
	while (i = rand() % 52, CardRef[i]);
	switch ((int)i / 13)
	{
	case 0:
		Suit = CardSuit::Spade;
		break;
	case 1:
		Suit =CardSuit::Heart;
		break;
	case 2:
		Suit = CardSuit::Diamond;
		break;
	case 3:
		Suit = CardSuit::Club;
		break;
	}
	return i;
}

char GameUtility::CardSuitToChar(CardSuit Suit)
{
	switch (Suit)
	{
	case CardSuit::Heart:
		return 'H';
	case CardSuit::Diamond:
		return 'D';
	case CardSuit::Spade:
		return 'S';
	case CardSuit::Club:
		return 'C';
	}
	return 0;
}

bool GameUtility::LoadD2DImage(WCHAR ImagePath[], ID2D1Bitmap** BitMapMayClear)
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

bool GameUtility::IsPointInRect(D2D1_POINT_2F Point, D2D1_RECT_F Rect)
{
	return (Point.x > Rect.left &&Point.x<Rect.right &&Point.y>Rect.top &&Point.y < Rect.bottom) ? true : false;
}

char GameUtility::GetParentCard(Card * ChildCard)
{
	for (int i = 0; i < 52; i++)
		if (CardRef[i]->GetChildCard() == ChildCard)
			return i;
	return -1;
}

char GameUtility::GetObjectUnderCursor()
{
	auto CursorPosition = PlayerControllerInstance.GetPosition();
	vector<char> InRangeCard;
	for (int i=0;i<52;i++) //检查是不是指向某张牌
	{
		if (CursorPosition.x>CardRef[i]->GetPosition().x - CARD_WIDTH/2.f &&
			CursorPosition.x<CardRef[i]->GetPosition().x + CARD_WIDTH / 2.f && 
			CursorPosition.y>CardRef[i]->GetPosition().y - CARD_HEIGHT / 2.f && 
			CursorPosition.y<CardRef[i]->GetPosition().y + CARD_HEIGHT / 2.f)
			InRangeCard.push_back(i);
	}
	if (InRangeCard.size()>0) //对牌进行排序，取Z序最大的一张
	{
		std::sort(InRangeCard.begin(), InRangeCard.end(), [](const char &a, const char &b)->bool {return CardRef[a]->GetZOrder() > CardRef[b]->GetZOrder(); });
		return InRangeCard[0]+1;
	}
	else //看指针是不是指向某个位置的底部 
	{
		for (int i=0;i<4;i++)
			if (IsPointInRect(CursorPosition, D2D1::RectF(TARGETSLOT_X +i*TARGETSLOT_SPACE - CARD_WIDTH / 2.f, TARGETSLOT_Y - CARD_HEIGHT / 2.f, TARGETSLOT_X + i*TARGETSLOT_SPACE + CARD_WIDTH / 2.f, (float)TARGETSLOT_Y + CARD_HEIGHT)))
				return i-12;
		if (IsPointInRect(CursorPosition, D2D1::RectF(CARDSTACK_X - CARD_WIDTH / 2.f, CARDSTACK_Y - CARD_HEIGHT / 2.f, CARDSTACK_X + CARD_WIDTH / 2.f, (float)CARDSTACK_Y + CARD_HEIGHT)))
			return -8;
		for (int i = 0; i <7; i++)
			if (IsPointInRect(CursorPosition, D2D1::RectF(DESKSLOT_X + i*DESKSLOT_SPACE - CARD_WIDTH / 2.f, DESKSLOT_Y - CARD_HEIGHT / 2.f, DESKSLOT_X + i*DESKSLOT_SPACE + CARD_WIDTH / 2.f, (float)DESKSLOT_Y + CARD_HEIGHT)))
				return i-7;
	}
	return 0;
}
