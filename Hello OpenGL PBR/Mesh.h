#pragma once
#include "glm/glm.hpp"
#include <string>
#include "glad/glad.h"
#include <vector>
#include "Shader.h"

struct Vertex
{
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};

enum class TextureType:unsigned char
{
	DiffuseMap,
	SepcularMap,
	NormalMap
};

struct Texture
{
	GLuint id;
	TextureType type;
};

class Mesh
{
	GLuint VAO, VBO, EBO;
public:
	Mesh();
	std::vector<Vertex> vertices;
	std::vector<GLuint> indices;
	std::vector<Texture> textures;
	void FillData(std::vector<Vertex> newVertices, std::vector<GLuint> newIndices, std::vector<Texture> newTextures);
	void RefreshBuffer();
	void Draw(Shader* UsedShader, bool ProcessShader = true);
	inline GLuint GetVAO()
	{
		return VAO;
	}
	virtual ~Mesh();
};