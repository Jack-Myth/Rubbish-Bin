#include "windows.h"
#include "glad/glad.h"
#include "glfw3.h"
#include "glm/glm.hpp"
#include "GLFWMainWindow.h"
#include "Camera.h"
#include "Shader.h"
#include "Light.h"
#include <time.h>
#include <Model.h>
#include <glm/gtc/matrix_transform.hpp>
#include <map>
#include <../Hello OpenGL/stb_image.h>
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"assimp-vc140-mt.lib")

void TryRender();
void BuildScene();
void ObjectAutomove();
GLuint BuildNewBox(GLuint* pVBO = nullptr);
GLuint BuildSurface();

void ProcessInput(GLFWwindow* pWindow);

#define ROCKS_COUNT 81920
#define ROCKS_RANGE 50
#define ROCKS_WIDTH 30

float moveSpeed = 1.f;
GLFWMainWindow* pMainWindow = nullptr;
Camera* pMyCamera = nullptr;
Shader* DefaultPhong = nullptr,*Unlit=nullptr,*TestforFramebuffer=nullptr, *SkyBoxShader=nullptr,*GeoShader=nullptr,*ExplodeShader=nullptr,*PlanetShader=nullptr;
Model* targetModel=nullptr,*Planet=nullptr,*Rock=nullptr;
glm::mat4x4 RocksTranformMatrix[ROCKS_COUNT];
FPointLight Pointlight;
FDirectionalLight DirLight;
FSpotlight Flashlight;
GLuint BoxVAO,SurfaceVAO,TestGeoVAO;
FTransform ModelTransform[7];
GLuint StoneTexture,GrassTexture,WindowTexture, texColorBuffer,CubeMap,PlanetTexture,RockTexture;
GLuint FBO;
glm::mat4x4 RocksRotM(1),RocksOffsetM(1);
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
	pMyCamera = new Camera(4.f/3.f);
	glfwSetInputMode(pMainWindow->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	pMainWindow->AttachCamera(pMyCamera);

	//Compile Shaders;
	DefaultPhong = new Shader("VertexShader.vert", "LightedObjShader.glsl","DefaultPhong");
	Unlit = new Shader("VertexShader.vert", "LightShader.glsl", "Unlit");
	TestforFramebuffer = new Shader("FrameBuffer_VertexShader.vert", "FrameBuffer_FragShader.glsl", "FrameBuffer");
	SkyBoxShader = new Shader("CubeMap.vert", "CubeMap.glsl","CubeMap");
	GeoShader = new Shader();
	GeoShader->AttachShader(GL_VERTEX_SHADER, "GeoShader.vert");
	GeoShader->AttachShader(GL_GEOMETRY_SHADER, "GeoShader.geom");
	GeoShader->AttachShader(GL_FRAGMENT_SHADER, "GeoShader.glsl");
	GeoShader->Link();
	ExplodeShader = new Shader();
	ExplodeShader->AttachShader(GL_VERTEX_SHADER, "VertexShader.vert");
	if (!ExplodeShader->AttachShader(GL_GEOMETRY_SHADER, "Explode.geom"))
	{
		printf("Error While Compiling Exploded.geom\nErrorMsg:%s", ExplodeShader->GetLastError().c_str());
	}
	ExplodeShader->AttachShader(GL_FRAGMENT_SHADER, "LightedObjShader.glsl");
	ExplodeShader->Link();
	PlanetShader=new Shader("RocksShader.vert", "LightedObjShader.glsl");

	//Load Texture
	StoneTexture = LoadTexture("stone.jpg");
	GrassTexture = LoadTexture("grass.png");
	WindowTexture = LoadTexture("blending_transparent_window.png");
	BuildScene();
	BoxVAO = BuildNewBox(nullptr);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	//glEnable(GL_CULL_FACE);
	//glDepthMask(GL_FALSE);
	
	while(!glfwWindowShouldClose(pMainWindow->GetWindow()))
	{
		ProcessInput(pMainWindow->GetWindow());
		ObjectAutomove();
		glBindFramebuffer(GL_FRAMEBUFFER, FBO);
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		glEnable(GL_DEPTH_TEST);
		TryRender();
		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		glDisable(GL_DEPTH_TEST);
		glBindVertexArray(SurfaceVAO);
		//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		TestforFramebuffer->Use();
		TestforFramebuffer->SetInt("Tex0", 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, texColorBuffer);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
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
		pMyCamera->Move(glm::vec3(0, 0, -0.2f)*moveSpeed);
	if (glfwGetKey(pWindow, GLFW_KEY_S) == GLFW_PRESS)
		pMyCamera->Move(glm::vec3(0, 0, 0.2f)*moveSpeed);
	if (glfwGetKey(pWindow, GLFW_KEY_A) == GLFW_PRESS)
		pMyCamera->Move(glm::vec3(-0.2f, 0, 0)*moveSpeed);
	if (glfwGetKey(pWindow, GLFW_KEY_D) == GLFW_PRESS)
		pMyCamera->Move(glm::vec3(0.2f, 0, 0)*moveSpeed);
	if (glfwGetKey(pWindow, GLFW_KEY_X) == GLFW_PRESS)
		targetModel->Transform.Rotation.x++;
	if (glfwGetKey(pWindow, GLFW_KEY_Y) == GLFW_PRESS)
		targetModel->Transform.Rotation.y++;
	if (glfwGetKey(pWindow, GLFW_KEY_Z) == GLFW_PRESS)
		targetModel->Transform.Rotation.z++;
}

GLuint BuildNewBox(GLuint* pVBO/*=nullptr*/)
{
	float vertices[] = {
		//坐标(XYZ),贴图坐标(XY),面法线(XYZ)
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,
		0.5f, -0.5f, -0.5f,  1.0f, 0.0f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f,  0.0f, -1.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f,  0.0f, -1.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 0.0f,  0.0f,  0.0f, -1.0f,

		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f,  0.0f, 1.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f,  0.0f, 1.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 1.0f,  0.0f,  0.0f, 1.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 1.0f,  0.0f,  0.0f, 1.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f,  0.0f, 1.0f,

		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  1.0f, 1.0f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f, -1.0f,  0.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f, -1.0f,  0.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  1.0f, 0.0f, -1.0f,  0.0f,  0.0f,

		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  1.0f,  0.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  1.0f,  0.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  1.0f,  0.0f,  0.0f,

		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f, -0.5f,  1.0f, 1.0f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f,  0.0f,
		0.5f, -0.5f,  0.5f,  1.0f, 0.0f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f,  0.5f,  0.0f, 0.0f,  0.0f, -1.0f,  0.0f,
		-0.5f, -0.5f, -0.5f,  0.0f, 1.0f,  0.0f, -1.0f,  0.0f,

		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f, -0.5f,  1.0f, 1.0f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
		0.5f,  0.5f,  0.5f,  1.0f, 0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f,  0.5f,  0.0f, 0.0f,  0.0f,  1.0f,  0.0f,
		-0.5f,  0.5f, -0.5f,  0.0f, 1.0f,  0.0f,  1.0f,  0.0f
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

GLuint BuildSurface()
{
	float vertices[] =
	{
		0,0,0,0,0,1,0,0,
		1,0,0,0,0,1,1,0,
		1,1,0,0,0,1,1,1,
		0,1,0,0,0,1,0,1
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices),vertices,GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), 0);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
	glBindVertexArray(NULL);
	return VAO;
}

void BuildScene()
{
	//Load Cube map
	std::vector<std::string> textures_faces =
	{
		"CubeMap/cottoncandy_rt.tga",
		"CubeMap/cottoncandy_lf.tga",
		"CubeMap/cottoncandy_up.tga",
		"CubeMap/cottoncandy_dn.tga",
		"CubeMap/cottoncandy_bk.tga",
		"CubeMap/cottoncandy_ft.tga"
	};
	CubeMap = LoadCubeMap(textures_faces);

	//Load Planet and Rocket
	{
		Planet = Model::LoadMesh("Models/planet.obj");
		Rock = Model::LoadMesh("Models/rock.obj");
		Planet->Transform.Location = glm::vec3(50,50, 50);
		Planet->Transform.Rotation = glm::vec3(0, 0, 0);
		glm::vec3 RocksOffset = Planet->Transform.Location+glm::vec3(-15.f, -10.f, -120.f);
		//build rocks transform
		for (int i = 0; i < ROCKS_COUNT; i++)
		{
			FTransform tmpTransform;
			float TargetRadius = rand() % 360;
			tmpTransform.Location.x = glm::cos(TargetRadius)*ROCKS_RANGE +rand() % ROCKS_WIDTH;
			tmpTransform.Location.z = glm::sin(TargetRadius)*ROCKS_RANGE +rand() % ROCKS_WIDTH;
			tmpTransform.Location.y = rand() % ROCKS_WIDTH;
			tmpTransform.Scale = glm::vec3(0.1, 0.1, 0.1);
			RocksTranformMatrix[i]=tmpTransform.GenModelMatrix();
		}
		RocksOffsetM=glm::translate(RocksOffsetM, RocksOffset);
		GLuint ModelViewBuffer;
		glGenBuffers(1, &ModelViewBuffer);
		glBindBuffer(GL_ARRAY_BUFFER,ModelViewBuffer);
		glBufferData(GL_ARRAY_BUFFER, sizeof(RocksTranformMatrix), RocksTranformMatrix, GL_STATIC_DRAW);
		for (int i=0;i<Rock->GetMeshes().size();i++)
		{
			glBindVertexArray(Rock->GetMeshes()[i]->GetVAO());
			glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), 0);
			glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)sizeof(glm::vec4));
			glVertexAttribPointer(5, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(2 * sizeof(glm::vec4)));
			glVertexAttribPointer(6, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(glm::vec4), (void*)(3 * sizeof(glm::vec4)));
			glEnableVertexAttribArray(3);
			glEnableVertexAttribArray(4);
			glEnableVertexAttribArray(5);
			glEnableVertexAttribArray(6);
			glVertexAttribDivisor(3, 1);
			glVertexAttribDivisor(4, 1);
			glVertexAttribDivisor(5, 1);
			glVertexAttribDivisor(6, 1);
			glBindVertexArray(NULL);
		}
	}

	char FileP[1024] = {0};
	OPENFILENAMEA OpenFN = {NULL};
	OpenFN.lStructSize = sizeof(OPENFILENAMEA);
	OpenFN.Flags = OFN_FILEMUSTEXIST;
	OpenFN.lpstrFilter = "模型文件\0*.obj;*.fbx;*.3ds\0\0";
	OpenFN.nMaxFile = MAX_PATH;
	OpenFN.lpstrFile = FileP;
	OpenFN.hInstance = GetModuleHandle(NULL);
	OpenFN.lpstrTitle = "选择模型文件";
	if (GetOpenFileNameA(&OpenFN))
	{
		targetModel = Model::LoadMesh(FileP);
		if (targetModel)
		{
			targetModel->Transform.Scale = glm::vec3(0.1f, 0.1f, 0.1f);
			//targetModel->Transform.Rotation.z = 180.f;
		}
	}
	Pointlight.diffuse = glm::vec3(5, 5, 5);
	Pointlight.linear = 0.022f;
	Pointlight.quadratic = 0.0019f;
	ModelTransform[0].Location = glm::vec3(10.f, -5.f, 0);
	ModelTransform[0].Scale = glm::vec3(50, 1, 50);
	ModelTransform[1].Scale = glm::vec3(10.f, 10.f, 10.f);
	ModelTransform[2].Location = glm::vec3(15.f, 0.f, 0);
	ModelTransform[2].Scale = glm::vec3(10.f, 10.f, 10.f);
	
	//Grass Texture
	SurfaceVAO = BuildSurface();
	ModelTransform[3].Location = glm::vec3(0, -4.5f, 11);
	ModelTransform[3].Scale = glm::vec3(2, 2, 2);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBlendEquation(GL_FUNC_ADD);
	ModelTransform[4].Location = glm::vec3(0, 0, 11);
	ModelTransform[4].Scale = glm::vec3(5, 5, 5);
	ModelTransform[5].Location = glm::vec3(2, -1.f, 12);
	ModelTransform[5].Scale = glm::vec3(5, 5, 5);
	ModelTransform[6].Location = glm::vec3(-2, 1.f, 13);
	ModelTransform[6].Scale = glm::vec3(5, 5, 5);
	/*glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	GLuint tmptexture;
	glGenTextures(1, &tmptexture);
	glBindTexture(GL_TEXTURE_2D, tmptexture);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 768, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tmptexture, 0);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 1024, 768, 0, GL_DEPTH_STENCIL,GL_UNSIGNED_INT_24_8, nullptr);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, tmptexture, 0);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE)
	{
	}
	GLuint RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1024, 768);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);*/

	glGenFramebuffers(1, &FBO);
	glBindFramebuffer(GL_FRAMEBUFFER, FBO);
	glGenTextures(1, &texColorBuffer);
	glBindTexture(GL_TEXTURE_2D, texColorBuffer);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 1024, 768, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//glBindTexture(GL_TEXTURE_2D, NULL);
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, texColorBuffer, 0);
	GLuint RBO;
	glGenRenderbuffers(1, &RBO);
	glBindRenderbuffer(GL_RENDERBUFFER, RBO);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, 1024, 768);
	//glBindRenderbuffer(GL_RENDERBUFFER, NULL);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, RBO);
	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
	{
		printf("FrameBuffer Initalization Failed.\n");
	}
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	// Build TestGeoVAO
	{
		glGenVertexArrays(1, &TestGeoVAO);
		GLuint BufferID;
		glGenBuffers(1, &BufferID);
		glBindVertexArray(TestGeoVAO);
		float points[] = {
			-0.5f,  0.5f, 1.0f, 0.0f, 0.0f, // 左上
			0.5f,  0.5f, 0.0f, 1.0f, 0.0f, // 右上
			0.5f, -0.5f, 0.0f, 0.0f, 1.0f, // 右下
			-0.5f, -0.5f, 1.0f, 1.0f, 0.0f  // 左下
		};
		glBindBuffer(GL_ARRAY_BUFFER, BufferID);
		glBufferData(GL_ARRAY_BUFFER, sizeof(points), points, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), 0);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(2 * sizeof(float)));
		glEnableVertexAttribArray(0);
		glEnableVertexAttribArray(1);
		glBindVertexArray(NULL);
	}
}

