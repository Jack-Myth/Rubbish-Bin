#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <windows.h>
#include <vector>
//#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#pragma comment(lib,"glfw3.lib")
#include <GL/gl.h>
#include "Camera.h"
#include "GLFWMainWindow.h"
#include "cmath"
#pragma comment(lib, "opengl32.lib")
typedef glm::vec3 FPoint;

class BSurface
{
	inline float Base0_2(float t)
	{
		return 0.5f* (t * t - 2 * t + 1);
	}
	inline float Base1_2(float t)
	{
		return 0.5f* (-2 * t * t + 2 * t + 1);
	}
	inline float Base2_2(float t)
	{
		return 0.5f* t* t;
	}
public:
	std::vector<FPoint> ControlPoints;
	int SizeU,SizeV;
	BSurface() = default;
	BSurface(int mSizeU,int mSizeV,std::initializer_list<FPoint> InitControlPoints)
	{
		SizeU = mSizeU;
		SizeV = mSizeV;
		ControlPoints.insert(ControlPoints.begin(), InitControlPoints);
	}
	FPoint Calculate(float u,float v)
	{
		int UIndexBegin = (int)(u * (SizeU - 2)); //+0,+1,+2
		u = u * (SizeU - 2);
		u -= (int)u;
		int VIndexBegin = (int)(v * (SizeV - 2)); //+0,+1,+2
		v = v * (SizeV - 2);
		v -= (int)v;
		/*if (VIndexBegin >= SizeV - 2)
			VIndexBegin--;*/
		FPoint result = glm::vec3(0);
		for (int i=0;i<3;i++)
		{
			for (int j=0;j<3;j++)
			{
				//Both 2-Times B-spline
				float tmpFloat = 0;
				switch (j)
				{
				case 0:
					tmpFloat = Base0_2(u);
					break;
				case 1:
					tmpFloat = Base1_2(u);
					break;
				case 2:
					tmpFloat = Base2_2(u);
					break;
				}
				switch (i)
				{
				case 0:
					tmpFloat *= Base0_2(v);
					break;
				case 1:
					tmpFloat *= Base1_2(v);
					break;
				case 2:
					tmpFloat *= Base2_2(v);
					break;
				}
				result = result + ControlPoints[(VIndexBegin+i) * SizeU + (UIndexBegin+j)] * tmpFloat;
			}
		}
		return result;
	}
};

void ProcessInput(GLFWwindow* pWindow);
void Render();
void InitMatrix();

BSurface GlobalBSurface(4, 3,
	{ {0,0,0},{10,0,0},{20,0,0},{30,-10,0},
		{0,0,10},{10,0,10},{20,0,10},{30,0,10},
		{0,0,20},{10,0,20},{20,0,20},{30,0,20} });

