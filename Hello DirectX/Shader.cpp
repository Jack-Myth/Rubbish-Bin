#include "Shader.h"
#include "DxInfo.h"
#include "D3Dcompiler.h"
#pragma comment(lib,"D3DCompiler")

bool FShader::CompileShader(const std::string ShaderPath /*= ""*/)
{
	wsprintf()
}

FShader* FShader::LoadVertexShader(const std::string ShaderPath /*= ""*/)
{
	FShader* tmpShader = new FShader();
	do 
	{
		if (tmpShader->CompileShader(ShaderPath))
		{
			HRESULT hr = D3D11Info.D3D11Device->CreateVertexShader(tmpShader->ShaderBuffer->GetBufferPointer(),
				tmpShader->ShaderBuffer->GetBufferSize(), NULL, &tmpShader->VertexShader);
			if (FAILED(hr))
			{
				puts("During Create Vertex Shader:");
				HR(hr);
				break;
			}
			return tmpShader;
		}
	} while (0);
	printf("Failed to load Vertex Shader:%s\n", ShaderPath.c_str());
	delete tmpShader;
	return nullptr;
}

FShader* FShader::LoadPixelShader(const std::string ShaderPath /*= ""*/)
{
	FShader* tmpShader = new FShader();
	do
	{
		if (tmpShader->CompileShader(ShaderPath))
		{
			HRESULT hr = D3D11Info.D3D11Device->CreatePixelShader(tmpShader->ShaderBuffer->GetBufferPointer(),
				tmpShader->ShaderBuffer->GetBufferSize(), NULL, &tmpShader->PixelShader);
			if (FAILED(hr))
			{
				puts("During Create Pixel Shader:");
				HR(hr);
				break;
			}
			return tmpShader;
		}
	} while (0);
	printf("Failed to load Vertex Shader:%s\n", ShaderPath.c_str());
	delete tmpShader;
	return nullptr;
}
