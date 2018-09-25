#pragma once
#include <DirectXMath.h>

struct FTransform
{
	DirectX::XMFLOAT3 Location;

	//Warning:I Use X as Roll,Y as Pitch and Z as Yaw
	DirectX::XMFLOAT3 Rotation;
	
	DirectX::XMFLOAT3 Scale;
	DirectX::XMMATRIX GenTransformMatrix();
};