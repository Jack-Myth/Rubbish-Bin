#define STB_IMAGE_IMPLEMENTATION
#include "glad/glad.h"
#include "GLFWMainWindow.h"
#include "Model.h"
#include "Shader.h"
#include <windows.h>
#include <commdlg.h>
#include "glm/gtc/type_ptr.hpp"
#pragma comment(lib,"glfw3.lib")
#pragma comment(lib,"opengl32.lib")
#pragma comment(lib,"assimp-vc140-mt.lib")

void BuildScene();
void LoadShader();
void RenderScene();
GLuint BuildNewBox(GLuint* pVBO = nullptr);
GLuint MatricesUniformBuffer;
GLFWMainWindow* pMainWindow = nullptr;
Camera* pCamera = nullptr;
Shader* HDRShader = nullptr, *PBRShader = nullptr;
GLuint HDRImage;
GLuint BoxVAO;
Model* TargetModel;
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
#if _DEBUG
	{
		AllocConsole();
		FILE* tmpDebugFile;
		freopen_s(&tmpDebugFile, "CONOUT$", "w", stdout);
	}
#endif
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	void ControlView(GLFWwindow* p, double xpos, double ypos);
	pMainWindow = new GLFWMainWindow(800,600,"Hello OpenGL PBR",nullptr,nullptr,&ControlView);
	pCamera = new Camera(800, 600, 75,glm::vec3(0,0,-100.f));
	pMainWindow->AttachCamera(pCamera);
	glfwSetInputMode(pMainWindow->GetWindow(), GLFW_CURSOR, GLFW_CURSOR_NORMAL);
	glEnable(GL_DEPTH_TEST);
	LoadShader();
	BuildScene();
	while (!glfwWindowShouldClose(pMainWindow->GetWindow()))
	{
		glClearColor(0, 0, 0, 1);
		glClear(GL_COLOR_BUFFER_BIT);
		RenderScene();
		glfwSwapBuffers(pMainWindow->GetWindow());
		glfwPollEvents();
	}
}

void ControlView(GLFWwindow* p, double xpos, double ypos)
{
	if (glfwGetMouseButton(p,GLFW_MOUSE_BUTTON_LEFT)==GLFW_PRESS&&TargetModel)
	{
		TargetModel->Transform.Rotation.g += xpos - GLFWMainWindow::MouseLastPos.x;
		TargetModel->Transform.Rotation.r += ypos - GLFWMainWindow::MouseLastPos.y;
	}
	if (glfwGetMouseButton(p,GLFW_MOUSE_BUTTON_RIGHT)==GLFW_PRESS)
	{
		glm::vec3 CameraLocation = pCamera->GetCameraLocation();
		CameraLocation.z += ypos - GLFWMainWindow::MouseLastPos.y;
		pCamera->SetCameraLocation(CameraLocation);
	}
	GLFWMainWindow::MouseLastPos.x = xpos;
	GLFWMainWindow::MouseLastPos.y = ypos;
}

void LoadShader()
{
	HDRShader = new Shader("HDRShader.vert", "HDRShader.glsl");
	PBRShader = new Shader("PBRShader.vert", "PBRShader.glsl");
	HDRShader->BindUniformBlockToIndex("Matrices", 0);
	PBRShader->BindUniformBlockToIndex("Matrices", 0);
	glGenBuffers(1, &MatricesUniformBuffer);
	glBindBuffer(GL_UNIFORM_BUFFER, MatricesUniformBuffer);
	glBufferData(GL_UNIFORM_BUFFER, 2 * sizeof(glm::mat4), nullptr, GL_STATIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	glBindBufferBase(GL_UNIFORM_BUFFER, 0, MatricesUniformBuffer);
	std::vector<std::string> textures_faces =
	{
		"CubeMap/velcor_rt.tga",
		"CubeMap/velcor_lf.tga",
		"CubeMap/velcor_up.tga",
		"CubeMap/velcor_dn.tga",
		"CubeMap/velcor_bk.tga",
		"CubeMap/velcor_ft.tga"
	};
	HDRImage = LoadCubeMap(textures_faces,{0,0,0,0,0,0});
	HDRShader->SetInt("HDRImage", 0);
}

void BuildScene()
{
	char FileP[1024] = { 0 };
	OPENFILENAMEA OpenFN = { NULL };
	OpenFN.lStructSize = sizeof(OPENFILENAMEA);
	OpenFN.Flags = OFN_FILEMUSTEXIST;
	OpenFN.lpstrFilter = "模型文件\0*.obj;*.fbx;*.3ds\0\0";
	OpenFN.nMaxFile = MAX_PATH;
	OpenFN.lpstrFile = FileP;
	OpenFN.hInstance = GetModuleHandle(NULL);
	OpenFN.lpstrTitle = "选择模型文件";
	if (GetOpenFileNameA(&OpenFN))
	{
		TargetModel = Model::LoadMesh(FileP);
		if (TargetModel)
		{
			TargetModel->Transform.Scale = glm::vec3(1.f,1.f,1.f);
			//targetModel->Transform.Rotation.z = 180.f;
		}
	}
	BoxVAO = BuildNewBox();
}

void RenderScene()
{
	glm::mat4 ViewMatrix = pCamera->GetViewMatrix();
	glm::mat4 ProjectionMatrix = pCamera->GetProjectionMatrix();
	//Fill Uniform Buffer
	glBindBuffer(GL_UNIFORM_BUFFER, MatricesUniformBuffer);
	glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), glm::value_ptr(ProjectionMatrix));
	glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), glm::value_ptr(ViewMatrix));
	glDisable(GL_DEPTH_TEST);
	HDRShader->Use();
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP,HDRImage);
	glBindVertexArray(BoxVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindTexture(GL_TEXTURE_CUBE_MAP, NULL);
	glEnable(GL_DEPTH_TEST);
	if (TargetModel)
	{
		PBRShader->Use();
		//TODO:Fill Property
		TargetModel->Draw(PBRShader, false);
	}
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