#pragma once
#include <vector>
#include <string>
#include "glad/glad.h"
#include "glm/glm.hpp"

class Shader
{
	bool bisLinked=false;
	std::string Name;
	GLuint ShaderProgramID;
	std::vector<GLuint> ShaderObjectList;
	static std::vector<Shader*> AllShaderRef;
	std::string LastError;
public:
	Shader(std::string Name="");
	Shader(std::string VertexShaderFilePath, std::string FragmentShaderFilePath,std::string Name="");
	bool AttachShader(GLenum ShaderType,std::string ShaderFilePath);
	bool Link();
	inline GLuint GetShaderProgramID()
	{
		return ShaderProgramID;
	}
	inline bool IsLinked() 
	{
		return bisLinked; 
	};
	inline void Use()
	{
		glUseProgram(ShaderProgramID);
	}
	inline std::string& GetLastError()
	{
		return LastError;
	}
	void SetFloat(std::string VarName, GLfloat Value);
	void SetInt(std::string VarName, GLint Value);
	void SetMatrix4x4(std::string MatrixName, const glm::mat4x4 Martix);
	void SetMatrix3x3(std::string MatrixName, const glm::mat3x3 Martix);
	void SetVec3(std::string VarName, glm::vec3 Value);
	void SetVec4(std::string VarName, glm::vec4 Value);
};