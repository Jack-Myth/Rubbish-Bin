#include "Model.h"
#include "glm/gtc/matrix_transform.hpp"

void Model::processNode(aiNode* node, const aiScene* scene)
{
	for (unsigned int i=0;i<node->mNumMeshes;i++)
	{
		Mesh* LoadedMesh = processMesh(scene->mMeshes[node->mMeshes[i]], scene);
		meshes.push_back(LoadedMesh);
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++)
		processNode(node->mChildren[i], scene);
}

Mesh* Model::processMesh(aiMesh* ai_mesh, const aiScene* scene)
{
	Mesh* curMesh= new Mesh();
	//Process Vertices
	for (unsigned int i=0;i<ai_mesh->mNumVertices;i++)
	{
		Vertex tmpVertex;
		tmpVertex.Position.x = ai_mesh->mVertices[i].x;
		tmpVertex.Position.y = ai_mesh->mVertices[i].y;
		tmpVertex.Position.z = ai_mesh->mVertices[i].z;
		tmpVertex.Normal.x = ai_mesh->mNormals[i].x;
		tmpVertex.Normal.y = ai_mesh->mNormals[i].y;
		tmpVertex.Normal.z = ai_mesh->mNormals[i].z;
		if (ai_mesh->HasTextureCoords(0))
		{
			tmpVertex.TexCoords.x = ai_mesh->mTextureCoords[0][i].x;
			tmpVertex.TexCoords.y = ai_mesh->mTextureCoords[0][i].y;
		}
		else
			tmpVertex.TexCoords = glm::vec2(0, 0);
		curMesh->vertices.push_back(tmpVertex);
	}
	//Process Indices
	for (unsigned int i=0;i<ai_mesh->mNumFaces;i++)
	{
		for (unsigned int index = 0; index < ai_mesh->mFaces[i].mNumIndices; index++)
			curMesh->indices.push_back(ai_mesh->mFaces[i].mIndices[index]);
	}
	//Process Material(Textures)
	auto tmpMat = scene->mMaterials[ai_mesh->mMaterialIndex];
	if (tmpMat->GetTextureCount(aiTextureType_DIFFUSE))
	{
		Texture tmpTexture;
		aiString TexPath;
		tmpMat->GetTexture(aiTextureType_DIFFUSE, 0, &TexPath);
		auto it = std::find_if(TextureLoaded.begin(), TextureLoaded.end(), [TexPath](TextureRef& texA)
			{
				return texA.TexturePath == TexPath.C_Str();
			});
		if (it != TextureLoaded.end())
		{ 
			tmpTexture.id = it->id;
			tmpTexture.type = TextureType::DiffuseMap;
			curMesh->textures.push_back(tmpTexture);
		}
		else
		{
			tmpTexture.id = LoadTexture(TexPath.C_Str());
			if (tmpTexture.id + 1)
			{
				tmpTexture.type = TextureType::DiffuseMap;
				curMesh->textures.push_back(tmpTexture);
				TextureLoaded.push_back({ tmpTexture.id,TexPath.C_Str() });
			}
		}
	}
	if (tmpMat->GetTextureCount(aiTextureType_SPECULAR))
	{
		Texture tmpTexture;
		aiString TexPath;
		tmpMat->GetTexture(aiTextureType_DIFFUSE, 0, &TexPath);
		auto it = std::find_if(TextureLoaded.begin(), TextureLoaded.end(), [TexPath](TextureRef& texA)
			{
				return texA.TexturePath == TexPath.C_Str();
			});
		if (it != TextureLoaded.end())
		{
			tmpTexture.id = it->id;
			tmpTexture.type = TextureType::SepcularMap;
			curMesh->textures.push_back(tmpTexture);
		}
		else
		{
			tmpTexture.id = LoadTexture(TexPath.C_Str());
			if (tmpTexture.id + 1)
			{
				tmpTexture.type = TextureType::SepcularMap;
				curMesh->textures.push_back(tmpTexture);
				TextureLoaded.push_back({ tmpTexture.id,TexPath.C_Str() });
			}
		}
	}
	curMesh->RefreshBuffer();
	return curMesh;
}

Model* Model::LoadMesh(std::string MeshPath)
{
	Assimp::Importer importer;
	const aiScene* scene = importer.ReadFile(MeshPath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
	if (!scene||scene->mFlags&AI_SCENE_FLAGS_INCOMPLETE||!scene->mRootNode)
	{
		printf("Failed to Load Mesh \"%s\"\n", MeshPath.c_str());
		return nullptr;
	}
	Model* tmpModel = new Model();
	tmpModel->processNode(scene->mRootNode, scene);
	return tmpModel;
}

void Model::Draw(Shader* UsedShader, bool ProcessTexture)
{
	glm::mat4x4 ModelMatrix(1.f);
	ModelMatrix = glm::rotate(ModelMatrix, glm::radians(Transform.Rotation.z), glm::vec3(0, 1, 0));
	ModelMatrix = glm::rotate(ModelMatrix, glm::radians(Transform.Rotation.y - 90.f), glm::vec3(1, 0, 0));
	ModelMatrix = glm::rotate(ModelMatrix, glm::radians(Transform.Rotation.x), glm::vec3(0, 0, 1));
	ModelMatrix = glm::translate(ModelMatrix, Transform.Location);
	//ModelMatrix = glm::rotate(ModelMatrix, glm::radians(-90.f), glm::vec3(1, 0, 0));
	ModelMatrix = glm::scale(ModelMatrix, Transform.Scale);
	UsedShader->SetMatrix4x4("ModelMatrix", ModelMatrix);
	for (Mesh*& mesh : meshes)
		mesh->Draw(UsedShader, ProcessTexture);
}

Model::~Model()
{
	for (Mesh*& mesh : meshes)
		delete mesh;
}

glm::mat4x4 FTransform::GenModelMatrix()
{
	glm::mat4x4 ModelMatrix(1.f);
	ModelMatrix = glm::translate(ModelMatrix, Location);
	ModelMatrix = glm::rotate(ModelMatrix, glm::radians(Rotation.z), glm::vec3(0, 1, 0));
	ModelMatrix = glm::rotate(ModelMatrix, glm::radians(Rotation.y), glm::vec3(1, 0, 0));
	ModelMatrix = glm::rotate(ModelMatrix, glm::radians(Rotation.x), glm::vec3(0, 0, 1));
	ModelMatrix = glm::scale(ModelMatrix, Scale);
	return ModelMatrix;
}
