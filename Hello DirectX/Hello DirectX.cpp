#include "SDL2/SDL.h"
#include "SDL2/SDL_syswm.h"
#include <windows.h>
#include <stdio.h>
#include <d3d11.h>
#include <time.h>
#include <DirectXMath.h>
#include "DxInfo.h"
#include <commdlg.h>
#include "Model.h"
#include "Shader.h"
#pragma comment(lib,"d3d11.lib")
#pragma comment(lib,"assimp-vc140-mt.lib")


LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
bool InitD3D(HINSTANCE hInstance);
void ReleaseObjects();
void LoadShader();
void BuildScene();
void UpdateScene();
void RenderScene();

FD3D11Info D3D11Info;
SDL_Window* pMainWindow=nullptr;
HWND MainWindowHwnd=NULL;
DirectX::XMFLOAT4 ScreenColor;
RECT WindowRect = {0,0,1280,720};
FModel* targetModel;
FShader* DefVertShader, *DefPixelShader;
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
	LoadShader();
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

void LoadShader()
{
	DefVertShader = FShader::LoadVertexShader("Effects.fx", "VS");
	DefPixelShader = FShader::LoadPixelShader("Effects.fx", "PS");
}

void BuildScene()
{
	ScreenColor.x = 0.f;
	ScreenColor.y = 0.f;
	ScreenColor.z = 0.f;
	ScreenColor.w = 0.f;
	char FileP[1024] = { 0 };
	OPENFILENAMEA OpenFN = { NULL };
	OpenFN.lStructSize = sizeof(OPENFILENAMEA);
	OpenFN.Flags = OFN_FILEMUSTEXIST;
	OpenFN.lpstrFilter = "模型文件\0*.obj;*.fbx;*.3ds\0\0";
	OpenFN.nMaxFile = MAX_PATH;
	OpenFN.lpstrFile = FileP;
	OpenFN.hInstance = GetModuleHandle(NULL);
	OpenFN.lpstrTitle = "选择模型文件";
	if (GetOpenFileNameA(&OpenFN))
	{
		targetModel = FModel::LoadModel(FileP);
		if (targetModel)
		{
			FTransform mTransform = targetModel->GetTransform();
			mTransform.Scale = DirectX::XMFLOAT3(0.1f, 0.1f, 0.1f);
			targetModel->SetTransform(mTransform);
		}
	}
}

void UpdateScene()
{
	ScreenColor.x = (float)sin((double)clock() / CLOCKS_PER_SEC)*0.5f + 0.5f;
	ScreenColor.y = (float)sin((double)clock() / CLOCKS_PER_SEC)*0.5f + 0.5f;
	ScreenColor.z = (float)sin((double)clock() / CLOCKS_PER_SEC)*0.5f + 0.5f;
}

void RenderScene()
{
	D3D11Info.D3D11DeviceContext->ClearRenderTargetView(D3D11Info.RenderTargetView, (float*)&ScreenColor);
	if (targetModel)
		targetModel->Render(DefVertShader, DefPixelShader);
	D3D11Info.DXGISwapChain->Present(0, NULL);
}

void ReleaseObjects()
{
	D3D11Info.D3D11Device->Release();
	D3D11Info.D3D11DeviceContext->Release();
	D3D11Info.DXGISwapChain->Release();
}