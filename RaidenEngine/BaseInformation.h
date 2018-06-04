#pragma once
#include <stdio.h>
#include <thread>
#include <d2d1.h>
#include <vector>
#include <dwrite.h>
#include <dshow.h>
#include <wincodec.h>
#include <xaudio2.h>
#include <map>
#include <string>
#include <mutex>

struct BaseInformation
{
	HWND ViewportHWND;
	ID2D1Factory* D2D1Factory = NULL;
	IDWriteFactory* DWriteFactory = NULL;
	ID2D1HwndRenderTarget* D2D1HwndRenderTarget = NULL;
	IWICImagingFactory* WICImagingFactory = NULL;
	IDWriteTextFormat* BaseTextFormat=NULL;
	std::thread* ScriptExecuter = NULL;
	struct DirectShowInfo
	{
		IGraphBuilder *pGraph = NULL;
		IMediaControl *pControl = NULL;
		IMediaEvent   *pEvent = NULL;
		IMediaSeeking *pSeeking = NULL;
		IVMRWindowlessControl* pWindowLessControl = NULL;
	} DirectShowInfo;
	IXAudio2* XAudio2Engine=NULL;
	IXAudio2MasteringVoice* XAudio2MasteringVoice=NULL;
};

struct D2D1BitmapReference
{
	std::string PicturePath;
	int ReferenceCount = 0;
	ID2D1Bitmap* D2DBitmap=NULL;
};

struct ScriptExecutePoint
{
	char FilePath[256] = { NULL };
	long ScriptPoint = -1;
	bool ShouldAsync=false;
	//bool Set(char _FilePath[],long _ScriptPoint);
	template<typename TSelf,typename TTarget>
	void Execute(void* Self,void* Target);
	std::vector<bool*> Thread_ExitSignal;
};

struct SoundReference
{
	std::string SoundPath;
	BYTE* SoundBuffer = NULL;
	WAVEFORMATEX WaveFormat = { NULL };
	long BufferLength = 0;
	int ReferenceCount = 0;
	//IXAudio2SourceVoice* XAudio2SourceVoice=NULL;
};

struct D2DFrameAnimation
{
	D2D1BitmapReference* AnimationBitmap=NULL;
	int XCount = 0;
	int YCount = 0;
	int FramePerSecond=0;
	int AnimationState = 0;
	D2D1_SIZE_F RenderSize = { 0.f,0.f }; //By Percent of Screen
	bool Started=false;
	D2D1_POINT_2F Location;
	void PlayAtLocation(D2D1_POINT_2F _Location); //Submit Render Task To RenderGame()
	bool RenderOneTick(); //Return True If Animation Is Finished,Use For RenderGame()
	int MaxDelay = 0;
	~D2DFrameAnimation();
private:
	int Delay = 0;
};

namespace GameLogic
{
	struct AmmoBase
	{
		//char Name[200] = {NULL};
		bool Enabled = true;
		D2D1BitmapReference* AmmoImage = NULL;
		D2DFrameAnimation HittedAnimation;
		SoundReference* HittedSound = NULL;
		bool FromPlayer = false;
		int Attack = 0;
		float AttackRadius = 0;
		D2D1_POINT_2F Location = D2D1::Point2F(0.5, 0.5);
		D2D1_POINT_2F MoveDirection = D2D1::Point2F(0, 1);
		float Speed = 0.1f;
		ScriptExecutePoint RootScript;
		ScriptExecutePoint OnHited;
		AmmoBase();
		AmmoBase(const AmmoBase&);
		~AmmoBase();
	};

	struct PawnBase
	{
		//char Name[200] = { NULL };
		bool Enabled = true;
		D2D1BitmapReference* PawnImage = NULL;
		D2DFrameAnimation DeadAnimation;
		SoundReference* DeadSound = NULL;
		bool ControlledByPlayer = false;
		float BodyRadius = 0;
		int Health = 1;
		int ShootDelayMax = 100 ;
		int ShootDelay = 0;
		float Speed = 0;
		D2D1_POINT_2F Location = D2D1::Point2F(0.5, 1.5);
		ScriptExecutePoint RootScript;
		ScriptExecutePoint OnDamaged;
		ScriptExecutePoint OnDead;
		std::string WeaponAmmo;
		PawnBase();
		PawnBase(const PawnBase&);
		~PawnBase();
	};

	struct ItemBase
	{
		//char Name[200] = { NULL };
		bool Enabled = true;
		D2D1BitmapReference* ItemImage = NULL;
		D2DFrameAnimation PickedAnimation;
		SoundReference* PickedSound = NULL;
		float PickRadius = 0;
		D2D1_POINT_2F Location = D2D1::Point2F(0.5, 0.5);
		D2D1_POINT_2F MoveDirection = D2D1::Point2F(0, 0);
		float Speed = 0;
		ScriptExecutePoint RootScript;
		ScriptExecutePoint OnPickedUp;
		ItemBase();
		ItemBase(const ItemBase&);
		~ItemBase();
	};
	struct PlayerInfo
	{
		int Life = 3;
		bool IsLocal = true;
		SOCKADDR_IN PlayerAddr = { NULL };
		PawnBase* PlayerPawn = NULL;
	};
	struct VoidType
	{
		D2D1_POINT_2F Location;
		bool Enabled = true;
	};
	extern D2D1BitmapReference* GameBackground;
	extern std::vector<PlayerInfo> Players;
	extern GameLogic::PawnBase* DefPlayerPawn; //作为模板，不要修改
	extern float FlySpeed;
	extern float BackgroundState;
};

struct WhileInfo
{
	long StreamPosition;
	char expression[200];
};

class VoiceCallback : public IXAudio2VoiceCallback
{
public:
	SoundReference* SoundRef=NULL;
	IXAudio2SourceVoice* SourceVoice=NULL;
	XAUDIO2_BUFFER* XAudio2Buffer=NULL;
	VoiceCallback(SoundReference* _SoundRef, XAUDIO2_BUFFER* _XAudio2Buffer);
	//Called when the voice has just finished playing a contiguous audio stream.
	void OnStreamEnd();
	//Unused methods are stubs
	void OnVoiceProcessingPassEnd();
	void OnVoiceProcessingPassStart(UINT32 SamplesRequired);
	void OnBufferEnd(void * pBufferContext);
	void OnBufferStart(void * pBufferContext);
	void OnLoopEnd(void * pBufferContext);
	void OnVoiceError(void * pBufferContext, HRESULT Error);
};

extern std::map<std::string, D2D1BitmapReference> D2D1BitmapPool;
extern std::map<std::string, SoundReference> SoundPool;
extern std::mutex RenderLock;
extern std::mutex LogicLock;