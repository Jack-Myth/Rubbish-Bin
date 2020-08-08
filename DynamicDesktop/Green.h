#pragma once
#include "IModelBase.h"
#include "json.hpp"
#include "resource.h"
#include "Utils.h"
#include <functional>
#include <map>
#include "resource1.h"
#include "MediaFoundationUtils.h"

extern HWND DesktopWindowHwnd;

class Green : public IModelBase
{
	using json= nlohmann::json;

	struct GreenStateTransform
	{
		std::string Logic;
		std::vector<std::string> To;
	};
	struct GreenState
	{
		std::string VideoName;
		std::vector<GreenStateTransform> TransformLogic;
	};
	std::map<std::string, std::function<int(const std::vector<std::string>&)>> TransformFunctions;
	std::map<std::string, int> VideoResourceMapper;

	std::map<std::string,GreenState> StateCollection;
	
	std::string CurrentState;

	bool ForceWakeUp=false;
	inline static int Random(const std::vector<std::string>& States)
	{
		return rand() % States.size();
	}

	inline static int CheckSleep(const std::vector<std::string>& States)
	{
		RECT rcRect;
		HWND CurWindow = GetForegroundWindow();
		if (CurWindow == DesktopWindowHwnd)
			return -1;
		GetWindowRect(CurWindow, &rcRect);
		if (rcRect.right >= ScreenWidth * 0.55f && rcRect.left <= ScreenWidth * 0.45f)
			return Random(States);
		else
			return -1;
	}

	inline int CheckWake(const std::vector<std::string>& States)
	{
		if (ForceWakeUp)
		{
			ForceWakeUp = false;
			return Random(States);
		}
		else if (CheckSleep(States) < 0)
		{
			return Random(States);
		}
		return -1;
	}

	inline static int CheckLookAt(const std::vector<std::string>& States)
	{
		RECT rcRect;
		//判断太过简单粗暴，实际上还可能会有竖屏的情况
		//所以理论上应当加入屏幕长宽比进行计算
		HWND CurWindow = GetForegroundWindow();
		if (CurWindow == DesktopWindowHwnd)
			return -1;
		GetWindowRect(CurWindow, &rcRect);
		if (rcRect.right < ScreenWidth * 0.45f)
			return 0;
		else if (rcRect.left > ScreenWidth * 0.55f)
			return 1;
		else 
			return -1;
	}

	MediaEventCallback* GreenMFCallback=nullptr;
	int LastTryExitTime = 0;
	int TryExitCount = 0;
	bool NeedTransform = false,NeedReleaseMem=false;
	bool Played=false;
public:
	void TransformState()
	{
		auto& CurrentGreenState = StateCollection[CurrentState];
		for (auto& CurGST:CurrentGreenState.TransformLogic)
		{
			int To = TransformFunctions[CurGST.Logic](CurGST.To);
			if (To >= 0 && To < CurGST.To.size())
			{
				CurrentState = CurGST.To[To];
				break;
			}
		}
		PlayVideoByResource(TEXT("Green"),
			VideoResourceMapper[StateCollection[CurrentState].VideoName],
			DesktopHwnd, false, GreenMFCallback, false, false);
	}

	void StartupModel() override
	{
		InitMediaFoundation();
		//////////////////////////////////////////////////////////////////////////
		// Video Mapper and Function Mapper
		//////////////////////////////////////////////////////////////////////////
		VideoResourceMapper.insert_or_assign("Intro0", IDR_INTRO0);
		VideoResourceMapper.insert_or_assign("Intro1", IDR_INTRO1);
		VideoResourceMapper.insert_or_assign("Intro2", IDR_INTRO2);
		VideoResourceMapper.insert_or_assign("Idle", IDR_IDLE);
		VideoResourceMapper.insert_or_assign("IdleToSleepA", IDR_IDLETOSLEEPA);
		VideoResourceMapper.insert_or_assign("Play0", IDR_PLAY0);
		VideoResourceMapper.insert_or_assign("Play1", IDR_PLAY1);
		VideoResourceMapper.insert_or_assign("SeeLeft", IDR_SEELEFT);
		VideoResourceMapper.insert_or_assign("SeeRight", IDR_SEERIGHT);
		VideoResourceMapper.insert_or_assign("SleepA0", IDR_SLEEPA0);
		VideoResourceMapper.insert_or_assign("SleepA1", IDR_SLEEPA1);
		VideoResourceMapper.insert_or_assign("SleepALoop", IDR_SLEEPALOOP);
		VideoResourceMapper.insert_or_assign("SleepAToIdle", IDR_SLEEPATOIDLE);
		VideoResourceMapper.insert_or_assign("SleepAToSleepB", IDR_SLEEPATOSLEEPB);
		VideoResourceMapper.insert_or_assign("SleepBLoop", IDR_SLEEPBLOOP);
		VideoResourceMapper.insert_or_assign("SleepBToIdle", IDR_SLEEPBTOIDLE);

		TransformFunctions.insert_or_assign("Random", Random);
		TransformFunctions.insert_or_assign("CheckWake", 
			[this](const std::vector<std::string>& States){return CheckWake(States);});
		TransformFunctions.insert_or_assign("CheckSleep", 
			[this](const std::vector<std::string>& States) {return ForceWakeUp?-1:CheckSleep(States); });
		TransformFunctions.insert_or_assign("CheckLookAt", CheckLookAt);
		//////////////////////////////////////////////////////////////////////////
		// End Video Mapper and Function Mapper
		//////////////////////////////////////////////////////////////////////////
		int Length = 0;
		char* StateMachineBuffer = (char*)CreateBufferFromResource(TEXT("JSON"), IDR_GREEN_STATEMACHINE,&Length);
		char* SMStr = new char[Length + 1];
		memcpy(SMStr, StateMachineBuffer,Length);
		SMStr[Length] = 0;
		delete StateMachineBuffer;
		json StateList= json::parse(SMStr);
		for (auto& CurStateJson : StateList)
		{
			GreenState CurState;
			CurState.VideoName = CurStateJson["Video"];
			auto& TransformList = CurStateJson["Transform"];
			for (auto& CurTransformJson:TransformList)
			{
				GreenStateTransform CurTransform;
				CurTransform.Logic = CurTransformJson["Logic"];
				for (auto& ToJson : CurTransformJson["To"])
					CurTransform.To.push_back(ToJson);
				CurState.TransformLogic.push_back(CurTransform);
			}
			StateCollection.insert_or_assign(CurStateJson["StateName"], CurState);
		}
		CurrentState = "Enter";
		GreenMFCallback = new MediaEventCallback();
		GreenMFCallback->OnMediaEnd = [this]() 
		{
			this->NeedTransform = true;
		};
		TransformState();
	}


	void ShutdownModel() override
	{
	}


	void Tick(float delta) override
	{
		if (NeedTransform)
		{
			NeedTransform = false;
			TransformState();
		}

	}

	void OnMouseDown(int VKey, bool IsOnDesktop) override
	{
		if (!IsOnDesktop)
			return;
		switch (VKey)
		{
		case VK_LBUTTON:
			ForceWakeUp = true;
			break;
		case VK_MBUTTON:
			if (LastTryExitTime < 1000)
				TryExitCount += 1;
			else
				TryExitCount = 0;
			LastTryExitTime = 0;
			if (TryExitCount >= 5)
			{
				MessageBox(NULL, TEXT("Designed By JackMyth"), TEXT("Dynamic Desktop"), MB_OK);
				exit(0);
			}
			break;
		default:
			break;
		}
	}

	void OnMouseUp(int VKey) override
	{

	}
};