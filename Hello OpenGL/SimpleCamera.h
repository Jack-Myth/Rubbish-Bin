#pragma once
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"

class SimpleCamera
{
	glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
	glm::vec3 cameraRotation=glm::vec3(0,0,0);
	glm::mat4x4 ViewMatrix = glm::mat4x4(1.f);
public:
	SimpleCamera()=default;
	void MoveCameraLocal(glm::vec3 LocationOffset);
	void AddCameraRotation(glm::vec3 RotationOffset);
	const glm::mat4x4 GetViewMatrix();
};