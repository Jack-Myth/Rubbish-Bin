#include <stdio.h>
#include <windows.h>
#include <vector>
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include <GL/gl.h>
#include "Camera.h"
#include "GLFWMainWindow.h"
#include "cmath"
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib, "opengl32.lib")
#define PI 3.141592

void ProcessInput(GLFWwindow* pWindow);
void ConstructShape(float ShapeHeight, float RoundSize, int HightSplit, int RoundSplit);
void Animation();
void Render();

GLFWMainWindow* pMainWindow;
Camera* pMyCamera;
float moveSpeed = 1.f;
glm::vec3 CameraPos = glm::vec3(1, 1, 1), ModelRotation = glm::vec3(0, 0, 0);
glm::mat4 ModelM = glm::mat4(1.f);
std::vector<std::vector<glm::vec3>> Fragments;  //Fragments<Points<float3>>
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
	pMainWindow = new GLFWMainWindow(1024, 768, "Big Homework!");
	pMyCamera = new Camera(4.f / 3.f);
	glfwSetInputMode(pMainWindow->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	pMainWindow->AttachCamera(pMyCamera);
	ConstructShape(15, 10, 5, 30);
	glViewport(0, 0, 1024, 768);
	while (!glfwWindowShouldClose(pMainWindow->GetWindow()))
	{
		ProcessInput(pMainWindow->GetWindow());
		Animation();
		Render();
		glfwSwapBuffers(pMainWindow->GetWindow());
		glfwPollEvents();
	}
}

void Animation()
{
	glm::mat4 tmpZero(1.f);
	ModelM = glm::rotate(tmpZero, sinf(glfwGetTime()), glm::vec3(0, 0, 1));
}

void ProcessInput(GLFWwindow* pWindow)
{
	if (glfwGetKey(pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(pWindow, GL_TRUE);
	if (glfwGetKey(pWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		moveSpeed = 10.f;
	else
		moveSpeed = 1.f;
	if (glfwGetKey(pWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
		moveSpeed = 0.1f;
	if (glfwGetKey(pWindow, GLFW_KEY_W) == GLFW_PRESS)
		pMyCamera->Move(glm::vec3(0, 0, -0.2f) * moveSpeed);
	if (glfwGetKey(pWindow, GLFW_KEY_S) == GLFW_PRESS)
		pMyCamera->Move(glm::vec3(0, 0, 0.2f) * moveSpeed);
	if (glfwGetKey(pWindow, GLFW_KEY_A) == GLFW_PRESS)
		pMyCamera->Move(glm::vec3(-0.2f, 0, 0) * moveSpeed);
	if (glfwGetKey(pWindow, GLFW_KEY_D) == GLFW_PRESS)
		pMyCamera->Move(glm::vec3(0.2f, 0, 0) * moveSpeed);
}

void ConstructShape(float ShapeHeight,float RoundSize,int HightSplit,int RoundSplit)
{
	Fragments.clear();
	std::vector<std::vector<glm::vec3>> Points(HightSplit+1, std::vector<glm::vec3>(RoundSplit + 1));
	//Surface
	for (int hindex=0;hindex<=HightSplit;hindex++)
	{
		float h = hindex / (float)HightSplit * ShapeHeight;
		for (int rindex=0;rindex<=RoundSplit;rindex++)
		{
			float r = rindex / (float)RoundSplit * 360.f;
			Points[hindex][rindex] = glm::vec3(sin(r * PI / 180.f) * glm::max(RoundSize * (1.f - h / ShapeHeight), 0.001f),
				h, cos(r * PI / 180.f) * glm::max(RoundSize * (1.f - h / ShapeHeight),0.001f));
		}
	}
	//the point at Buttom Center is no needed,Manually add it when constructing Fragments;
	//Construct Fragments:
	for (int hindex = HightSplit; hindex > 0; hindex--)
	{
		for (int rindex=0;rindex<RoundSplit;rindex++)
		{
			Fragments.push_back(std::vector<glm::vec3>());
			std::vector<glm::vec3>* pworkingFragment = &Fragments[Fragments.size()-1];
			//Push in Clockwise
			//For Normal Checking.
			pworkingFragment->push_back(Points[hindex][rindex]);
			pworkingFragment->push_back(Points[hindex][rindex + 1]);
			pworkingFragment->push_back(Points[hindex - 1][rindex + 1]);
			pworkingFragment->push_back(Points[hindex - 1][rindex]);
		}
	}
	//Construct Buttom Surfaces;
	for (int rindex=0;rindex<RoundSplit;rindex++)
	{
		Fragments.push_back(std::vector<glm::vec3>());
		std::vector<glm::vec3>* pworkingFragment = &Fragments[Fragments.size()-1];
		//Push in Clockwise
		pworkingFragment->push_back(Points[0][rindex]);
		pworkingFragment->push_back(Points[0][rindex + 1]);
		pworkingFragment->push_back(glm::vec3(0, 0, 0)); //Manually added Center point.
	}
	//Construction finished.
}

void Render()
{
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glLineWidth(10.f);
	glColor3f(0.4f, 0.8f, 1.f);
	glm::mat4 ProjectionMatrix = pMyCamera->GetProjectionMatrix();
	glm::mat4 ViewMatrix = pMyCamera->GetViewMatrix();
	for (std::vector<glm::vec3>& Fragment:Fragments)
	{
		//Check Normal
		std::vector<glm::vec3> tmpFragment = Fragment;
		for (int i=0;i<3;i++)
			tmpFragment[i] = ModelM*glm::vec4(tmpFragment[i], 1.f);
		glm::vec3 NormalVector = glm::normalize(glm::cross(tmpFragment[0] - tmpFragment[1], tmpFragment[2] - tmpFragment[1]));
		glm::vec3 CameraDirectionVector = tmpFragment[0] - pMyCamera->GetCameraLocation();
		//CameraForwardVector = glm::vec3(CameraForwardVector.z, CameraForwardVector.x, CameraForwardVector.x);
		if (glm::dot(CameraDirectionVector, NormalVector) <= 0)
		{
			//Draw this fragment.
			glm::vec3 WorkdingPoint = ProjectionMatrix * ViewMatrix * ModelM * glm::vec4(Fragment[0], 1.f);
			WorkdingPoint /= WorkdingPoint.z;
			for (unsigned int i=1;i<Fragment.size()+1;i++)
			{
				glm::vec3 tmpPoint = ProjectionMatrix * ViewMatrix * ModelM * glm::vec4(Fragment[i % Fragment.size()], 1.f);
				tmpPoint /= tmpPoint.z;
				glBegin(GL_LINES);
				glVertex2f(WorkdingPoint.x, WorkdingPoint.y);
				glVertex2f(tmpPoint.x, tmpPoint.y);
				glEnd();
				WorkdingPoint = tmpPoint;
			}
		}
	}
	glFlush();
}