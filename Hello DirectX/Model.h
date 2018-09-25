#pragma once
#include <vector>
#include <DirectXMath.h>
#include "Transform.h"

struct FVertex
{
	DirectX::XMFLOAT3 Pos;
	DirectX::XMFLOAT3 Normal;
	DirectX::XMFLOAT2 UV;
};

class FMesh
{
	std::vector<FVertex> VertexBuffer;
	std::vector<unsigned int> Indices;
public:
	void FillData(std::vector<FVertex> VertexBuffer,std::vector<unsigned int> Indices);
	void Render(class ID3D11VertexShader* VertexShader,class ID3D11PixelShader* PixelShader,bool ProcessTexture=false);
};

class FModel
{
	FTransform ModelTransform;
	std::vector<FMesh*> MeshCollection;
	FModel();
	void processNode(class aiNode* node, const class aiScene* scene);
	FMesh* processMesh(class aiMesh* ai_mesh, const class aiScene* scene);
public:
	static FModel* LoadModel(std::string FilePath);
	void Render(class ID3D11VertexShader* VertexShader, class ID3D11PixelShader* PixelShader);
	inline void SetTransform(FTransform newTransform)
	{
		ModelTransform = newTransform;
	}
	inline FTransform GetTransform()
	{
		return ModelTransform;
	}
};