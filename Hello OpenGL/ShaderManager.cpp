#include <streambuf> 
#include <fstream>
#include "ShaderManager.h"
#include <string>

#define ERRMSG_BUFFER_SIZE 1024
ShaderManager* ShaderManager::ShaderManagerInstance = nullptr;

bool Shader::CompileShader()
{
	GLint success = 0;
	glCompileShader(ShaderObjectID);
	glGetShaderiv(ShaderObjectID,GL_COMPILE_STATUS, &success);
	if (success)
	{
		bIsCompiled = true;
		return true;
	}
	else
	{
		char ErrorMsg[ERRMSG_BUFFER_SIZE];
		glGetShaderInfoLog(ShaderObjectID, sizeof(ErrorMsg), nullptr, ErrorMsg);
		LastError = ErrorMsg;
		return false;
	}
}

Shader* Shader::LoadShader(GLenum mShaderType, const char* ShaderBuffer, const std::string ShaderName/*=""*/)
{
	Shader* tmppShader = new Shader;
	tmppShader->ShaderObjectID = glCreateShader(mShaderType);
	glShaderSource(tmppShader->GetObjectID(), 1, &ShaderBuffer, nullptr);
	tmppShader->SetName(ShaderName);
	tmppShader->ShaderType = mShaderType;
	ShaderManager::Get().InsertShaderRef(tmppShader);
	return tmppShader;
}

Shader* Shader::LoadShaderFromFile(GLenum mShaderType, std::string ShaderFilePath, const std::string ShaderName /*= ""*/)
{
	std::ifstream tmpStream(ShaderFilePath);
	if (!tmpStream.is_open())
		return nullptr;
	std::string tmpShaderContent=std::string(std::istreambuf_iterator<char>(tmpStream),std::istreambuf_iterator<char>());
	return LoadShader(mShaderType, tmpShaderContent.c_str(),ShaderName);
}

Shader::~Shader()
{
	ShaderManager::Get().RemoveShaderRef(this);
}

ShaderProgram::ShaderProgram(const std::string ProgramName /*= ""*/)
{
	ShaderManager::Get().InsertShaderProgramRef(this);
	SetName(ProgramName);
}

bool ShaderProgram::LinkShader(std::initializer_list<Shader*> iShaderList)
{
	if (IsLinked())
	{
		LastError = "Program \"" + Name + "\" has already linked";
		return false;
	}
	for (Shader* pShaderI:iShaderList)
		ShaderList.push_back(pShaderI);
	for (Shader* pShaderI:ShaderList)
		glAttachShader(ShaderProgramID, pShaderI->GetObjectID());
	glLinkProgram(ShaderProgramID);
	GLint success=0;
	glGetProgramiv(ShaderProgramID, GL_LINK_STATUS, &success);
	if (success)
	{
		bIsLinked = true;
		return true;
	}
	else
	{
		char ErrorMsg[ERRMSG_BUFFER_SIZE];
		glGetProgramInfoLog(ShaderProgramID, sizeof(ErrorMsg), nullptr, ErrorMsg);
		LastError = ErrorMsg;
		return false;
	}
}

Shader* ShaderProgram::GetVertexShader()
{
	for (Shader* pShaderI:ShaderList)
	{
		if (pShaderI->IsTypeEqual(GL_VERTEX_SHADER))
			return pShaderI;
	}
	return nullptr;
}

Shader* ShaderProgram::GetFragmentShader()
{
	for (Shader* pShaderI : ShaderList)
	{
		if (pShaderI->IsTypeEqual(GL_FRAGMENT_SHADER))
			return pShaderI;
	}
	return nullptr;
}

Shader* ShaderProgram::GetGeometryShader()
{
	for (Shader* pShaderI : ShaderList)
	{
		if (pShaderI->IsTypeEqual(GL_GEOMETRY_SHADER))
			return pShaderI;
	}
	return nullptr;
}

void ShaderProgram::SetUniform(const std::string UniformName, bool BoolValue) const
{
	glUniform1i(glGetUniformLocation(ShaderProgramID, UniformName.c_str()), (int)BoolValue);
}

void ShaderProgram::SetUniform(const std::string UniformName, int IntValue) const
{
	glUniform1i(glGetUniformLocation(ShaderProgramID, UniformName.c_str()), IntValue);
}

void ShaderProgram::SetUniform(const std::string UniformName, float FloatValue) const
{
	glUniform1f(glGetUniformLocation(ShaderProgramID, UniformName.c_str()), FloatValue);
}

void ShaderProgram::SetUniform(const std::string UniformName, double FloatValue) const
{
	glUniform1f(glGetUniformLocation(ShaderProgramID, UniformName.c_str()), (float)FloatValue);
}

void ShaderProgram::SetUniform(const std::string UniformName, float FloatX, float FloatY) const
{
	glUniform2f(glGetUniformLocation(ShaderProgramID, UniformName.c_str()), FloatX,FloatY);
}

void ShaderProgram::SetUniform(const std::string UniformName, float FloatX, float FloatY, float FloatZ) const
{
	glUniform3f(glGetUniformLocation(ShaderProgramID, UniformName.c_str()), FloatX, FloatY, FloatZ);
}

void ShaderProgram::SetUniform(const std::string UniformName, float FloatX, float FloatY, float FloatZ, float FloatW) const
{
	glUniform4f(glGetUniformLocation(ShaderProgramID, UniformName.c_str()), FloatX, FloatY, FloatZ, FloatW);
}

ShaderProgram::~ShaderProgram()
{
	ShaderManager::Get().RemoveShaderProgramRef(this);
}

void ShaderManager::InsertShaderRef(Shader* pShader)
{
	ShaderList.push_back(pShader);
}

void ShaderManager::RemoveShaderRef(Shader* pShader)
{
	ShaderList.erase(std::find(ShaderList.begin(), ShaderList.end(), pShader));
}

void ShaderManager::InsertShaderProgramRef(ShaderProgram* pShaderProgram)
{
	ShaderProgramList.push_back(pShaderProgram);
}

void ShaderManager::RemoveShaderProgramRef(ShaderProgram* pShaderProgram)
{
	ShaderProgramList.erase(std::find(ShaderList.begin(), ShaderList.end(), pShaderProgram));
}

ShaderManager& ShaderManager::Get()
{
	if (!ShaderManagerInstance)
		ShaderManagerInstance = new ShaderManager;
	return *ShaderManagerInstance;
}

Shader * ShaderManager::FindShader(const std::string Name)
{
	for (Shader* pShader:ShaderList)
	{
		if (pShader->GetName() == Name)
			return pShader;
	}
}

ShaderProgram * ShaderManager::FindShaderProgram(const std::string Name)
{
	for (ShaderProgram* pShaderProgram : ShaderProgramList)
	{
		if (pShaderProgram->GetName() == Name)
			return pShaderProgram;
	}
}