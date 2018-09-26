#pragma once
#include <d3d11.h>
#include <d3dcommon.h>
#include <DirectXMath.h>
#if _DEBUG
#define WAIT_EXIT system("pause >nul")
#else
#define WAIT_EXIT
#endif
#if defined(DEBUG) | defined(_DEBUG)
#ifndef HR
#define HR(x)									\
	{									\
		HRESULT __hr = (x);						\
		LPSTR output;                                    	\
		FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM |		\
			FORMAT_MESSAGE_IGNORE_INSERTS 	 |		\
			FORMAT_MESSAGE_ALLOCATE_BUFFER,			\
			NULL,						\
			__hr,						\
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	\
			(LPSTR) &output,				\
			0,						\
			NULL);					        \
		puts(output);		\
	}
#endif
#else
#ifndef HR
#define HR(x) (x)
#endif
#endif 

struct FD3D11Info
{
	ID3D11Device* D3D11Device;
	IDXGISwapChain* DXGISwapChain;
	ID3D11DeviceContext* D3D11DeviceContext;
	D3D_FEATURE_LEVEL D3DFeatureLevel;
	ID3D11RenderTargetView* RenderTargetView;
	ID3D11Buffer* D3DMVPBuffer;
};

struct FMVPBuffer
{
	DirectX::XMMATRIX ModelMatrix;
	DirectX::XMMATRIX ViewMatrix;
	DirectX::XMMATRIX ProjectionMatrix;
};
extern FD3D11Info D3D11Info;
extern FMVPBuffer MVPBuffer;