#include <windows.h>
#include "glad/glad.h"
#include "GLFW/glfw3.h"
#include "glm/glm.hpp"
#include <stdio.h>
#include "GLFWMainWindow.h"
#include "Camera.h"
#include <vector>
#include "Model.h"
#include "Light.h"
#include <time.h>
#include <glm/gtc/type_ptr.hpp>
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"assimp-vc140-mt.lib")

void TryRender();
void ProcessSceneMovement();
void BuildScene();
void LoadShaders();
GLuint BuildBox(GLuint* pVBO = nullptr);
void ProcessInput(GLFWwindow* pWindow);

GLFWMainWindow* pMainWindow;
Camera* pMyCamera;
float moveSpeed = 10.f;
GLuint BoxVAO,WoodTexture;
FDirectionalLight DirLight;
Shader* DefaultPhong = nullptr;
std::vector<FTransform> BoxTransform;  //Include the boxes' Transform
GLuint MatricesUniformBuffer;
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
#ifdef _DEBUG
	AllocConsole();
	FILE* tmpDebugFile;
	freopen_s(&tmpDebugFile, "CONOUT$", "w", stdout);
#endif
	srand((unsigned int)time(NULL));
	//Init OpenGL
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_SAMPLES, 4);
	pMainWindow = new GLFWMainWindow(1024, 768);
	pMyCamera = new Camera(4.f / 3.f);
	glfwSetInputMode(pMainWindow->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	pMainWindow->AttachCamera(pMyCamera);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_MULTISAMPLE);
	LoadShaders();
	BuildScene();
	while (!glfwWindowShouldClose(pMainWindow->GetWindow()))
	{
		ProcessInput(pMainWindow->GetWindow());
		ProcessSceneMovement();
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		TryRender();
		glfwSwapBuffers(pMainWindow->GetWindow());
		glfwPollEvents();
	}
}

