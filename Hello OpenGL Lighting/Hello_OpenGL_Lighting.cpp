#define STB_IMAGE_IMPLEMENTATION
#include <windows.h>
#include "glad/glad.h"
#include <glfw3.h>
#include "GLFWMainWindow.h"
#include "stb_image.h"
#include <vector>
#include "glm/glm.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "Shader.h"
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"opengl32.lib")

GLuint BuildABox(GLuint* pVBO = nullptr, GLuint* pEBO = nullptr);
glm::mat4x4 CreateRandModelMatrix();
GLuint LoadTexture(std::string ImagePath);
void TryRender();
void ProcessInput(GLFWwindow* pWindow);
GLuint TryBuiltLightVAO();
void BuildScene();

GLFWMainWindow* pMainWindow=nullptr;
Camera* pMyCamera=nullptr;
std::vector<GLuint> VAOCollection;
std::vector<glm::mat4x4> ModelMatrixs;
GLuint TextureBack = 0, TextureFront = 0;
Shader* DefaultShader=nullptr,*LightShader=nullptr, *LightedShader = nullptr;
GLuint BoxVBO,BoxEBO;
GLuint LightSourceVAO;
glm::mat4x4 LightTransform(1.f);
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
#ifdef _DEBUG
	AllocConsole();
	freopen("CONOUT$", "w", stdout);
#endif
	srand((unsigned int)hInstance);
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	pMainWindow = new GLFWMainWindow(800, 600, "Hello OpenGL Lighting!");
	glfwSetInputMode(pMainWindow->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	pMyCamera = new Camera(4.f / 3.f);
	pMainWindow->AttachCamera(pMyCamera);
	DefaultShader = new Shader("VertexShader.vert", "FragmentShader.glsl");
	LightShader = new Shader("VertexShader.vert", "LightShader.glsl");
	LightedShader = new Shader("VertexShader.vert", "LightedObjShader.glsl");
	BuildScene();
	TextureBack = LoadTexture("MyTextureBack.jpg");
	TextureFront = LoadTexture("MyTextureFront.jpg");
	glEnable(GL_DEPTH_TEST);
	//Render Loop
	while (!glfwWindowShouldClose(pMainWindow->GetWindow()))
	{
		ProcessInput(pMainWindow->GetWindow());
		//glClearColor(0.2f, 0.4f, 0.6f, 0.1f);
		glClearColor(0, 0, 0, 1.f);
		glClear(GL_COLOR_BUFFER_BIT);
		TryRender();
		glfwSwapBuffers(pMainWindow->GetWindow());
		glfwPollEvents();
	}
	exit(0);
}

void ProcessInput(GLFWwindow* pWindow)
{
	if (glfwGetKey(pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(pWindow, GL_TRUE);
	if (glfwGetKey(pWindow, GLFW_KEY_W) == GLFW_PRESS)
		pMyCamera->Move(glm::vec3(0, 0, -0.2f));
	if (glfwGetKey(pWindow, GLFW_KEY_S) == GLFW_PRESS)
		pMyCamera->Move(glm::vec3(0, 0, 0.2f));
	if (glfwGetKey(pWindow, GLFW_KEY_A) == GLFW_PRESS)
		pMyCamera->Move(glm::vec3(-0.2f, 0, 0));
	if (glfwGetKey(pWindow, GLFW_KEY_D) == GLFW_PRESS)
		pMyCamera->Move(glm::vec3(0.2f, 0, 0));
}

void BuildScene()
{
	VAOCollection.push_back(BuildABox(&BoxVBO,&BoxEBO));
	for (int i=0;i<9;i++)
	{
	VAOCollection.push_back(VAOCollection[0]);
	}
	ModelMatrixs.push_back(glm::mat4x4(1.f));
	for (int i = 0; i < 9; i++)
	{
		ModelMatrixs.push_back(CreateRandModelMatrix());
	}

	//Build Light
	LightSourceVAO=TryBuiltLightVAO();
	LightTransform = glm::translate(LightTransform, glm::vec3(0, 1.f, 5.f));
	LightTransform = glm::scale(LightTransform, glm::vec3(0.2f));
}

GLuint BuildABox(GLuint* pVBO/*=nullptr*/, GLuint* pEBO/*=nullptr*/)
{
	float vertexs[] =
	{
		1.f,1.f,-1.f,0,1.f,			//   1.......0
		-1.f,1.f,-1.f,1.f,1.f,		//  ..      ..
		-1.f,1.f,1.f,1.f,0,		    // . .     . .
		1.f,1.f,1.f,0.f,0.f,		//2.......3  .
		1.f,-1.f,-1.f,0,0,			//.  5.......4
		-1.f,-1.f,-1.f,1.f,0,		//. .     . .
		-1.f,-1.f,1.f,1.f,1.f,		//..      ..
		1.f,-1.f,1.f,0,1.f,			//6.......7
		1.f,1.f,-1.f,1.f,0,		//8(0)
		-1.f,1.f,-1.f,0,0,		//9(1)
		-1.f,1.f,1.f,0,1.f,		//10(2)
		1.f,1.f,1.f,1.f,1.f,	//11(3)
	};
	unsigned int elements[] =
	{
		0,1,2,0,2,3,
		9,10,6,9,6,5,
		//1,2,6,1,6,5,
		4,5,6,4,6,7,
		8,11,7,8,7,4,
		//0,3,7,0,7,4,
		0,1,5,0,5,4,
		3,2,6,3,6,7
	};
	GLuint VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertexs), vertexs, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(elements), elements, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3*sizeof(float)));
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(NULL);
	if (pVBO)
		*pVBO = VBO;
	if (pEBO)
		*pEBO = EBO;
	return VAO;
}

