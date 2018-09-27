#pragma once
#include <DirectXMath.h>
#include "Transform.h"

class Camera
{
	FTransform CameraTransform;
	float FOV;
	float FarClipPlane=100000,NearClipPlane=5;
	DirectX::XMFLOAT2 ViewportSize;
	
public:
	inline void SetCameraTransform(const FTransform& newCameraTransform)
	{
		CameraTransform = newCameraTransform;
	}
	inline FTransform GetCameraTransform()
	{
		return CameraTransform;
	}
	inline void SetViewportSize(DirectX::XMFLOAT2 newSize)
	{
		ViewportSize = newSize;
	}
	inline DirectX::XMFLOAT2 GetViewportSize()
	{
		return ViewportSize;
	}

	inline void SetFOV(float newFOV)
	{
		FOV = newFOV;
	}
	inline float GetFOV()
	{
		return FOV;
	}

	void ProcessMouseInput(int DetailX, int DetailY);

	DirectX::XMMATRIX GenViewMatrix();
	DirectX::XMMATRIX GenProjectionMatrix();
};