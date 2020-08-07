#pragma once
#include "IModelBase.h"
#include "json.hpp"
#include "resource.h"
#include "Utils.h"
#include <functional>
#include <map>

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
	std::map<std::string, std::function<bool(std::vector<std::string>)>> TransformFunctions;
	std::map<std::string, int> VideoResourceMapper;

	std::vector<GreenState> StateCollection;
public:
	void StartupModel() override
	{
		//////////////////////////////////////////////////////////////////////////
		// Video Mapper and Function Mapper
		//////////////////////////////////////////////////////////////////////////



		//////////////////////////////////////////////////////////////////////////
		// End Video Mapper and Function Mapper
		//////////////////////////////////////////////////////////////////////////
		int Length = 0;
		char* StateMachineBuffer = (char*)CreateBufferFromResource("Json", IDR_GREEN_STATEMACHINE,&Length);
		char* SMStr = new char[Length + 1];
		memcpy(SMStr, StateMachineBuffer);
		SMStr[Length] = 0;
		delete StateMachineBuffer;
		json StateList= json::parse(SMStr);
		for (auto& CurStateJson : StateList)
		{
			GreenState CurState;
			std::string VideoName = CurStateJson["VideoName"];
			auto& TransformList = CurStateJson["Transform"];
			for (auto& CurTransformJson:TransformList)
			{
				GreenStateTransform CurTransform;
				CurTransform.Logic = CurTransformJson["Logic"];
				for (auto& ToJson : CurTransformJson["To"])
					CurTransform.To.push_back(ToJson);
				CurState.TransformLogic.push_back(CurTransform);
			}
		}
	}


	void ShutdownModel() override
	{
		throw std::logic_error("The method or operation is not implemented.");
	}


	void Tick(float delta) override
	{
		throw std::logic_error("The method or operation is not implemented.");
	}


	void OnMouseDown(int VKey, bool IsOnDesktop) override
	{
		throw std::logic_error("The method or operation is not implemented.");
	}


	void OnMouseUp(int VKey) override
	{
		throw std::logic_error("The method or operation is not implemented.");
	}

};