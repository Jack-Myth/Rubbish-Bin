#include "Mesh.h"

void Mesh::FillData(std::vector<Vertex> newVertices, std::vector<GLuint> newIndices, std::vector<Texture> newTextures)
{
	vertices = newVertices;
	indices = newIndices;
	textures = newTextures;
	RefreshBuffer();
}

void Mesh::RefreshBuffer()
{
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VAO);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(VBO, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
	glBufferData(EBO, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
	//Vertex Position
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Position));
	glEnableVertexAttribArray(0);
	//VertexNormal
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
	glEnableVertexAttribArray(1);
	//Texture Coordinate;
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
	glEnableVertexAttribArray(2);
	glBindVertexArray(NULL);
}

void Mesh::Draw(Shader* UsedShader)
{
	int MaxTextureSize = textures.size() > 15 ? 15 : textures.size();
	UsedShader->SetInt("DiffuseMap", 0);
	UsedShader->SetInt("SepcularMap", 1);
	UsedShader->SetInt("NormalMap", 2);
	for (int i=0;i<MaxTextureSize;i++)
	{
		switch (textures[i].type)
		{
			case TextureType::DiffuseMap:
				glActiveTexture(GL_TEXTURE0); //DiffuseMap
				break;
			case TextureType::SepcularMap:
				glActiveTexture(GL_TEXTURE1);
				break;
			case TextureType::NormalMap:
				glActiveTexture(GL_TEXTURE2);
		}
		glBindTexture(GL_TEXTURE_2D, textures[i].id);
	}
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(NULL);
}
