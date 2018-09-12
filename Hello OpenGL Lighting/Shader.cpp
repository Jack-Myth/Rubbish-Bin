#include "Shader.h"
#include <fstream>
#include <streambuf>
#include "glm/gtc/type_ptr.hpp"

std::vector<Shader*> Shader::AllShaderRef;

Shader::Shader(std::string Name/*=""*/)
{
	this->Name = Name;
	ShaderProgramID = glCreateProgram();
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
	for (auto it= ShaderObjectList.begin();it<ShaderObjectList.end();)
	{
		glDeleteShader(*it);
		it = ShaderObjectList.erase(it);
	}
	return true;
}

void Shader::SetFloat(std::string VarName, GLfloat Value)
{
	glUniform1f(glGetUniformLocation(ShaderProgramID, VarName.c_str()), Value);
}

void Shader::SetInt(std::string VarName, GLint Value)
{
	glUniform1i(glGetUniformLocation(ShaderProgramID, VarName.c_str()), Value);
}

void Shader::SetMatrix4x4(std::string MatrixName, const glm::mat4x4 Martix)
{
	glUniformMatrix4fv(glGetUniformLocation(ShaderProgramID, MatrixName.c_str()), 1,GL_FALSE,glm::value_ptr(Martix));
}

void Shader::SetMatrix3x3(std::string MatrixName, const glm::mat3x3 Martix)
{
	glUniformMatrix3fv(glGetUniformLocation(ShaderProgramID, MatrixName.c_str()), 1, GL_FALSE, glm::value_ptr(Martix));
}

void Shader::SetVec3(std::string VarName, glm::vec3 Value)
{
	glUniform3f(glGetUniformLocation(ShaderProgramID,VarName.c_str()), Value.x, Value.y, Value.z);
}

void Shader::SetVec4(std::string VarName, glm::vec4 Value)
{
	glUniform4f(glGetUniformLocation(ShaderProgramID, VarName.c_str()), Value.x, Value.y, Value.z,Value.w);
}