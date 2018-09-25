#pragma once

#include <string>

struct FShader
{
	union
	{
		class ID3D11VertexShader* VertexShader;
		class ID3D11GeometryShader* GeometryShader;
		class ID3D11PixelShader* PixelShader;
		class ID3D11ComputeShader* ComputeShader;
	};
	class ID3DBlob* ShaderBuffer;
	std::string ShaderPath;
	bool CompileShader(const std::string ShaderPath);
	static FShader* LoadVertexShader(const std::string ShaderPath);
	static FShader* LoadPixelShader(const std::string ShaderPath);
};