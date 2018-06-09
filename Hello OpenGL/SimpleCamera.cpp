#include "SimpleCamera.h"
#include <glfw3.h>

extern glm::mat4x4 ProjectionMatrix;
extern glm::vec2 ScreenSize;
void SimpleCamera::MoveCameraLocal(glm::vec3 LocationOffset)
{
	glm::vec3 cameraDirection = glm::vec3(0, 0, -1.f);
	glm::vec3 cameraRight;
	glm::mat4x4 rotationMartix(1.f);
	rotationMartix = glm::rotate(rotationMartix, glm::radians(cameraRotation.z), glm::vec3(0, 1, 0));
	rotationMartix = glm::rotate(rotationMartix, glm::radians(cameraRotation.y), glm::vec3(1, 0, 0));
	cameraDirection = (rotationMartix*glm::vec4(cameraDirection.r, cameraDirection.g, cameraDirection.b, 1.f));
	cameraDirection = glm::normalize(cameraDirection);
	cameraPos += cameraDirection * LocationOffset.z;
	cameraRight = glm::cross(glm::vec3(0, 1, 0), cameraDirection);
	cameraPos += cameraRight * LocationOffset.x;
}

void SimpleCamera::AddCameraRotation(glm::vec3 RotationOffset)
{
		cameraRotation += RotationOffset;
		if (cameraRotation.y > 85 || cameraRotation.y < -85)
			cameraRotation.y -= RotationOffset.y;
}

void SimpleCamera::SetFOV(float newFOV)
{
	fov = newFOV;
	ProjectionMatrix = glm::perspective(fov, ScreenSize.x / ScreenSize.y, 0.1f, 1000.f);
}

float SimpleCamera::GetFOV()
{
	return fov;
}

const glm::mat4x4 SimpleCamera::GetViewMatrix()
{
	glm::mat4x4 LookAtMatrix;
	glm::vec3 cameraDirection=glm::vec3(0,0,-1.f);
	glm::mat4x4 rotationMartix(1.f);
	rotationMartix = glm::rotate(rotationMartix, glm::radians(cameraRotation.z), glm::vec3(0, 1, 0));
	rotationMartix = glm::rotate(rotationMartix, glm::radians(cameraRotation.y), glm::vec3(1, 0, 0));
	cameraDirection = (rotationMartix*glm::vec4(cameraDirection.r, cameraDirection.g, cameraDirection.b, 1.f));
	cameraDirection = glm::normalize(cameraDirection);
	LookAtMatrix = glm::lookAt(cameraPos,cameraPos+cameraDirection,glm::vec3(0,1,0));
	return LookAtMatrix;
}

