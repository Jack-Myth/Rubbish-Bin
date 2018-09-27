#include "Camera.h"

void Camera::ProcessMouseInput(int DetailX, int DetailY)
{
	CameraTransform.Rotation.z += DetailX/10.f;
	if (CameraTransform.Rotation.y + DetailY/10.f > 80 || CameraTransform.Rotation.y + DetailY/10.f < -80)
		return;
	CameraTransform.Rotation.y += DetailY/10.f;
}

DirectX::XMMATRIX Camera::GenViewMatrix()
{
	DirectX::XMVECTOR Direction = DirectX::XMVectorSet(0, 0, 1, 0);
	DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(
		CameraTransform.Rotation.y, CameraTransform.Rotation.z, 0);
	Direction = DirectX::XMVector3TransformCoord(Direction, RotationMatrix);
	DirectX::XMVECTOR LocationVector = DirectX::XMLoadFloat3(&CameraTransform.Location);
	return DirectX::XMMatrixLookAtLH(LocationVector, DirectX::XMVectorAdd(LocationVector, Direction), DirectX::XMVectorSet(0, 1, 0, 0));
}

DirectX::XMMATRIX Camera::GenProjectionMatrix()
{
	return DirectX::XMMatrixPerspectiveFovLH(0.4f*3.14f, ViewportSize.x / ViewportSize.y, NearClipPlane, FarClipPlane);
}

