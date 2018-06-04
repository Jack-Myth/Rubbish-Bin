#pragma once
#include <glad/glad.h>
#include <string>
#include <vector>

class Shader
{
	GLuint ShaderObjectID=0;
	bool bIsCompiled = false;
	GLenum ShaderType;
	std::string Name;
	std::string LastError;
	Shader();
public:
	inline GLuint GetObjectID()
	{
		return ShaderObjectID;
	}
	inline void SetName(const std::string NewName)
	{
		Name = NewName;
	}
	inline std::string GetName()
	{
		return Name;
	}
	std::string GetLastError()
	{
		return LastError;
	}
	bool CompileShader();
	inline bool IsCompiled()
	{
		return bIsCompiled;
	}
	bool IsTypeEqual(GLenum sType)
	{
		return ShaderType == sType;
	}
	static Shader* LoadShader(GLenum mShaderType, const char* ShaderBuffer,const std::string ShaderName="");
	static Shader* LoadShaderFromFile(GLenum mShaderType,std::string ShaderFilePath, const std::string ShaderName = "");
	virtual ~Shader();
};

class ShaderProgram
{
	GLuint ShaderProgramID = 0;
	bool bIsLinked = false;
	std::vector<Shader*> ShaderList;
	std::string Name;
	std::string LastError;
public:
	ShaderProgram(const std::string ProgramName = "");
	inline GLuint GetObjectID()
	{
		return ShaderProgramID;
	}
	void AttachShader(Shader* pShader)
	{
		if(pShader)
			ShaderList.push_back(pShader);
	}
	bool LinkShader(std::initializer_list<Shader*> iShaderList={});
	bool IsLinked()
	{
		return bIsLinked;
	}
	std::string GetLastError()
	{
		return LastError;
	}
	Shader* GetVertexShader();
	Shader* GetFragmentShader();
	Shader* GetGeometryShader();
	inline void Use()
	{
		glUseProgram(ShaderProgramID);
	}
	void SetName(const std::string NewName)
	{
		Name = NewName;
	}
	std::string GetName()
	{
		return Name;
	}
	void SetUniform(const std::string UniformName, bool BoolValue) const;
	void SetUniform(const std::string UniformName, int IntValue) const;
	void SetUniform(const std::string UniformName, float FloatValue) const;
	void SetUniform(const std::string UniformName, double FloatValue) const;
	void SetUniform(const std::string UniformName, float FloatX, float FloatY) const;
	void SetUniform(const std::string UniformName, float FloatX, float FloatY, float FloatZ) const;
	void SetUniform(const std::string UniformName, float FloatX, float FloatY, float FloatZ, float FloatW) const;
	virtual ~ShaderProgram();
};

class ShaderManager
{
	friend class Shader;
	friend class ShaderProgram;
	static ShaderManager* ShaderManagerInstance;
	std::vector<Shader*> ShaderList;
	std::vector<ShaderProgram*> ShaderProgramList;
	void InsertShaderRef(Shader* pShader);
	void RemoveShaderRef(Shader* pShader);
	void InsertShaderProgramRef(ShaderProgram* pShaderProgram);
	void RemoveShaderProgramRef(ShaderProgram* pShaderProgram);
public:
	static ShaderManager& Get();
	Shader * FindShader(const std::string Name);
	ShaderProgram * FindShaderProgram(const std::string Name);
};