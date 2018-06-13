#pragma once
#include "Mesh.h"
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct FTransform
{
	glm::vec3 Location=glm::vec3(0,0,0);
	glm::vec3 Rotation= glm::vec3(0, 0, 0);
	glm::vec3 Scale= glm::vec3(1, 1, 1);
};

struct TextureRef
{
	GLuint id;
	std::string TexturePath;
};

class Model
{
	std::vector<Mesh*> meshes;
	std::vector<TextureRef> TextureLoaded;
	Model()=default;
	void processNode(aiNode* node, const aiScene* scene);
	Mesh* processMesh(aiMesh* ai_mesh, const aiScene* scene);
public:
	FTransform Transform;
	static Model* LoadMesh(std::string MeshPath);
	void Draw(Shader* UsedShader);
	virtual ~Model();
};