#include "SDL2/SDL.h"
#include "SDL2/SDL_syswm.h"
#include <windows.h>
#include <stdio.h>
#include <d3d11.h>
#include <time.h>
#include <d3d9types.h>
#include <DirectXMath.h>
#pragma comment(lib,"d3d11.lib")
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
		if(FAILED(__hr))							\
		{								\
			LPWSTR output;                                    	\
			FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM |		\
				FORMAT_MESSAGE_IGNORE_INSERTS 	 |		\
				FORMAT_MESSAGE_ALLOCATE_BUFFER,			\
				NULL,						\
				__hr,						\
				MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),	\
				(LPTSTR) &output,				\
				0,						\
				NULL);					        \
			MessageBox(NULL, output, L"Error", MB_OK);		\
		}								\
	}
#endif
#else
#ifndef HR
#define HR(x) (x)
#endif
#endif 

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
bool InitD3D(HINSTANCE hInstance);
void ReleaseObjects();
void BuildScene();
void UpdateScene();
void RenderScene();

struct FD3D11Info
{
	ID3D11Device* D3D11Device;
	IDXGISwapChain* DXGISwapChain;
	ID3D11DeviceContext* D3D11DeviceContext;
	D3D_FEATURE_LEVEL D3DFeatureLevel;
	ID3D11RenderTargetView* RenderTargetView;
} D3D11Info;

SDL_Window* pMainWindow=nullptr;
HWND MainWindowHwnd=NULL;
DirectX::XMFLOAT4 ScreenColor;
RECT WindowRect = {0,0,1280,720};
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
#if _DEBUG
	{
		AllocConsole();
		FILE* tmpDebugFile;
		freopen_s(&tmpDebugFile, "CONOUT$", "w", stdout);
	}
#endif
	WNDCLASSEX wnd = { NULL };
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wnd.hCursor = LoadCursor(0, IDC_ARROW);
	wnd.hIcon = LoadIcon(0, IDC_ICON);
	wnd.hInstance = hInstance;
	wnd.lpfnWndProc = MainWndProc;
	wnd.lpszClassName = TEXT("GalEngine Blank Win32 Window");
	wnd.style = CS_VREDRAW || CS_HREDRAW;
	RegisterClassEx(&wnd);
	AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, false);
	MainWindowHwnd = CreateWindow(wnd.lpszClassName,TEXT("Hello DirectX!"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, NULL, NULL, hInstance, NULL);
	if (MainWindowHwnd)
		ShowWindow(MainWindowHwnd,nShowCmd);
	else
	{
		MessageBox(NULL, TEXT("Unable to create Window,Exit!"),TEXT("Error"),MB_OK);
		return -1;
	}
	if (!InitD3D(hInstance))
	{
		MessageBox(MainWindowHwnd, TEXT("DirectX Init Failed,Exit!"), TEXT("Error"), MB_OK);
		return-1;
	}
	BuildScene();
	MSG msg = { NULL };
	while (msg.message!=WM_QUIT)
	{
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else
		{
			UpdateScene();
			RenderScene();
		}
	}
	ReleaseObjects();
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CLOSE:
			PostQuitMessage(0);
			break;
		default:
			break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

bool InitD3D(HINSTANCE hInstance)
{
	HRESULT hr;
	DXGI_MODE_DESC backbufferDesc = { NULL };
	backbufferDesc.Height = 720;
	backbufferDesc.Width = 1280;
	backbufferDesc.RefreshRate.Numerator = 60;
	backbufferDesc.RefreshRate.Denominator = 1;
	backbufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	backbufferDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;
	backbufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	DXGI_SWAP_CHAIN_DESC swapchainDesc = { NULL };
	swapchainDesc.SampleDesc.Count = 1;
	swapchainDesc.SampleDesc.Quality = 0;
	swapchainDesc.BufferCount = 1;
	swapchainDesc.BufferDesc = backbufferDesc;
	swapchainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapchainDesc.OutputWindow = MainWindowHwnd;
	swapchainDesc.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	swapchainDesc.Windowed = true;
	swapchainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	//swapchainDesc.SampleDesc will be NULL
	hr=D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, NULL, NULL, NULL, D3D11_SDK_VERSION,
		&swapchainDesc, &D3D11Info.DXGISwapChain, &D3D11Info.D3D11Device, NULL, &D3D11Info.D3D11DeviceContext);
	if (FAILED(hr))
	{
		HR(hr);
		return false;
	}
	ID3D11Texture2D* BackBufferTexture;
	D3D11Info.DXGISwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&BackBufferTexture);
	D3D11Info.D3D11Device->CreateRenderTargetView(BackBufferTexture, NULL, &D3D11Info.RenderTargetView);
	BackBufferTexture->Release();
	D3D11Info.D3D11DeviceContext->OMSetRenderTargets(1, &D3D11Info.RenderTargetView, nullptr);
	return true;
}

void BuildScene()
{
	ScreenColor.x = 0.f;
	ScreenColor.y = 0.f;
	ScreenColor.z = 0.f;
	ScreenColor.w = 0.f;
}

void UpdateScene()
{
	ScreenColor.x = sin((double)clock() / CLOCKS_PER_SEC)*0.5 + 0.5;
	ScreenColor.y = sin((double)clock() / CLOCKS_PER_SEC)*0.5 + 0.5;
	ScreenColor.z = sin((double)clock() / CLOCKS_PER_SEC)*0.5 + 0.5;
}

void RenderScene()
{
	D3D11Info.D3D11DeviceContext->ClearRenderTargetView(D3D11Info.RenderTargetView, (float*)&ScreenColor);
	D3D11Info.DXGISwapChain->Present(0, NULL);
}

void ReleaseObjects()
{
	D3D11Info.D3D11Device->Release();
	D3D11Info.D3D11DeviceContext->Release();
	D3D11Info.DXGISwapChain->Release();
}