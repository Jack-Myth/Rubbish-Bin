#pragma once
#include <d2d1.h>
#include <vector>

#define CARD_WIDTH 90
#define CARD_HEIGHT 145
#define CARDSTACK_X 90
#define CARDSTACK_Y 100
#define CARDSTACK_SPACE 105
#define TARGETSLOT_X 405
#define TARGETSLOT_Y 100
#define TARGETSLOT_SPACE 105
#define DESKSLOT_X 90
#define DESKSLOT_Y 300
#define DESKSLOT_SPACE 105


namespace CardSlots
{
	extern std::vector<char> CardStack;
	extern char ActivingCardNumber;
	extern char TargetSlots[4];
	extern char DeskSlots[7];
};

enum class CardSuit
{
	Spade=0, //黑桃
	Heart=1,  //红桃
	Diamond=2, //方片
	Club=3 //梅花
};

class CardAnimator
{
	 class Card* TargetCard;
	D2D1_RECT_F  From;
	D2D1_RECT_F To;
	D2D1_POINT_2F PositionTarget;
	//0-1
	float Value=0;
	float RequestTime;
public:
	CardAnimator(class Card* TargetCard, D2D1_RECT_F  From, D2D1_POINT_2F PositionTarget, float RequestTime);

	//如果Animator已播放至尾部，返回true
	//返回true意味着此Animator已可销毁
	bool UpdateAnimationAndRender();
	void ForceEnd();
};

class Card
{
	//S->"黑桃(Spade)"
	//H->"红桃(Heart)"
	//D->"方片(Diamond)"
	//C->"梅花(Club)"
	CardSuit Suit;

	//数字，A-K:1-13
	char Number=0;
	ID2D1Bitmap* FrontImage=NULL;

	//-1—-4黑桃，红桃，方片，梅花的目标位置
	//0:牌堆
	//1-6:桌面从左到右的位置
	char Place=0;

	//牌面是否可见
	bool Showing=false;

	//子牌(堆叠在这张牌之上的牌)
	//因为只有56张牌，所以应当不需要设置上层牌的指针，取上层牌遍历即可
	Card* ChildCard=NULL;

	//上一次点击的时间，用于分析双击事件
	float LastClickTime=0;

	//牌当前位置
	D2D1_POINT_2F Position;
	
	//牌的上一个正确位置
	D2D1_POINT_2F LastPosition;

	//Z序
	char ZOrder;
	
	friend CardAnimator;
	//卡牌动画，每张牌只能附加一个动画实例
	CardAnimator* CardAnimation=NULL;
	void DoubleClick();//双击逻辑
public:
	//S->"黑桃(Spade)"
	//H->"红桃(Heart)"
	//D->"方片(Diamond)"
	//C->"梅花(Club)"
	Card(CardSuit Suit, char Number, WCHAR* ImagePath,char Place, char ZOrder);
	
	void Render();
	void Move(D2D1_POINT_2F Target);
	
	bool CanDrag();
	void TryShow();

	//主要作用是返回原有位置
	//其次是分析双击事件
	void Release();

	bool ApplyChildTo(Card* CardToApply);
	void ApplyAnimator(D2D1_POINT_2F Target);
	void RecordPosition();
	//Set
	//////////////////////////////////////////////////////////////////////////
	void SetZOrder(char NewZOrder);
	void SetShowing(bool NewShowing);
	void SetChild(Card* NewChildCard);
	void SetPlace(char NewPlace);
	void SetPosition(D2D1_POINT_2F NewPosition);
	//////////////////////////////////////////////////////////////////////////

	//Get
	//////////////////////////////////////////////////////////////////////////
	char GetZOrder();
	D2D1_POINT_2F GetPosition();
	char GetNumber();
	CardSuit GetSuit();
	bool IsShowing();
	Card* GetChildCard();
	char GetPlace();
	//////////////////////////////////////////////////////////////////////////
};

class PlayerController
{
	//持有的牌
	Card* HoldingCard;
	//（上一次记录时）鼠标指针的位置
	D2D1_POINT_2F Position;
public:
	void UpdatePosition(D2D1_POINT_2F CurrentPosition);
	void MouseDown();
	void MouseUp();
	D2D1_POINT_2F GetPosition();
	Card* GetHoldingCard();
};