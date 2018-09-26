#include "Shader.h"
#include "DxInfo.h"
#include "D3Dcompiler.h"
#include <sstream>
#pragma comment(lib,"D3DCompiler")

bool FShader::CompileShader(const std::string& ShaderPath,LPCSTR pEntryFunction,LPCSTR pTarget)
{
	std::wstringstream wss;
	wss << ShaderPath.c_str();
	ID3DBlob* ErrorMsg=nullptr;
	HRESULT hr = D3DCompileFromFile(wss.str().c_str(), nullptr, nullptr, pEntryFunction, pTarget, NULL, NULL, &ShaderBuffer, &ErrorMsg);
	if (FAILED(hr))
	{
		printf("In Shader:%s:\n", ShaderPath.c_str());
		HR(hr);
		if (ErrorMsg->GetBufferPointer())
			printf("%s\n", (char*)ErrorMsg->GetBufferPointer());
		ErrorMsg->Release();
		return false;
	}
	ErrorMsg->Release();
	return true;
}

FShader* FShader::LoadVertexShader(const std::string& ShaderPath,LPCSTR pEntryFunction)
{
	FShader* tmpShader = new FShader();
	do 
	{
		if (tmpShader->CompileShader(ShaderPath, pEntryFunction,"vs_5_0"))
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

FShader* FShader::LoadPixelShader(const std::string& ShaderPath,LPCSTR pEntryFunction)
{
	FShader* tmpShader = new FShader();
	do
	{
		if (tmpShader->CompileShader(ShaderPath, pEntryFunction,"ps_5_0"))
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
