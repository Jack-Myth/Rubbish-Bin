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
	void Render(class FShader* VertexShader, class FShader* PixelShader,bool ProcessTexture=false);
};

class FModel
{
	FTransform ModelTransform;
	std::vector<FMesh*> MeshCollection;
	void processNode(struct aiNode* node, const struct aiScene* scene);
	FMesh* processMesh(struct aiMesh* ai_mesh, const struct aiScene* scene);
public:
	static FModel* LoadModel(std::string FilePath);
	void Render(class FShader* VertexShader, class FShader* PixelShader);
	inline void SetTransform(FTransform newTransform)
	{
		ModelTransform = newTransform;
	}
	inline FTransform GetTransform()
	{
		return ModelTransform;
	}
};