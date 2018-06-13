#pragma once
#include "glfw3.h"
#include "glm/glm.hpp"
#include "Camera.h"

class GLFWMainWindow
{
	GLFWwindow* pWindow=nullptr;
	static Camera* AttachedCamera;
	static glm::vec2 MouseLastPos;
public:
	GLFWMainWindow(int width=800,int height=600,const char* title="Hello OpenGL!",
		GLFWframebuffersizefun FramebufferSizeCallback=nullptr, GLFWscrollfun ScroolCallback=nullptr, GLFWcursorposfun CursorPosCallback=nullptr);
	GLFWwindow* GetWindow();
	void Resize(int width, int height);
	void Move(int xoffset, int yoffset);
	void MoveTo(int x, int y);
	glm::vec2 GetWindowPos();
	glm::vec2 GetWindowSize();

	//Bind camera For Default Callback function
	static void AttachCamera(Camera* cameraToAttach);
	virtual ~GLFWMainWindow();
};