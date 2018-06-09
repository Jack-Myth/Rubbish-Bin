#include "SimpleCamera.h"
#include <glfw3.h>

SimpleCamera::SimpleCamera()
{
	cameraDirection = glm::normalize(cameraPos - cameraTarget);
	cameraRight = glm::normalize(glm::cross(glm::vec3(0.0f, 1.0f, 0.0f), cameraDirection));
	cameraUp = glm::normalize(glm::cross(cameraDirection, cameraRight));
}

void SimpleCamera::MoveCamera(glm::vec3 LocationOffset)
{
	cameraPos += LocationOffset;
	ViewMatrix=glm::translate(ViewMatrix, -LocationOffset);
}

void SimpleCamera::AddCameraRotation(glm::vec3 RotationOffset)
{
	
}

const glm::mat4x4 SimpleCamera::GetViewMatrix()
{
	//glm::translate(ViewMatrix, glm::vec3(0, 0, -10.f));
	glm::mat4x4 LookAtMatrix;
	float radius = 7.f;
	float camX = sin(glfwGetTime())*radius;
	float camY = cos(glfwGetTime())*radius;
	LookAtMatrix = glm::lookAt(glm::vec3(camX, 0, camY),glm::vec3(0,0,0),glm::vec3(0,1,0));
	return LookAtMatrix;
}

