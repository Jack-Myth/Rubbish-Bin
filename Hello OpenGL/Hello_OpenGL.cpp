#include <iostream>
#include <fstream>
#include <windows.h>
#include "glad/glad.h"
#include <glfw3.h>
#include "GlobalMacros.h"
#include <string>
#include <streambuf>  
#pragma comment(lib,"glfw3.lib")

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

void ProcessInput(GLFWwindow* window);
void TryCompileShader();
void TryBindVAO();
void TryBindEBO();
void TryRender(GLFWwindow* window);
void TryCreateTexture();

GLFWwindow* pMainWindow = nullptr;
GLuint ShaderProgram,VAO,VAO2,EBO;
GLuint MyTexture;
int RenderMod = 0;
bool IsSwitchRenderModPress = false;
int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd )
{
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	//glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
	pMainWindow = glfwCreateWindow(800, 600, "Hello OpenGL!", NULL, NULL);
	if (!pMainWindow)
	{
		SHOW_DEBUG_DIALOG("OpenGL Main Window Create Failed! Exiting...");
		glfwTerminate();
		return  -1;
	}
	glfwMakeContextCurrent(pMainWindow);
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		SHOW_DEBUG_DIALOG("Failed to initialize GLAD");
		return -1;
	}
	glViewport(0, 0, 800, 600);
	glfwSetFramebufferSizeCallback(pMainWindow, [](GLFWwindow* window, int width, int height)
								   {
									   glViewport(0, 0, width, height);
								   });
	TryCompileShader();
	TryBindEBO();
	TryBindVAO();
	TryCreateTexture();
	while (!glfwWindowShouldClose(pMainWindow))
	{
		ProcessInput(pMainWindow);
		glClearColor(0.4f, 0.8f, 1.f,1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		TryRender(pMainWindow);
		glfwSwapBuffers(pMainWindow);
		glfwPollEvents();
	}
	glfwTerminate();
	return 0;
}

void SwitchRenderMode()
{
	RenderMod = (RenderMod + 1) % 2;
}

void ProcessInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE)==GLFW_PRESS)
		glfwSetWindowShouldClose(window,GLFW_TRUE);
	int KeyStatus = glfwGetKey(window, GLFW_KEY_C);
	if (!IsSwitchRenderModPress&&KeyStatus == GLFW_PRESS)
	{
		SwitchRenderMode();
	}
	IsSwitchRenderModPress = glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE ? false : true;
}

void TryCompileShader()
{
	GLuint VertexShader, FragmentShader;
	VertexShader = glCreateShader(GL_VERTEX_SHADER);
	std::ifstream shaderStream("VertexShader.glsl");
	std::string ShaderSourceStr=std::string(std::istreambuf_iterator<char>(shaderStream), std::istreambuf_iterator<char>());
	const char* pssstr = ShaderSourceStr.c_str();
	glShaderSource(VertexShader, 1, &pssstr, nullptr);
	glCompileShader(VertexShader);
	GLint isCompileSuccess = 0;
	glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &isCompileSuccess);
	if (!isCompileSuccess)
	{
		char ErrorInfomationBuffer[512];
		glGetShaderInfoLog(VertexShader, 512, nullptr, ErrorInfomationBuffer);
		SHOW_DEBUG_DIALOG(ErrorInfomationBuffer);
		//exit(-1);
	}
	shaderStream = std::ifstream("FragmentShader.glsl");
	ShaderSourceStr= std::string(std::istreambuf_iterator<char>(shaderStream), std::istreambuf_iterator<char>());
	pssstr = ShaderSourceStr.c_str();
	FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(FragmentShader, 1, &pssstr, nullptr);
	glCompileShader(FragmentShader);
	glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &isCompileSuccess);
	if (!isCompileSuccess)
	{
		char ErrorInfomationBuffer[512];
		glGetShaderInfoLog(FragmentShader, 512, nullptr, ErrorInfomationBuffer);
		SHOW_DEBUG_DIALOG(ErrorInfomationBuffer);
		//exit(-1);
	}
	ShaderProgram = glCreateProgram();
	glAttachShader(ShaderProgram, VertexShader);
	glAttachShader(ShaderProgram, FragmentShader);
	glLinkProgram(ShaderProgram);
	glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &isCompileSuccess);
	if (!isCompileSuccess)
	{
		char ErrorInfomationBuffer[512];
		glGetProgramInfoLog(ShaderProgram, 512, nullptr, ErrorInfomationBuffer);
		SHOW_DEBUG_DIALOG(ErrorInfomationBuffer);
	}
	glUseProgram(ShaderProgram);
	glDeleteShader(VertexShader);
	glDeleteShader(FragmentShader);
}

