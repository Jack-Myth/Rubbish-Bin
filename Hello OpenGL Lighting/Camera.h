#pragma once
#include "glm/glm.hpp"

//Notice: X as Roll, Y as Pitch, Z as Yaw
//Position XYZ use OpenGL Coodination;
class Camera
{
	glm::vec3 CameraPos;
	glm::vec3 CameraRotation;
	float fov, Aspect;
	glm::mat4x4 ProjectionMatrix;
public:
	Camera(float initAspect, float initFOV = 90.f, glm::vec3 initPos = glm::vec3(0, 0, 1.f));
	Camera(int WindowWidth, int WindowHeight, float initFOV = 90.f, glm::vec3 initPos = glm::vec3(0, 0, 0))
		:Camera((float)WindowWidth / WindowHeight, initFOV, initPos){};
	void SetCameraLocation(glm::vec3 newPos);
	glm::vec3 GetCameraLocation();

	//Notice: X as Roll, Y as Pitch, Z as Yaw
	glm::vec3 GetCameraRotation();
	void Move(glm::vec3 PosOffset);

	//Notice: X as Roll, Y as Pitch, Z as Yaw
	void SetCameraRotation(glm::vec3 newRotation);
	//Notice: X as Roll, Y as Pitch, Z as Yaw
	void Rotate(glm::vec3 RotationOffset);
	glm::mat4x4 GetViewMatrix();
	const glm::mat4x4& GetProjectionMatrix();
	void SetAspect(float newAspect);
	float GetFOV();
	void SetFOV(float newFOV);
};