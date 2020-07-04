#pragma once
#include <windows.h>

class IModelBase
{
public:
	virtual void StartupModel() = 0;
	virtual void ShutdownModel() = 0;
	virtual void Tick(float delta) = 0;
	virtual void OnMouseDown(int VKey,bool IsOnDesktop) = 0;
	virtual void OnMouseUp(int VKey) = 0;
};