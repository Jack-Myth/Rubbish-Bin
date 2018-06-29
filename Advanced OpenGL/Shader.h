#pragma once
#include <vector>
#include <string>
#include "glad/glad.h"
#include "glm/glm.hpp"

class Shader
{
	static std::vector<Shader*> ShaderList;
	bool bisLinked=false;
	std::string Name;
	GLuint ShaderProgramID;
	std::vector<GLuint> ShaderObjectList;
	static std::vector<Shader*> AllShaderRef;
	std::string LastError;
public:
	Shader(std::string Name="");
	Shader(std::string VertexShaderFilePath, std::string FragmentShaderFilePath,std::string Name="");
	inline std::string GetName()
	{
		return Name;
	}
	inline std::string SetName(std::string NewName)
	{
		Name = NewName;
	}
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
	Shader& SetFloat(std::string VarName, GLfloat Value);
	Shader& SetInt(std::string VarName, GLint Value);
	Shader& SetMatrix4x4(std::string MatrixName, const glm::mat4x4 Martix);
	Shader& SetMatrix3x3(std::string MatrixName, const glm::mat3x3 Martix);
	Shader& SetVec3(std::string VarName, glm::vec3 Value);
	Shader& SetVec4(std::string VarName, glm::vec4 Value);
	static std::vector<Shader*> GetShaderList();
	static Shader* FindShaderByName(std::string Name);
	virtual ~Shader();
};

GLuint LoadTexture(std::string ImagePath);
GLuint LoadCubeMap(std::vector<std::string> CubeMapFaces,std::vector<bool> FlipMask = {0,0,1,1,0,0});