void ProcessInput(GLFWwindow* pWindow)
{
	if (glfwGetKey(pWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(pWindow, GL_TRUE);
	if (glfwGetKey(pWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		moveSpeed = 100.f;
	else
		moveSpeed = 10.f;
	if (glfwGetKey(pWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
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

void LoadShaders()
{
	DefaultPhong = new Shader("VertexShader.vert", "LightedObjShader.glsl");
	DefaultPhong->BindUniformBlockToIndex("Matrices", 0);
	glGenBuffers(1, &MatricesUniformBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, MatricesUniformBuffer);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, MatricesUniformBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, MatricesUniformBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(pMyCamera->GetProjectionMatrix()));
	glBindBuffer(GL_UNIFORM_BUFFER, NULL);
}

GLuint BuildBox(GLuint* pVBO/*=nullptr*/)
{
	float vertices[] = {
		//坐标(XYZ),贴图坐标(XY),面法线(XYZ)
		-50.f, -50.f, -50.f,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
		50.f, -50.f, -50.f,  1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
		50.f,  50.f, -50.f,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
		50.f,  50.f, -50.f,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
		-50.f,  50.f, -50.f,  0.0f, 1.0f,  0.0f,  0.0f, -1.0f,
		-50.f, -50.f, -50.f,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,

		-50.f, -50.f,  50.f,  0.0f, 0.0f,  0.0f,  0.0f, 1.0f,
		50.f, -50.f,  50.f,  1.0f, 0.0f,  0.0f,  0.0f, 1.0f,
		50.f,  50.f,  50.f,  1.0f, 1.0f,  0.0f,  0.0f, 1.0f,
		50.f,  50.f,  50.f,  1.0f, 1.0f,  0.0f,  0.0f, 1.0f,
		-50.f,  50.f,  50.f,  0.0f, 1.0f,  0.0f,  0.0f, 1.0f,
		-50.f, -50.f,  50.f,  0.0f, 0.0f,  0.0f,  0.0f, 1.0f,

		-50.f,  50.f,  50.f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
		-50.f,  50.f, -50.f,  1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
		-50.f, -50.f, -50.f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
		-50.f, -50.f, -50.f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
		-50.f, -50.f,  50.f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
		-50.f,  50.f,  50.f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,

		50.f,  50.f,  50.f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
		50.f,  50.f, -50.f,  1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
		50.f, -50.f, -50.f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
		50.f, -50.f, -50.f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
		50.f, -50.f,  50.f,  0.0f, 0.0f,  1.0f,  0.0f,  0.0f,
		50.f,  50.f,  50.f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,

		-50.f, -50.f, -50.f,  0.0f, 1.0f,  0.0f, -1.0f,  0.0f,
		50.f, -50.f, -50.f,  1.0f, 1.0f,  0.0f, -1.0f,  0.0f,
		50.f, -50.f,  50.f,  1.0f, 0.0f,  0.0f, -1.0f,  0.0f,
		50.f, -50.f,  50.f,  1.0f, 0.0f,  0.0f, -1.0f,  0.0f,
		-50.f, -50.f,  50.f,  0.0f, 0.0f,  0.0f, -1.0f,  0.0f,
		-50.f, -50.f, -50.f,  0.0f, 1.0f,  0.0f, -1.0f,  0.0f,

		-50.f,  50.f, -50.f,  0.0f, 1.0f,  0.0f,  1.0f,  0.0f,
		50.f,  50.f, -50.f,  1.0f, 1.0f,  0.0f,  1.0f,  0.0f,
		50.f,  50.f,  50.f,  1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
		50.f,  50.f,  50.f,  1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
		-50.f,  50.f,  50.f,  0.0f, 0.0f,  0.0f,  1.0f,  0.0f,
		-50.f,  50.f, -50.f,  0.0f, 1.0f,  0.0f,  1.0f,  0.0f
	};
	GLuint VAO, VBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0); //Vertices Position;
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(5 * sizeof(float))); //Surface Normal Vector
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float))); //Texture Coodination
	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glEnableVertexAttribArray(2);
	glBindVertexArray(NULL);
	if (pVBO)
		*pVBO = VBO;
	return VAO;
}

void BuildScene()
{
	BoxVAO = BuildBox();
	DirLight.dir = glm::vec3(0.5, -0.5, 0.5);
	WoodTexture = LoadTexture("wood.jpg");
	FTransform tmpTransform;
	//Make floor
	tmpTransform.Location = glm::vec3(0, -50, 0); 
	tmpTransform.Scale = glm::vec3(8,0.1,8);
	BoxTransform.push_back(tmpTransform);
	//Make Box (On Surface)
	tmpTransform.Location = glm::vec3(-100, 0, -100);
	tmpTransform.Scale = glm::vec3(1, 1, 1);
	BoxTransform.push_back(tmpTransform);
	//Make Box (Float)
	tmpTransform.Location = glm::vec3(100, 50, 0);
	tmpTransform.Rotation = glm::vec3(rand() % 360, rand() % 360, rand() % 360);
	BoxTransform.push_back(tmpTransform);
}

void ProcessSceneMovement()
{

}

void TryRender()
{
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glm::mat4x4 ViewMatrix = pMyCamera->GetViewMatrix();
	glm::mat4x4 ProjectionMatrix = pMyCamera->GetProjectionMatrix();
	glBindBuffer(GL_UNIFORM_BUFFER, MatricesUniformBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(pMyCamera->GetProjectionMatrix()));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(ViewMatrix));
	DefaultPhong->Use();
	DefaultPhong->SetInt("UseDiffuseMap", GL_TRUE);
	DefaultPhong->SetInt("UseSkyboxRefractionAsDiffuseMap", GL_FALSE);
	DefaultPhong->SetInt("UseSkyboxReflectionAsDiffuseMap", GL_FALSE);
	DefaultPhong->SetInt("UseSpecluarMap", GL_FALSE);
	DefaultPhong->SetInt("UseSkyboxReflection", GL_FALSE);
	DefaultPhong->SetInt("UseDepthVisualization", GL_FALSE);
	DefaultPhong->SetFloat("shininess", 32);
	DefaultPhong->SetMatrix3x3("VectorMatrix", glm::transpose(glm::inverse(ViewMatrix)));
	DefaultPhong->SetMatrix3x3("NormalMatrix", glm::transpose(glm::inverse(ViewMatrix)));
	DefaultPhong->SetVec3("ambientColor", glm::vec3(1, 1, 1));
	DirLight.ApplyToShader(DefaultPhong, "DirectionalLight");
	DefaultPhong->SetInt("DiffuseMap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, WoodTexture);
	glBindVertexArray(BoxVAO);
	for (FTransform BoxT:BoxTransform)
	{
		DefaultPhong->SetMatrix4x4("ModelMatrix", BoxT.GenModelMatrix());
		DefaultPhong->SetMatrix3x3("NormalMatrix", glm::transpose(glm::inverse(ViewMatrix*BoxT.GenModelMatrix())));
		//glBufferSubData(GL_UNIFORM_BUFFER, 2*sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(BoxT.GenModelMatrix()));
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}