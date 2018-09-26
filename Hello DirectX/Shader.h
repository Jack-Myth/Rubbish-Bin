#pragma once
#include <d3dcommon.h>
#include <string>

class FShader
{
public:
	union
	{
		struct ID3D11VertexShader* VertexShader;
		struct ID3D11GeometryShader* GeometryShader;
		struct ID3D11PixelShader* PixelShader;
		struct ID3D11ComputeShader* ComputeShader;
	};
	ID3DBlob* ShaderBuffer;
	std::string ShaderPath;
	bool CompileShader(const std::string& ShaderPath,LPCSTR pEntryFunction, LPCSTR pTarget);
	static FShader* LoadVertexShader(const std::string& ShaderPath,LPCSTR pEntryFunction);
	static FShader* LoadPixelShader(const std::string& ShaderPath,LPCSTR pEntryFunction);
};