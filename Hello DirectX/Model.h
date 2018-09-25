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
	struct ID3D11InputLayout* Inputlayout;
	struct ID3D11Buffer* D3DVertexBuffer, *D3DIndicesBuffer;
public:
	void FillData(std::vector<FVertex> VertexBuffer,std::vector<unsigned int> Indices);
	void RenderInit(class FShader* VertexShader, class FShader* PixelShader,bool ProcessTexture=false);
	void Draw();
};

class FModel
{
	FTransform ModelTransform;
	std::vector<FMesh*> MeshCollection;
	void processNode(struct aiNode* node, const struct aiScene* scene);
	FMesh* processMesh(struct aiMesh* ai_mesh, const struct aiScene* scene);
public:
	static FModel* LoadModel(std::string FilePath);
	void RenderInit(class FShader* VertexShader, class FShader* PixelShader);
	void Draw();
	inline void SetTransform(FTransform newTransform)
	{
		ModelTransform = newTransform;
	}
	inline FTransform GetTransform()
	{
		return ModelTransform;
	}
};