void TryBindVAO()
{
	float vertices[] =
	{
		-0.5f,-0.5f,0.f, -0.2f,0.f,
		0.5f,-0.5f,0.f,1.2f,0.f,
		0.f,0.5f,0.f,0.5f,1.2f
	};
	GLuint VertexBufferOID;
	glGenBuffers(1, &VertexBufferOID);
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VertexBufferOID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(2);
	glBindVertexArray(NULL);
}

void TryBindEBO()
{
	glGenBuffers(1, &EBO);
	GLuint VBO2;
	glGenBuffers(1, &VBO2);
	float vertices[] = {
		0.5f,0.1f,0,   /*位置*/  0.7f,0.f,0.7f,1.f,/*顶点色*/
		0.3f,-0.7f,0, /*位置*/  0.f,1.f,0.f,1.f,/*顶点色*/
		-0.3f,-0.7f,0, /*位置*/  0.f,1.f,0.f,1.f,/*顶点色*/
		-0.5f,0.1,0, /*位置*/  0.5f,0.1f,0.7f,1.f,/*顶点色*/
		0,0.5f,0, /*位置*/  0.f,0.f,1.f,0.2f,/*顶点色*/
		-0.1f,0.1f,0, /*位置*/  0.5f,0.1f,0.7f,1.f,/*顶点色*/
		0.1f,0.1f,0, /*位置*/  0.5f,0.1f,0.7f,1.f/*顶点色*/
	};
	unsigned int indices[] = { // 注意索引从0开始! 
		0, 2, 5, // 第一个三角形
		3, 6, 1,  // 第二个三角形
		4,6,5//第三个三角形
	};
	glGenVertexArrays(1, &VAO2);
	glBindVertexArray(VAO2);
	glBindBuffer(GL_ARRAY_BUFFER, VBO2);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 7 * sizeof(float), (void*)(3 * sizeof(float)));
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 7 * sizeof(float), 0);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(NULL);
}

void TryCreateTexture()
{
	glGenTextures(1, &MyTexture);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, MyTexture);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	int width, height,channels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* MyTexturePixelData= stbi_load("MyTexture.jpg", &width, &height, &channels, 0);
	if (!MyTexturePixelData)
	{
		SHOW_DEBUG_DIALOG("Failed To Create Texture!");
		return;
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, MyTexturePixelData);
	//glGenerateMipmap(GL_TEXTURE_2D);
	stbi_image_free(MyTexturePixelData);
}

void TryRender(GLFWwindow* window)
{
	glUseProgram(ShaderProgram);
	float gTime = (float)glfwGetTime();
	glUniform1f(glGetUniformLocation(ShaderProgram, "DynamicColor"),gTime);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, MyTexture);
	glUniform1i(glGetUniformLocation(ShaderProgram, "MyTexture"), 0);
	switch (RenderMod)
	{
		case 0:
			glBindVertexArray(VAO);
			glDrawArrays(GL_TRIANGLES, 0, 3);
			break;
		case 1:
			glBindVertexArray(VAO2);
			//glDrawArrays(GL_TRIANGLES, 0, 3);
			glDrawElements(GL_TRIANGLES, 9, GL_UNSIGNED_INT, 0);
			break;
	}
}