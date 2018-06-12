#include "glad/glad.h"
#include "GLFWMainWindow.h"
#include <stdio.h>

#define MOUSE_SPEED 0.2f

Camera* GLFWMainWindow::AttachedCamera;

glm::vec2 GLFWMainWindow::MouseLastPos;

GLFWMainWindow::GLFWMainWindow(int width, int height, const char* title, 
	GLFWframebuffersizefun FramebufferSizeCallback, GLFWscrollfun ScroolCallback, GLFWcursorposfun CursorPosCallback)
{
	pWindow = glfwCreateWindow(width, height, title, nullptr, nullptr);
	glfwMakeContextCurrent(pWindow);
	gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
	glViewport(0, 0, width, height);
	MouseLastPos = glm::vec2(width*0.5f, height*0.5f);
	glfwSetFramebufferSizeCallback(pWindow, FramebufferSizeCallback?FramebufferSizeCallback:[](GLFWwindow* p, int newWidth, int newHeight)
		{
			glViewport(0, 0, newWidth, newHeight);
			if (AttachedCamera)
			{
				AttachedCamera->SetAspect((float)newWidth / newHeight);
			}
		});
	glfwSetScrollCallback(pWindow, ScroolCallback?ScroolCallback:[](GLFWwindow* p, double X, double Y)
		{
			if (AttachedCamera)
			{
				AttachedCamera->SetFOV(AttachedCamera->GetFOV() + Y);
				printf("FOV Changed To:%0.1f\n", AttachedCamera->GetFOV());
			}
		});
	glfwSetCursorPosCallback(pWindow, CursorPosCallback?CursorPosCallback:[](GLFWwindow* p, double xpos, double ypos)
		{
			if (AttachedCamera)
			{
				AttachedCamera->Rotate(glm::vec3(0, MOUSE_SPEED*(MouseLastPos.y - ypos), MOUSE_SPEED*(MouseLastPos.x - xpos)));
				MouseLastPos.x = xpos;
				MouseLastPos.y = ypos;
			}
			
		});
}

GLFWwindow* GLFWMainWindow::GetWindow()
{
	return pWindow;
}

void GLFWMainWindow::Resize(int width, int height)
{
	glfwSetWindowSize(GetWindow(), width, height);
	printf("WindowSize Changed To %dx%d", width, height);
}

void GLFWMainWindow::Move(int xoffset, int yoffset)
{
	glm::vec2 tmpPos = GetWindowPos();
	MoveTo(tmpPos.x + xoffset, tmpPos.y + yoffset);
}

void GLFWMainWindow::MoveTo(int x, int y)
{
	glfwSetWindowPos(GetWindow(),x, y);
	printf("Window Moved To %dx%d", x, y);
}

glm::vec2 GLFWMainWindow::GetWindowPos()
{
	int xpos,ypos;
	glfwGetWindowPos(GetWindow(), &xpos, &ypos);
	return glm::vec2(xpos, ypos);
}

glm::vec2 GLFWMainWindow::GetWindowSize()
{
	int width, height;
	glfwGetWindowSize(GetWindow(), &width, &height);
	return glm::vec2(width, height);
}

void GLFWMainWindow::AttachCamera(Camera* cameraToAttach)
{
	AttachedCamera = cameraToAttach;
}

GLFWMainWindow::~GLFWMainWindow()
{
	glfwDestroyWindow(pWindow);
}