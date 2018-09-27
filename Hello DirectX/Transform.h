#pragma once
#include <DirectXMath.h>

struct FTransform
{
	DirectX::XMFLOAT3 Location=DirectX::XMFLOAT3(0,0,0);

	//Warning:I Use X as Roll,Y as Pitch and Z as Yaw
	DirectX::XMFLOAT3 Rotation=DirectX::XMFLOAT3(0, 0, 0);
	
	DirectX::XMFLOAT3 Scale= DirectX::XMFLOAT3(1, 1, 1);
	DirectX::XMMATRIX GenTransformMatrix();
};