void ObjectAutomove()
{

}

void TryRender()
{
	glm::mat4x4 ViewMatrix = pMyCamera->GetViewMatrix();
	glm::mat4x4 ProjectionMatrix = pMyCamera->GetProjectionMatrix();
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//SkyBox
	glDepthFunc(GL_LEQUAL);
	SkyBoxShader->Use();
	SkyBoxShader->SetMatrix4x4("ViewMatrix", ViewMatrix);
	SkyBoxShader->SetMatrix4x4("ProjectionMatrix", ProjectionMatrix);
	glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMap);
	glBindVertexArray(BoxVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);

	glDepthMask(GL_TRUE);
	DefaultPhong->Use();
	DefaultPhong->SetMatrix4x4("ViewMatrix", ViewMatrix);
	DefaultPhong->SetMatrix4x4("ProjectionMatrix", ProjectionMatrix);
	DefaultPhong->SetFloat("shininess", 32);
	DefaultPhong->SetMatrix3x3("VectorMatrix", glm::transpose(glm::inverse(ViewMatrix)));
	DefaultPhong->SetMatrix4x4("ModelMatrix", glm::mat4x4(1.f));
	DefaultPhong->SetMatrix3x3("NormalMatrix", glm::transpose(glm::inverse(ViewMatrix)));
	DefaultPhong->SetVec3("ambientColor", glm::vec3(1, 1, 1));
	Pointlight.pos = ViewMatrix * glm::vec4(Pointlight.pos, 1.f);
	Pointlight.ApplyToShader(DefaultPhong, "PointLight[0]");
	DirLight.ApplyToShader(DefaultPhong, "DirectionalLight");
	Flashlight.ApplyToShader(DefaultPhong, "FlashLight");
	DefaultPhong->SetInt("UseDiffuseMap", GL_TRUE);
	DefaultPhong->SetInt("DiffuseMap", 0);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, StoneTexture);
	DefaultPhong->SetInt("UseSkyboxReflection", GL_TRUE);
	DefaultPhong->SetInt("Skybox", 2);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_CUBE_MAP, CubeMap);
	glBindVertexArray(BoxVAO);
	DefaultPhong->SetMatrix4x4("ModelMatrix", glm::mat4x4(1.f));
	DefaultPhong->SetInt("UseDepthVisualization", GL_FALSE);
	//glDrawArrays(GL_TRIANGLES, 0, 36);
	for (int i = 0; i < 3; i++)
	{
		DefaultPhong->SetMatrix4x4("ModelMatrix", ModelTransform[i].GenModelMatrix());
		glDrawArrays(GL_TRIANGLES, 0, 36);
	}

	//Render Target Modle
	if (targetModel)
	{
		//Test Skybox Reflection
		DefaultPhong->Use();
		DefaultPhong->SetInt("UseSkyboxReflectionAsDiffuseMap", GL_FALSE);
		DefaultPhong->SetInt("UseSkyboxRefractionAsDiffuseMap", GL_TRUE);
		targetModel->Draw(ExplodeShader,false);
	}
	DefaultPhong->SetInt("UseSkyboxReflectionAsDiffuseMap", GL_FALSE);
	DefaultPhong->SetInt("UseSkyboxRefractionAsDiffuseMap", GL_FALSE);

	//Draw GeoShader's House
	/*glBindVertexArray(TestGeoVAO);
	GeoShader->Use();
	glDrawArrays(GL_POINTS, 0, 4);*/

	//Draw Planet and rocks
	{
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, RockTexture);
		Planet->Draw(DefaultPhong,false);

		PlanetShader->Use();
		PlanetShader->SetMatrix4x4("ViewMatrix", ViewMatrix);
		PlanetShader->SetMatrix4x4("ProjectionMatrix", ProjectionMatrix);
		PlanetShader->SetFloat("shininess", 32);
		PlanetShader->SetMatrix3x3("VectorMatrix", glm::transpose(glm::inverse(ViewMatrix)));
		//PlanetShader->SetMatrix4x4("ModelMatrix", glm::mat4x4(1.f));
		PlanetShader->SetMatrix3x3("NormalMatrix", glm::transpose(glm::inverse(ViewMatrix)));
		PlanetShader->SetVec3("ambientColor", glm::vec3(1, 1, 1));
		Pointlight.ApplyToShader(PlanetShader, "PointLight[0]");
		DirLight.ApplyToShader(PlanetShader, "DirectionalLight");
		Flashlight.ApplyToShader(PlanetShader, "FlashLight");
		PlanetShader->SetInt("UseDiffuseMap", GL_FALSE);
		PlanetShader->SetInt("DiffuseMap", 0);
		RocksRotM = glm::rotate(RocksRotM, glm::radians(1.f), glm::vec3(0, 1, 0));
		PlanetShader->SetMatrix4x4("RotMatrix", RocksRotM);
		PlanetShader->SetMatrix4x4("OffsetMatrix", RocksOffsetM);
		//PlanetShader->SetInt("UseDepthVisualization", GL_FALSE);
		for (int i=0;i<Rock->GetMeshes().size();i++)
		{
			glBindVertexArray(Rock->GetMeshes()[i]->GetVAO());
			glDrawElementsInstanced(GL_TRIANGLES, Rock->GetMeshes()[i]->indices.size(), GL_UNSIGNED_INT, nullptr, ROCKS_COUNT);
			glBindVertexArray(NULL);
		}
	}

	DefaultPhong->Use();
	glBindVertexArray(SurfaceVAO);

	//Draw Grass
	DefaultPhong->SetMatrix4x4("ModelMatrix", ModelTransform[3].GenModelMatrix());
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, GrassTexture);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr, 100);  //Draw Elements Instanced.
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

	//Draw Window
	glBindTexture(GL_TEXTURE_2D, WindowTexture);
	glDepthMask(GL_FALSE);
	std::map<float, FTransform*> sorted;
	for (unsigned int i = 4; i < 7; i++)
	{
		float distance = glm::length(pMyCamera->GetCameraLocation() - ModelTransform[i].Location);
		sorted[distance] = &ModelTransform[i];
	}
	for (auto it=sorted.rbegin(); it != sorted.rend(); ++it)
	{
		DefaultPhong->SetMatrix4x4("ModelMatrix", it->second->GenModelMatrix());
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	}

	glDepthMask(GL_TRUE);
}