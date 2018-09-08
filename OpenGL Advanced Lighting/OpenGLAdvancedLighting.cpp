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
#include "glm/gtc/matrix_transform.hpp"
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"assimp-vc140-mt.lib")

#define SHADOWMAP_WIDTH 1024
#define SHADOWMAP_HEIGHT 1024

void TryRender();
void PreRender();
void DrawDebugDepth();
void ConfigShaderAndLightTransform();
void ProcessSceneMovement();
void BuildScene();
void LoadShaders();
void GenFrameBuffers(); 
GLuint BuildBox(GLuint* pVBO = nullptr);
void ProcessInput(GLFWwindow* pWindow);
GLuint BuildSurface();

void RenderQuad();

GLFWMainWindow* pMainWindow;
Camera* pMyCamera;
float moveSpeed = 10.f;
GLuint ShadowbufferFBO;
GLuint BoxVAO,WoodTexture,SurfaceVAO;
GLuint ShadowMap;
FDirectionalLight DirLight;
Shader* DefaultPhong = nullptr,*simpleDepthShader =nullptr,*simpleRenderTexture=nullptr;
std::vector<FTransform> BoxTransform;  //Include the boxes' Transform
GLuint MatricesUniformBuffer;
glm::mat4 LightSpaceTransformMat;
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
	pMyCamera->SetNearPanel(1.f);
	pMyCamera->SetFarPanel(10000.f);
	glfwSetInputMode(pMainWindow->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	pMainWindow->AttachCamera(pMyCamera);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glEnable(GL_MULTISAMPLE);
	LoadShaders();
	BuildScene();
	GenFrameBuffers();
	while (!glfwWindowShouldClose(pMainWindow->GetWindow()))
	{
		ProcessInput(pMainWindow->GetWindow());
		ProcessSceneMovement();
		glClearColor(1, 1, 1, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		PreRender();
		TryRender();
		//DrawDebugDepth();
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

	//Light Shader
	simpleDepthShader = new Shader("simpleDepthShader.vert", "emptyShader.glsl");

	//Debug Shader
	simpleRenderTexture = new Shader("simpleRenderTexture.vert", "simpleRenderTexture.glsl");
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
	SurfaceVAO = BuildSurface();
	DirLight.dir = glm::vec3(0.5, -0.5, 0.5);
	WoodTexture = LoadTexture("wood01.jpg");
	FTransform tmpTransform;
	//Make floor
	tmpTransform.Location = glm::vec3(0, -50, 0); 
	tmpTransform.Scale = glm::vec3(10,0.1,10);
	BoxTransform.push_back(tmpTransform);
	//Make Box (On Surface)
	tmpTransform.Location = glm::vec3(-100, 0, -100);
	tmpTransform.Scale = glm::vec3(1, 1, 1);
	BoxTransform.push_back(tmpTransform);
	//Make Box (Float with out rot)
	tmpTransform.Location = glm::vec3(-100, 100, 200);
	tmpTransform.Scale = glm::vec3(1, 1, 1);
	BoxTransform.push_back(tmpTransform);
	//Make Box (0,0,0)
	tmpTransform.Location = glm::vec3(0, 0, 0);
	tmpTransform.Scale = glm::vec3(1, 1, 1);
	BoxTransform.push_back(tmpTransform);
	//Make Box (Float)
	tmpTransform.Location = glm::vec3(100, 50, 0);
	tmpTransform.Rotation = glm::vec3(rand() % 360, rand() % 360, rand() % 360);
	BoxTransform.push_back(tmpTransform);
}

void GenFrameBuffers()
{
	glGenFramebuffers(1, &ShadowbufferFBO);
	glGenTextures(1, &ShadowMap);
	glBindTexture(GL_TEXTURE_2D, ShadowMap);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glBindFramebuffer(GL_FRAMEBUFFER, ShadowbufferFBO);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, ShadowMap, 0);
	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);
	glBindFramebuffer(GL_FRAMEBUFFER, NULL);
}

void ProcessSceneMovement()
{

}

void PreRender()
{
	glEnable(GL_DEPTH_TEST);
	glViewport(0, 0, SHADOWMAP_WIDTH, SHADOWMAP_HEIGHT);
	glBindFramebuffer(GL_FRAMEBUFFER, ShadowbufferFBO);
	glClear(GL_DEPTH_BUFFER_BIT);
	ConfigShaderAndLightTransform();
	//Render Only Box
	glBindVertexArray(BoxVAO);
	for (FTransform BoxT : BoxTransform)
	{
		simpleDepthShader->SetMatrix4x4("model", BoxT.GenModelMatrix());
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
	glm::vec2 CurWindowSize = pMainWindow->GetWindowSize();
	glViewport(0, 0, (int)CurWindowSize.r, (int)CurWindowSize.g);
}

void ConfigShaderAndLightTransform()
{
	glm::mat4 LightProjection = glm::ortho(-500.f, 500.f, -500.f, 500.f, 1.f, 20000.f);
	glm::mat4 LightView = glm::lookAt(-(DirLight.dir*700.f), glm::vec3(0, 0, 0), glm::vec3(0,1.0,0));
	glm::mat4x4 ViewMatrix = pMyCamera->GetViewMatrix();
	glm::mat4x4 ProjectionMatrix = pMyCamera->GetProjectionMatrix();
	//LightSpaceTransformMat = ProjectionMatrix * ViewMatrix;
	LightSpaceTransformMat = LightProjection * LightView;
	simpleDepthShader->Use();
	simpleDepthShader->SetMatrix4x4("lightSpaceMatrix", LightSpaceTransformMat);
}

void TryRender()
{
	glEnable(GL_DEPTH_TEST);
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
	DefaultPhong->SetMatrix4x4("lightSpaceMatrix", LightSpaceTransformMat);
	DefaultPhong->SetVec3("ambientColor", glm::vec3(1, 1, 1));
	DirLight.ApplyToShader(DefaultPhong, "DirectionalLight");
	DefaultPhong->SetInt("DiffuseMap", 0);
	DefaultPhong->SetInt("ShadowMap", 3);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, WoodTexture);
	//Bind Shadow Map
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, ShadowMap);
	glBindVertexArray(BoxVAO);
	for (FTransform BoxT:BoxTransform)
	{
		DefaultPhong->SetMatrix4x4("ModelMatrix", BoxT.GenModelMatrix());
		DefaultPhong->SetMatrix3x3("NormalMatrix", glm::transpose(glm::inverse(ViewMatrix*BoxT.GenModelMatrix())));
		//glBufferSubData(GL_UNIFORM_BUFFER, 2*sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(BoxT.GenModelMatrix()));
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}
}

void DrawDebugDepth()
{
	simpleRenderTexture->Use();
	simpleRenderTexture->SetInt("depthMap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, ShadowMap);
	RenderQuad();
}

void RenderQuad()
{
	glDisable(GL_DEPTH_TEST);
	glBindVertexArray(SurfaceVAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL);
	glBindVertexArray(0);
}

GLuint BuildSurface()
{
	float vertices[] =
	{
		0,0,0,0,0,
		1,0,0,1,0,
		1,1,0,1,1,
		0,1,0,0,1
	};
	unsigned int indices[] =
	{
		0,1,2,0,2,3
	};
	GLuint VAO, VBO, EBO;
	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindVertexArray(NULL);
	return VAO;
}