GLuint LoadTexture(std::string ImagePath)
{
	int x, y,channels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* imageData = stbi_load(ImagePath.c_str(), &x, &y, &channels, 0);
	if (!imageData)
	{
		printf("Load Image \"%s\" Failed!", ImagePath.c_str());
		return 0;
	}
	GLuint tmpTextureID = 0;
	glGenTextures(1, &tmpTextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, tmpTextureID);
	if (channels==3)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D,NULL);
	stbi_image_free(imageData);
	return tmpTextureID;
}

glm::mat4x4 CreateRandModelMatrix()
{
	glm::mat4x4 defMatrix(1.f);
	defMatrix = glm::rotate(defMatrix, glm::radians((float)(rand() % 360)), 
		glm::vec3(rand() / (float)INT_MAX, rand() / (float)INT_MAX, rand() / (float)INT_MAX));
	defMatrix = glm::translate(defMatrix, glm::vec3(rand() % 20 - 10, rand() % 20 - 10, rand() % 20 - 10));
	return defMatrix;
}

GLuint TryBuiltLightVAO()
{
	GLuint LightVAO;
	glGenVertexArrays(1, &LightVAO);
	glBindVertexArray(LightVAO);
	glBindBuffer(GL_ARRAY_BUFFER, BoxVBO);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, BoxEBO);
	glEnableVertexAttribArray(0);
	glBindVertexArray(NULL);
	return LightVAO;
}

void TryRender()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4x4 ViewMatrix = pMyCamera->GetViewMatrix();
	glm::mat4x4 ProjectionMatrix = pMyCamera->GetProjectionMatrix();
	/*DefaultShader->Use();
	DefaultShader->SetInt("TextureBack", 0);
	DefaultShader->SetInt("TextureFront", 1);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, TextureBack);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, TextureFront);
	DefaultShader->SetMatrix4x4("ViewMatrix", ViewMatrix);
	DefaultShader->SetMatrix4x4("ProjectionMatrix", ProjectionMatrix);
	for (int i=0;i<ModelMatrixs.size();i++)
	{
		int x = VAOCollection.size() > i ? i : VAOCollection.size() - 1;
		DefaultShader->SetMatrix4x4("ModelMatrix",ModelMatrixs[i]);
		glBindVertexArray(VAOCollection[x]);
		glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
	}*/
	LightShader->Use();
	glBindVertexArray(LightSourceVAO);
	LightShader->SetMatrix4x4("ViewMatrix", ViewMatrix);
	LightShader->SetMatrix4x4("ProjectionMatrix", ProjectionMatrix);
	LightShader->SetMatrix4x4("ModelMatrix", LightTransform);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
	LightedShader->Use();
	LightedShader->SetVec3("objectColor", glm::vec3(1.0f, 0.5f, 0.31f));
	LightedShader->SetVec3("lightColor", glm::vec3(1.0f, 1.f, 1.f));
	LightedShader->SetMatrix4x4("ViewMatrix", ViewMatrix);
	LightedShader->SetMatrix4x4("ProjectionMatrix", ProjectionMatrix);
	LightedShader->SetMatrix4x4("ModelMatrix", ModelMatrixs[0]);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(NULL);
}