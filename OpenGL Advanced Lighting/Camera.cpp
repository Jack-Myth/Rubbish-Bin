#include "Camera.h"
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

Camera::Camera(float initAspect, float initFOV/*=90.f*/, glm::vec3 initPos/*=glm::vec3(0,0,0)*/)
{
	CameraPos = initPos;
	CameraRotation = glm::vec3(0, 0, 0);
	fov = initFOV;
	Aspect = initAspect;
	ProjectionMatrix = glm::perspective(glm::radians(fov), Aspect, NearPanel,FarPanel);
}

void Camera::SetCameraLocation(glm::vec3 newPos)
{
	CameraPos = newPos;
}

glm::vec3 Camera::GetCameraLocation()
{
	return CameraPos;
}

glm::vec3 Camera::GetCameraRotation()
{
	return CameraRotation;
}

void Camera::Move(glm::vec3 PosOffset)
{
	glm::mat4x4 RotationMatrix(1.f);
	RotationMatrix = glm::rotate(RotationMatrix, glm::radians(CameraRotation.z), glm::vec3(0, 1, 0));
	RotationMatrix = glm::rotate(RotationMatrix, glm::radians(CameraRotation.y), glm::vec3(1, 0, 0));
	//RotationMatrix Will Ignore the Roll axis.
	PosOffset = RotationMatrix * glm::vec4(PosOffset, 1.f);
	CameraPos += PosOffset;
}

void Camera::SetCameraRotation(glm::vec3 newRotation)
{
	CameraRotation = newRotation;
}

void Camera::Rotate(glm::vec3 RotationOffset)
{
	CameraRotation += RotationOffset;
	if (CameraRotation.y > 85 || CameraRotation.y < -85)
		CameraRotation.y -= RotationOffset.y;
}

glm::mat4x4 Camera::GetViewMatrix()
{
	glm::mat4x4 RotationMatrix(1.f);
	glm::vec3 Direction(0,0,-1);
	RotationMatrix = glm::rotate(RotationMatrix, glm::radians(CameraRotation.z), glm::vec3(0, 1, 0));
	RotationMatrix = glm::rotate(RotationMatrix, glm::radians(CameraRotation.y), glm::vec3(1, 0, 0));
	//RotationMatrix Will Ignore the Roll axis.
	Direction = RotationMatrix * glm::vec4(Direction, 1.f);
	return glm::lookAt(CameraPos, CameraPos + Direction, glm::vec3(0, 1, 0)); //LookAt Matrix
}

const glm::mat4x4& Camera::GetProjectionMatrix()
{
	return ProjectionMatrix;
}

void Camera::SetAspect(float newAspect)
{
	Aspect = newAspect;
	ProjectionMatrix = glm::perspective(glm::radians(fov), Aspect, NearPanel, FarPanel);
}

float Camera::GetFOV()
{
	return fov;
}

void Camera::SetFOV(float newFOV)
{
	fov = newFOV;
	ProjectionMatrix = glm::perspective(glm::radians(fov), Aspect, NearPanel, FarPanel);
}

void Camera::SetNearPanel(float newNear)
{
	NearPanel = newNear;
	ProjectionMatrix = glm::perspective(glm::radians(fov), Aspect, NearPanel,FarPanel);
}

void Camera::SetFarPanel(float newFar)
{
	FarPanel = newFar;
	ProjectionMatrix = glm::perspective(glm::radians(fov), Aspect, NearPanel, FarPanel);
}

glm::vec3 Camera::GetForwardVector()
{
	glm::mat4x4 RotationMatrix(1.f);
	glm::vec3 Direction(0, 0, -1);
	RotationMatrix = glm::rotate(RotationMatrix, glm::radians(CameraRotation.z), glm::vec3(0, 1, 0));
	RotationMatrix = glm::rotate(RotationMatrix, glm::radians(CameraRotation.y), glm::vec3(1, 0, 0));
	//RotationMatrix Will Ignore the Roll axis.
	Direction = RotationMatrix * glm::vec4(Direction, 1.f);
	return Direction;
}
