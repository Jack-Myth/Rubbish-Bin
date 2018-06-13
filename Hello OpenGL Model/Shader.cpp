#include "Shader.h"
#include <fstream>
#include <streambuf>
#include "glm/gtc/type_ptr.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

std::vector<Shader*> Shader::ShaderList;

std::vector<Shader*> Shader::AllShaderRef;

Shader::Shader(std::string Name/*=""*/)
{
	this->Name = Name;
	ShaderProgramID = glCreateProgram();
	ShaderList.push_back(this);
}

Shader::Shader(std::string VertexShaderFilePath, std::string FragmentShaderFilePath, std::string Name/*=""*/) :Shader(Name)
{
	bool isSucceed = true;
	if (!AttachShader(GL_VERTEX_SHADER, VertexShaderFilePath))
	{
		printf("%s\n", GetLastError().c_str());
		isSucceed = false;
	}
	if (!AttachShader(GL_FRAGMENT_SHADER, FragmentShaderFilePath))
	{
		printf("%s\n", GetLastError().c_str());
		isSucceed = false;
	}
	if (!isSucceed)
	{
		printf("Compile Error!\n");
		return;
	}
	Link();
}

bool Shader::AttachShader(GLenum ShaderType, std::string ShaderFilePath)
{
	if (IsLinked())
	{
		LastError = "Shader Program has alreay been linked!";
		return false;
	}
	GLuint tmpShaderID=glCreateShader(ShaderType);
	std::ifstream tmpFileStream = std::ifstream(ShaderFilePath);
	if (!tmpFileStream.is_open())
	{
		LastError = "Unable To Open File:" + ShaderFilePath;
		return false;
	}
	std::string FileDataStr = std::string(std::istreambuf_iterator<char>(tmpFileStream), std::istreambuf_iterator<char>());
	const char* pDataStr = FileDataStr.c_str();
	glShaderSource(tmpShaderID, 1, &pDataStr, nullptr);
	glCompileShader(tmpShaderID);
	int success=0;
	glGetShaderiv(tmpShaderID, GL_COMPILE_STATUS, &success);
	if (!success)
	{
		char ErrMsg[10240];
		glGetShaderInfoLog(tmpShaderID, sizeof(ErrMsg), nullptr, ErrMsg);
		LastError = "In Shader "+ShaderFilePath+":\n"+ErrMsg;
		glDeleteShader(tmpShaderID);
		return false;
	}
	ShaderObjectList.push_back(tmpShaderID);
	return true;
}

bool Shader::Link()
{
	for (GLuint& ShaderID:ShaderObjectList)
		glAttachShader(ShaderProgramID, ShaderID);
	glLinkProgram(ShaderProgramID);
	int success = 0;
	glGetProgramiv(ShaderProgramID, GL_LINK_STATUS, &success);
	if (!success)
	{
		char ErrMsg[10240];
		glGetProgramInfoLog(ShaderProgramID, sizeof(ErrMsg), nullptr, ErrMsg);
		LastError = ErrMsg;
		for (GLuint& ShaderID : ShaderObjectList)
			glDetachShader(ShaderProgramID, ShaderID);
		return false;
	}
	bisLinked = true;
	for (auto it= ShaderObjectList.begin();it<ShaderObjectList.end();++it)
	{
		glDeleteShader(*it);
		it = ShaderObjectList.erase(it);
	}
	return true;
}

Shader& Shader::SetFloat(std::string VarName, GLfloat Value)
{
	GLuint Location = glGetUniformLocation(ShaderProgramID, VarName.c_str());
	glUniform1f(Location, Value);
	if (!(Location+1))
		printf("Unknow Location of Variable \"%s\"\n", VarName.c_str());
	return *this;
}

Shader& Shader::SetInt(std::string VarName, GLint Value)
{
	GLuint Location = glGetUniformLocation(ShaderProgramID, VarName.c_str());
	glUniform1i(Location, Value);
	if (!(Location+1))
		printf("Unknow Location of Variable \"%s\"\n", VarName.c_str());
	return *this;
}

Shader& Shader::SetMatrix4x4(std::string MatrixName, const glm::mat4x4 Martix)
{
	GLuint Location = glGetUniformLocation(ShaderProgramID, MatrixName.c_str());
	glUniformMatrix4fv(Location, 1,GL_FALSE,glm::value_ptr(Martix));
	if (!(Location + 1))
		printf("Unknow Location of Variable \"%s\"\n", MatrixName.c_str());
	return *this;
}

Shader& Shader::SetMatrix3x3(std::string MatrixName, const glm::mat3x3 Martix)
{
	GLuint Location = glGetUniformLocation(ShaderProgramID, MatrixName.c_str());
	glUniformMatrix3fv(Location, 1, GL_FALSE, glm::value_ptr(Martix));
	if (!(Location + 1))
		printf("Unknow Location of Variable \"%s\"\n", MatrixName.c_str());
	return *this;
}

Shader& Shader::SetVec3(std::string VarName, glm::vec3 Value)
{
	GLuint Location = glGetUniformLocation(ShaderProgramID, VarName.c_str());
	glUniform3f(Location, Value.x, Value.y, Value.z);
	if (!(Location + 1))
		printf("Unknow Location of Variable \"%s\"\n", VarName.c_str());
	return *this;
}

Shader& Shader::SetVec4(std::string VarName, glm::vec4 Value)
{
	GLuint Location = glGetUniformLocation(ShaderProgramID, VarName.c_str());
	glUniform4f(Location, Value.x, Value.y, Value.z,Value.w);
	if (!(Location + 1))
		printf("Unknow Location of Variable \"%s\"\n", VarName.c_str());
	return *this;
}

std::vector<Shader*> Shader::GetShaderList()
{
	return ShaderList;
}

Shader* Shader::FindShaderByName(std::string Name)
{
	for (Shader*& pShader : ShaderList)
		if (pShader->GetName() == Name)
			return pShader;
	return nullptr;
}

Shader::~Shader()
{
	ShaderList.erase(std::find(ShaderList.begin(), ShaderList.end(), this));
}

GLuint LoadTexture(std::string ImagePath)
{
	int x, y, channels;
	stbi_set_flip_vertically_on_load(true);
	unsigned char* imageData = stbi_load(ImagePath.c_str(), &x, &y, &channels, 0);
	if (!imageData)
	{
		printf("Load Image \"%s\" Failed!", ImagePath.c_str());
		return (GLuint)-1;
	}
	GLuint tmpTextureID = 0;
	glGenTextures(1, &tmpTextureID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, tmpTextureID);
	if (channels == 3)
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, x, y, 0, GL_RGB, GL_UNSIGNED_BYTE, imageData);
	else
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, imageData);
	glGenerateMipmap(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, NULL);
	stbi_image_free(imageData);
	return tmpTextureID;
}