GLFWMainWindow* pMainWindow;
Camera* pMyCamera;
float moveSpeed = 1.f;
glm::vec3 CameraPos=glm::vec3(1,1,1), ModelRotation= glm::vec3(0, 0, 0);
glm::mat4 ModelM=glm::mat4(1.f), ViewM, ProjectionM;
int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd )
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_ANY_PROFILE);
	InitMatrix();
	pMainWindow = new GLFWMainWindow(1024, 768,"Big Homework!");
	pMyCamera = new Camera(4.f / 3.f);
	glfwSetInputMode(pMainWindow->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	pMainWindow->AttachCamera(pMyCamera);
	//gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glViewport(0, 0, 1024, 768);
	while (!glfwWindowShouldClose(pMainWindow->GetWindow()))
	{
		ProcessInput(pMainWindow->GetWindow());
		Render();
		glfwSwapBuffers(pMainWindow->GetWindow());
		glfwPollEvents();
	}
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
	if (glfwGetKey(pWindow, GLFW_KEY_HOME) == GLFW_PRESS || glfwGetKey(pWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
	{
		char FileP[1024] = { 0 };
		OPENFILENAMEA OpenFN = { NULL };
		OpenFN.lStructSize = sizeof(OPENFILENAMEA);
		OpenFN.Flags = OFN_FILEMUSTEXIST;
		OpenFN.lpstrFilter = "控制点数据文件\0*.txt\0\0";
		OpenFN.nMaxFile = MAX_PATH;
		OpenFN.lpstrFile = FileP;
		OpenFN.hInstance = GetModuleHandle(NULL);
		OpenFN.lpstrTitle = "控制点数据文件";
		if (GetOpenFileNameA(&OpenFN))
		{
			FILE* DataFile = fopen(OpenFN.lpstrFile, "r");
			int uCount, vCount;
			(void)fscanf(DataFile, "%d,%d", &uCount, &vCount);
			GlobalBSurface.SizeU = uCount;
			GlobalBSurface.SizeV = vCount;
			GlobalBSurface.ControlPoints.clear();
			GlobalBSurface.ControlPoints.resize(uCount * vCount);
			for (int v=0;v<vCount;v++)
			{
				for (int u=0;u<uCount;u++)
				{
					int x, y, z;
					(void)fscanf(DataFile, "%d,%d,%d;", &x, &y, &z);
					GlobalBSurface.ControlPoints[v * uCount + u] = glm::vec3(x, y, z);
				}
			}
		}
	}


	//Move
	GlobalBSurface.ControlPoints[4].y = sin(glfwGetTime()) * 10;
	GlobalBSurface.ControlPoints[7].y = sin(glfwGetTime()) * 10;
}

void InitMatrix()
{
	ProjectionM = glm::perspective(glm::radians(90.f), 1024.f / 768.f, 0.01f, 10000.f);
}

void Render()
{
	//Clear
	glClearColor(1, 1, 1, 1);
	glClear(GL_COLOR_BUFFER_BIT);
	glLineWidth(5);
	glm::mat4 ProjectionMatrix = pMyCamera->GetProjectionMatrix();
	glm::mat4 ViewMatrix = pMyCamera->GetViewMatrix();
	for (float u=0;u<=1.f;u+=0.05f)
	{
		glm::vec4 WorkingPoint = ProjectionMatrix * ViewMatrix * ModelM * glm::vec4(GlobalBSurface.Calculate(u, 0.f),1.f);
		WorkingPoint /= WorkingPoint.z;
		for (float v = 0.05f; v <= 1.f;v += 0.05f)
		{
			glm::vec4 tmpPoint = ProjectionMatrix * ViewMatrix * ModelM * glm::vec4(GlobalBSurface.Calculate(u, v), 1.f);
			tmpPoint /= tmpPoint.z;
			glColor3f(0.4, 0.8, 1.f);
			glBegin(GL_LINES);
			glVertex2f(WorkingPoint.x, WorkingPoint.y);
			glVertex2f(tmpPoint.x, tmpPoint.y);
			//glVertex3f(WorkingPoint.x, WorkingPoint.y, WorkingPoint.z);
			//glVertex3f(tmpPoint.x, tmpPoint.y, tmpPoint.z);
			glEnd();
			WorkingPoint = tmpPoint;
		}
	}
	for (float v = 0; v <= 1.f; v += 0.05f)
	{
		glm::vec4 WorkingPoint = ProjectionMatrix *ViewMatrix * ModelM * glm::vec4(GlobalBSurface.Calculate(0.f, v), 1.f);
		WorkingPoint /= WorkingPoint.z;
		for (float u = 0.05f; u <= 1.f; u += 0.05f)
		{
			glm::vec4 tmpPoint = ProjectionMatrix * ViewMatrix * ModelM * glm::vec4(GlobalBSurface.Calculate(u, v), 1.f);
			tmpPoint /= tmpPoint.z;
			glColor3f(0.4, 0.8, 1.f);
			glBegin(GL_LINES);
			glVertex2f(WorkingPoint.x, WorkingPoint.y);
			glVertex2f(tmpPoint.x, tmpPoint.y);
			//glVertex3f(WorkingPoint.x, WorkingPoint.y, WorkingPoint.z);
			//glVertex3f(tmpPoint.x, tmpPoint.y, tmpPoint.z);
			glEnd();
			WorkingPoint = tmpPoint;
		}
	}
	//Draw Control Point
	glLineWidth(2);
	for (int y = 0; y < GlobalBSurface.SizeV; y++)
	{
		glm::vec4 WorkingPoint = ProjectionMatrix * ViewMatrix * ModelM * glm::vec4(GlobalBSurface.ControlPoints[y * GlobalBSurface.SizeU], 1.f);
		WorkingPoint /= WorkingPoint.z;
		for (int x = 1; x < GlobalBSurface.SizeU; x++)
		{
			glm::vec4 tmpPoint = ProjectionMatrix * ViewMatrix * ModelM * glm::vec4(GlobalBSurface.ControlPoints[y * GlobalBSurface.SizeU+x], 1.f);
			tmpPoint /= tmpPoint.z;
			glColor3f(1.f, 0.8, 0.4f);
			glBegin(GL_LINES);
			glVertex2f(WorkingPoint.x, WorkingPoint.y);
			glVertex2f(tmpPoint.x, tmpPoint.y);
			glEnd();
			WorkingPoint = tmpPoint;
		}
	}
	for (int x = 0; x < GlobalBSurface.SizeU; x++)
	{
		glm::vec4 WorkingPoint = ProjectionMatrix * ViewMatrix * ModelM * glm::vec4(GlobalBSurface.ControlPoints[x], 1.f);
		WorkingPoint /= WorkingPoint.z;
		for (int y = 1; y < GlobalBSurface.SizeV; y++)
		{
			glm::vec4 tmpPoint = ProjectionMatrix * ViewMatrix * ModelM * glm::vec4(GlobalBSurface.ControlPoints[y * GlobalBSurface.SizeU + x], 1.f);
			tmpPoint /= tmpPoint.z;
			glColor3f(1.f, 0.8, 0.4f);
			glBegin(GL_LINES);
			glVertex2f(WorkingPoint.x, WorkingPoint.y);
			glVertex2f(tmpPoint.x, tmpPoint.y);
			glEnd();
			WorkingPoint = tmpPoint;
		}
	}
	glFlush();
}