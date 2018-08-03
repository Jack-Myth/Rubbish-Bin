#include "Mesh.h"

Mesh::Mesh()
{
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);
	glGenVertexArrays(1, &VAO);
}

void Mesh::FillData(std::vector<Vertex> newVertices, std::vector<GLuint> newIndices, std::vector<Texture> newTextures)
{
	vertices = newVertices;
	indices = newIndices;
	textures = newTextures;
	RefreshBuffer();
}

void Mesh::RefreshBuffer()
{
	glBindVertexArray(VAO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
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

void Mesh::Draw(Shader* UsedShader,bool ProcessShader)
{
	if (ProcessShader)
	{
		int MaxTextureSize = textures.size() > 15 ? 15 : textures.size();
		UsedShader->SetInt("DiffuseMap", 0);
		UsedShader->SetInt("SpecularMap", 1);
		UsedShader->SetInt("UseDiffuseMap", GL_FALSE);
		UsedShader->SetInt("UseSpecluarMap", GL_FALSE);
		for (int i = 0; i < MaxTextureSize; i++)
		{
			switch (textures[i].type)
			{
				case TextureType::DiffuseMap:
					glActiveTexture(GL_TEXTURE0); //DiffuseMap
					UsedShader->SetInt("UseDiffuseMap", GL_TRUE);
					break;
				case TextureType::SepcularMap:
					glActiveTexture(GL_TEXTURE1);
					UsedShader->SetInt("UseSpecluarMap", GL_TRUE);
					break;
					//Normal Map maybe 3 instaded of 2
					//case TextureType::NormalMap:
						//glActiveTexture(GL_TEXTURE2);
			}
			glBindTexture(GL_TEXTURE_2D, textures[i].id);
		}
	}
	UsedShader->Use();
	glActiveTexture(GL_TEXTURE0);
	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(NULL);
}

Mesh::~Mesh()
{
	glDeleteVertexArrays(1,&VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}
