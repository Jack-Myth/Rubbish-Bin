#include "windows.h"
#include "glad/glad.h"
#include "glfw3.h"
#include "glm/glm.hpp"
#include "GLFWMainWindow.h"
#include "Camera.h"
#include "Shader.h"
#include <time.h>
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"assimp-vc140-mt.lib")

void TryRender();
void BuildScene();
void ObjectAutomove();

void ProcessInput(GLFWwindow* pWindow);

float moveSpeed = 1.f;
GLFWMainWindow* pMainWindow = nullptr;
Camera* pMyCamera = nullptr;
Shader* DefaultPhong = nullptr;
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
#ifdef _DEBUG
	AllocConsole();
	FILE* tmpDebugFile;
	freopen_s(&tmpDebugFile,"CONOUT$", "w", stdout);
#endif
	srand((unsigned int)time(NULL));
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	pMainWindow = new GLFWMainWindow(1024, 768);
	pMyCamera = new Camera(1024, 768);
	glfwSetInputMode(pMainWindow->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	pMainWindow->AttachCamera(pMyCamera);
	DefaultPhong = new Shader("VertexShader.vert", "LightedObjShader.glsl","DefaultPhong");
	BuildScene();
	while(!glfwWindowShouldClose(pMainWindow->GetWindow()))
	{
		ProcessInput(pMainWindow->GetWindow());
		ObjectAutomove();
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
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
	if (glfwGetKey(pWindow, GLFW_KEY_W) == GLFW_PRESS)
		pMyCamera->Move(glm::vec3(0, 0, -0.2f)*moveSpeed);
	if (glfwGetKey(pWindow, GLFW_KEY_S) == GLFW_PRESS)
		pMyCamera->Move(glm::vec3(0, 0, 0.2f)*moveSpeed);
	if (glfwGetKey(pWindow, GLFW_KEY_A) == GLFW_PRESS)
		pMyCamera->Move(glm::vec3(-0.2f, 0, 0)*moveSpeed);
	if (glfwGetKey(pWindow, GLFW_KEY_D) == GLFW_PRESS)
		pMyCamera->Move(glm::vec3(0.2f, 0, 0)*moveSpeed);
}

void BuildScene()
{

}

void ObjectAutomove()
{

}

void TryRender()
{

}