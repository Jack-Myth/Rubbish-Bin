#include "Model.h"
#include "DxInfo.h"
#include "assimp/Importer.hpp"
#include <assimp/postprocess.h>
#include <assimp/scene.h>

void FMesh::FillData(std::vector<FVertex> VertexBuffer, std::vector<unsigned int> Indices)
{
	this->VertexBuffer = VertexBuffer;
	this->Indices = Indices;
}

void FMesh::Render(class ID3D11VertexShader* VertexShader, class ID3D11PixelShader* PixelShader, bool ProcessTexture/*=false*/)
{
	D3D11Info.D3D11DeviceContext->VSSetShader(VertexShader, nullptr, 0);
	D3D11Info.D3D11DeviceContext->PSSetShader(PixelShader, nullptr, 0);
	D3D11_INPUT_ELEMENT_DESC targetInputElement[3] = {NULL};
	//Vertex Pos
	targetInputElement[0].SemanticName = "POSITION";
	targetInputElement[0].AlignedByteOffset = 0;
	targetInputElement[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	targetInputElement[0].InputSlot = 0;
	targetInputElement[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	//Vertex Normal
	targetInputElement[1].SemanticName = "NORMAL";
	targetInputElement[1].AlignedByteOffset = sizeof(DirectX::XMFLOAT3);
	targetInputElement[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	targetInputElement[1].InputSlot = 0;
	targetInputElement[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	//UV
	targetInputElement[2].SemanticName = "UV";
	targetInputElement[2].AlignedByteOffset = 2 * sizeof(DirectX::XMFLOAT3);
	targetInputElement[2].Format = DXGI_FORMAT_R32G32_FLOAT;
	targetInputElement[2].InputSlot = 0;
	targetInputElement[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	D3D11Info.D3D11Device->CreateInputLayout(targetInputElement,3,VertexShader)
	D3D11Info.D3D11DeviceContext->Draw(VertexBuffer.size(), 0);
}

void FModel::processNode(aiNode* node, const aiScene* scene)
{
	for (int i=0;i<node->mNumMeshes;i++)
	{
		FMesh* targetMesh = processMesh(scene->mMeshes[node->mMeshes[i]], scene);
		MeshCollection.push_back(targetMesh);
	}
	for (int i = 0; i < node->mNumChildren; i++)
		processNode(node->mChildren[i], scene);
}

FMesh* FModel::processMesh(class aiMesh* ai_mesh, const class aiScene* scene)
{
	FMesh* targetMesh = new FMesh();
	std::vector<FVertex> MeshVertex;
	std::vector<unsigned int> MeshIndices;
	//Process Mesh vertices;
	for (int i=0;i<ai_mesh->mNumVertices;i++)
	{
		FVertex tmpVertex;
		tmpVertex.Pos.x = ai_mesh->mVertices[i].x;
		tmpVertex.Pos.y = ai_mesh->mVertices[i].y;
		tmpVertex.Pos.z = ai_mesh->mVertices[i].z;
		tmpVertex.Normal.x = ai_mesh->mNormals[i].x;
		tmpVertex.Normal.y = ai_mesh->mNormals[i].y;
		tmpVertex.Normal.z = ai_mesh->mNormals[i].z;
		if (ai_mesh->HasTextureCoords(0))
		{
			tmpVertex.UV.x = ai_mesh->mTextureCoords[0][i].x;
			tmpVertex.UV.y = ai_mesh->mTextureCoords[0][i].y;
		}
		else
			tmpVertex.UV = DirectX::XMFLOAT2(0,0);
		MeshVertex.push_back(tmpVertex);
	}
	//Process Indices(faces);
	for (int i=0;i<ai_mesh->mNumFaces;i++)
	{
		//Because I specify the aiProcess_Triangulate flag,so all the faces will have 3 indices;
		for (unsigned int index = 0; index < ai_mesh->mFaces[i].mNumIndices; index++)
			MeshIndices.push_back(ai_mesh->mFaces[o].mIndices[index]);
	}
	targetMesh->FillData(MeshVertex, MeshIndices);
	return targetMesh;
}

FModel* FModel::LoadModel(std::string FilePath)
{
	Assimp::Importer AssimpImporter;
	const aiScene* AssimpScene = AssimpImporter.ReadFile(FilePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenNormals);
	if (!AssimpScene || AssimpScene->mFlags&AI_SCENE_FLAGS_INCOMPLETE || !AssimpScene->mRootNode)
	{
		printf("Failed to Load Mesh \"%s\"\n", FilePath.c_str());
		return nullptr;
	}
	FModel* tmpModel = new FModel();
	tmpModel->processNode(AssimpScene->mRootNode, AssimpScene);
}

void FModel::Render(class ID3D11VertexShader* VertexShader, class ID3D11PixelShader* PixelShader)
{
	for (FMesh*& MeshElement : MeshCollection)
	{
		MeshElement->Render(VertexShader,PixelShader);
	}
}
