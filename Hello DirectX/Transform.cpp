#include "Transform.h"

DirectX::XMMATRIX FTransform::GenTransformMatrix()
{
	DirectX::XMMATRIX tmpTargetMatrix = DirectX::XMMatrixTranslation(Location.x, Location.y, Location.z);
	tmpTargetMatrix *= DirectX::XMMatrixRotationRollPitchYaw(Rotation.y, Rotation.z, Rotation.x);
	tmpTargetMatrix *= DirectX::XMMatrixScaling(Scale.x, Scale.y, Scale.z);
	return tmpTargetMatrix;
}
