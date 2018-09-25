#include <assimp/Importer.hpp>
#include <assimp/postprocess.h>
#include <assimp/scene.h>
#include "Model.h"
#include "DxInfo.h"
#include "Shader.h"

void FMesh::FillData(std::vector<FVertex> VertexBuffer, std::vector<unsigned int> Indices)
{
	this->VertexBuffer = VertexBuffer;
	this->Indices = Indices;
}

void FMesh::Render(FShader* VertexShader, struct FShader* PixelShader, bool ProcessTexture/*=false*/)
{
	D3D11Info.D3D11DeviceContext->VSSetShader(VertexShader->VertexShader, nullptr, 0);
	D3D11Info.D3D11DeviceContext->PSSetShader(PixelShader->PixelShader, nullptr, 0);
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
	ID3D11InputLayout* Inputlayout;
	D3D11Info.D3D11Device->CreateInputLayout(targetInputElement, 3,
		VertexShader->ShaderBuffer->GetBufferPointer(),
		VertexShader->ShaderBuffer->GetBufferSize(), &Inputlayout);
	D3D11_BUFFER_DESC BufferDesc = { NULL };
	BufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	BufferDesc.ByteWidth = VertexBuffer.size() * sizeof(FVertex);
	BufferDesc.CPUAccessFlags = NULL;
	BufferDesc.MiscFlags = NULL;
	BufferDesc.StructureByteStride = NULL;
	D3D11_SUBRESOURCE_DATA SubresourceData = { NULL };
	SubresourceData.pSysMem = VertexBuffer.data();
	D3D11_BUFFER_DESC IndicesBufferDesc = { NULL };
	IndicesBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	IndicesBufferDesc.ByteWidth = Indices.size() * sizeof(FVertex);
	IndicesBufferDesc.CPUAccessFlags = NULL;
	IndicesBufferDesc.MiscFlags = NULL;
	IndicesBufferDesc.StructureByteStride = NULL;
	D3D11_SUBRESOURCE_DATA IndicesSubresourceData = { NULL };
	IndicesSubresourceData.pSysMem = Indices.data();
	ID3D11Buffer* D3DVertexBuffer,*D3DIndicesBuffer;
	D3D11Info.D3D11Device->CreateBuffer(&BufferDesc, &SubresourceData, &D3DVertexBuffer);
	D3D11Info.D3D11Device->CreateBuffer(&IndicesBufferDesc, &IndicesSubresourceData, &D3DIndicesBuffer);
	UINT stride = sizeof(FVertex);
	UINT offset = 0;
	D3D11Info.D3D11DeviceContext->IASetVertexBuffers(0, 1, &D3DVertexBuffer, &stride, &offset);
	D3D11Info.D3D11DeviceContext->IASetInputLayout(Inputlayout);
	D3D11Info.D3D11DeviceContext->IASetIndexBuffer(D3DIndicesBuffer, DXGI_FORMAT_R32_UINT, 0);
	D3D11Info.D3D11DeviceContext->IASetPrimitiveTopology(D3D10_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	D3D11Info.D3D11DeviceContext->Draw(VertexBuffer.size(), 0);
}

void FModel::processNode(aiNode* node, const aiScene* scene)
{
	for (unsigned int i=0;i<node->mNumMeshes;i++)
	{
		FMesh* targetMesh = processMesh(scene->mMeshes[node->mMeshes[i]], scene);
		MeshCollection.push_back(targetMesh);
	}
	for (unsigned int i = 0; i < node->mNumChildren; i++)
		processNode(node->mChildren[i], scene);
}

FMesh* FModel::processMesh(struct aiMesh* ai_mesh, const struct aiScene* scene)
{
	FMesh* targetMesh = new FMesh();
	std::vector<FVertex> MeshVertex;
	std::vector<unsigned int> MeshIndices;
	//Process Mesh vertices;
	for (unsigned int i=0;i<ai_mesh->mNumVertices;i++)
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
	for (unsigned int i=0;i<ai_mesh->mNumFaces;i++)
	{
		//Because I specify the aiProcess_Triangulate flag,so all the faces will have 3 indices;
		for (unsigned int index = 0; index < ai_mesh->mFaces[i].mNumIndices; index++)
			MeshIndices.push_back(ai_mesh->mFaces[i].mIndices[index]);
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
	return tmpModel;
}

void FModel::Render(FShader* VertexShader, FShader* PixelShader)
{
	for (FMesh*& MeshElement : MeshCollection)
	{
		MeshElement->Render(VertexShader,PixelShader);
	}
}
