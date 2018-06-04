#define  _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <thread>
#include <string.h>
#include "FunctionLib.h"
#include "BaseInformation.h"
#include <map>

D2D1BitmapReference* GameLogic::GameBackground = NULL;
std::vector<GameLogic::PlayerInfo> GameLogic::Players({ { 100,true,{ NULL },NULL } });
GameLogic::PawnBase* GameLogic::DefPlayerPawn = NULL; //作为模板，不要修改
float GameLogic::BackgroundState = 0;
float GameLogic::FlySpeed = 0;
std::map<std::string, SoundReference> SoundPool;
std::map<std::string, D2D1BitmapReference> D2D1BitmapPool;
/*template<typename TSelf = GameLogic::VoidType, typename TTarget = GameLogic::VoidType>
std::thread* ScriptExecuter(char ScriptPath[], long ExecuteLocation, void* Self, void* Target, bool Async, std::thread** ThreadSlot);*/
extern std::vector<D2DFrameAnimation*> FrameAnimationCollection;
extern BaseInformation Sys;
std::mutex RenderLock;
std::mutex LogicLock;

GameLogic::AmmoBase::AmmoBase() {}
GameLogic::AmmoBase::AmmoBase(const AmmoBase& _AmmoBase)
{
	*this = _AmmoBase;
	AmmoImage = NULL;
	HittedSound = NULL;
	this->HittedAnimation.AnimationBitmap = NULL;
	D2D1BitmapCpyRef(&this->AmmoImage, _AmmoBase.AmmoImage);
	D2D1BitmapCpyRef(&this->HittedAnimation.AnimationBitmap, _AmmoBase.HittedAnimation.AnimationBitmap);
	SoundCpyRef(&this->HittedSound, _AmmoBase.HittedSound);
}
GameLogic::AmmoBase::~AmmoBase()
{
	D2D1BitmapCpyRef(&AmmoImage, NULL);
	SoundCpyRef(&HittedSound, NULL);
	D2D1BitmapCpyRef(&HittedAnimation.AnimationBitmap, NULL);
}

GameLogic::PawnBase::PawnBase() {}
GameLogic::PawnBase::PawnBase(const PawnBase& _PawnBase)
{
	*this = _PawnBase;
	PawnImage = NULL;
	DeadSound = NULL;
	this->DeadAnimation.AnimationBitmap = NULL;
	D2D1BitmapCpyRef(&this->PawnImage, _PawnBase.PawnImage);
	D2D1BitmapCpyRef(&this->DeadAnimation.AnimationBitmap, _PawnBase.DeadAnimation.AnimationBitmap);
	SoundCpyRef(&this->DeadSound, _PawnBase.DeadSound);
}
GameLogic::PawnBase::~PawnBase()
{
	D2D1BitmapCpyRef(&PawnImage, NULL);
	SoundCpyRef(&DeadSound, NULL);
	D2D1BitmapCpyRef(&DeadAnimation.AnimationBitmap, NULL);
}

GameLogic::ItemBase::ItemBase() {}
GameLogic::ItemBase::ItemBase(const ItemBase& _ItemBase)
{
	*this = _ItemBase;
	ItemImage = NULL;
	PickedSound = NULL;
	PickedAnimation.AnimationBitmap = NULL;
	D2D1BitmapCpyRef(&this->ItemImage, _ItemBase.ItemImage);
	D2D1BitmapCpyRef(&this->PickedAnimation.AnimationBitmap, _ItemBase.PickedAnimation.AnimationBitmap);
	SoundCpyRef(&this->PickedSound, _ItemBase.PickedSound);
}
GameLogic::ItemBase::~ItemBase()
{
	D2D1BitmapCpyRef(&ItemImage, NULL);
	SoundCpyRef(&PickedSound, NULL);
	D2D1BitmapCpyRef(&PickedAnimation.AnimationBitmap, NULL);
}

void VoiceCallback::OnVoiceProcessingPassEnd() {}
void VoiceCallback::OnVoiceProcessingPassStart(UINT32 SamplesRequired) {}
void VoiceCallback::OnBufferEnd(void * pBufferContext) { }
void VoiceCallback::OnBufferStart(void * pBufferContext) {}
void VoiceCallback::OnLoopEnd(void * pBufferContext) {}
void VoiceCallback::OnVoiceError(void * pBufferContext, HRESULT Error) {}
VoiceCallback::VoiceCallback(SoundReference* _SoundRef,XAUDIO2_BUFFER* _XAudio2Buffer)
{
	SoundRef = _SoundRef;
	XAudio2Buffer = _XAudio2Buffer;
}

void VoiceCallback::OnStreamEnd() 
{
	SourceVoice->Stop();
	SourceVoice->DestroyVoice();
	SoundRef->ReferenceCount--;
	if (SoundRef->ReferenceCount <= 0)
	{
		delete SoundRef->SoundBuffer;
		SoundPool.erase(SoundRef->SoundPath);
	}
	delete XAudio2Buffer;
	delete this;
}

void D2DFrameAnimation::PlayAtLocation(D2D1_POINT_2F _Location)
{
	if (!AnimationBitmap)
		return;
	Location = _Location;
	auto TmpFrameAnimation = new D2DFrameAnimation(*this);
	TmpFrameAnimation->AnimationBitmap = NULL;
	D2D1BitmapCpyRef(&TmpFrameAnimation->AnimationBitmap, AnimationBitmap);
	FrameAnimationCollection.push_back(TmpFrameAnimation);
}

bool D2DFrameAnimation::RenderOneTick()
{
	D2D1_SIZE_U RenderTargetSize = Sys.D2D1HwndRenderTarget->GetPixelSize();
	D2D1_SIZE_U BlockSize = { AnimationBitmap->D2DBitmap->GetPixelSize().width / XCount,AnimationBitmap->D2DBitmap->GetPixelSize().height / YCount };
	Sys.D2D1HwndRenderTarget->DrawBitmap(AnimationBitmap->D2DBitmap,
		D2D1::RectF((Location.x - RenderSize.width / 2)*RenderTargetSize.width, (Location.y - RenderSize.height / 4)*RenderTargetSize.height,
		(Location.x + RenderSize.width / 2)*RenderTargetSize.width, (Location.y + RenderSize.height / 4)*RenderTargetSize.height),1.f,D2D1_BITMAP_INTERPOLATION_MODE_LINEAR,
		D2D1::RectF((FLOAT)(AnimationState%XCount*BlockSize.width),(FLOAT)((int)(AnimationState/XCount)*BlockSize.height),
		(FLOAT)((AnimationState%XCount+1)*BlockSize.width), (FLOAT((int)(AnimationState / XCount+1)*BlockSize.height))));
	AnimationState++;
	if (AnimationState >= XCount*YCount)
		return true;
	return false;
}

D2DFrameAnimation::~D2DFrameAnimation()
{
	D2D1BitmapCpyRef(&AnimationBitmap, NULL);
}