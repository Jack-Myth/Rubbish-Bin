#pragma once
#include <d2d1.h>
#include <wincodec.h>
#include <windows.h>
#include <iostream>

extern struct ID2D1Factory* D2D1Factory;
extern struct ID2D1HwndRenderTarget* D2D1HwndRenderTarget;
extern struct IWICImagingFactory* WICImagingFactory;
extern HWND DesktopHwnd;
extern int ScreenWidth;
extern int ScreenHeight;

inline void InitD2D()
{
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &D2D1Factory);
	if (!D2D1Factory)
	{
		MessageBox(NULL, TEXT("初始化Direct2D失败，无法创建上下文"), TEXT("Dynamic Desktop LineAndCircle"), MB_OK);
		exit(-1);
	}
	CoInitialize(NULL);
	CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&WICImagingFactory));
	D2D1Factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM)),
		D2D1::HwndRenderTargetProperties(DesktopHwnd, D2D1::SizeU(ScreenWidth, ScreenHeight)), &D2D1HwndRenderTarget);
	if (!D2D1HwndRenderTarget)
	{
		MessageBox(NULL, TEXT("初始化Direct2D失败，无法创建渲染目标"), TEXT("Dynamic Desktop LineAndCircle"), MB_OK);
		exit(-1);
	}
}

inline ID2D1Bitmap* LoadD2DImage_Internal(IWICBitmapDecoder* m_pWicDecoder, UINT* Width = 0, UINT* Height = 0)
{
	IWICBitmap* m_pWicBitmap = NULL;
	IWICBitmapFrameDecode* m_pWicFrameDecoder = NULL;
	m_pWicDecoder->GetFrame(0, &m_pWicFrameDecoder);
	if (Width && Height)
		m_pWicFrameDecoder->GetSize(Width, Height);
	IWICBitmapSource* pWicSource = NULL;
	m_pWicFrameDecoder->QueryInterface(IID_PPV_ARGS(&pWicSource));
	IWICFormatConverter* pCovert = NULL;
	WICImagingFactory->CreateFormatConverter(&pCovert);
	pCovert->Initialize(
		pWicSource,
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		NULL,
		0.f,
		WICBitmapPaletteTypeCustom
	);
	WICImagingFactory->CreateBitmapFromSource(pCovert, WICBitmapCacheOnDemand, &m_pWicBitmap);
	ID2D1Bitmap* d2d1Bitmap = NULL;
	D2D1HwndRenderTarget->CreateBitmapFromWicBitmap(m_pWicBitmap, NULL, &d2d1Bitmap);
	pCovert->Release();
	m_pWicDecoder->Release();
	return d2d1Bitmap;
}

inline ID2D1Bitmap* LoadD2DImage(const WCHAR ImagePath[], UINT* Width = 0, UINT* Height = 0)
{
	IWICBitmapDecoder* m_pWicDecoder = NULL;
	IWICBitmapFrameDecode* m_pWicFrameDecoder = NULL;
	if (FAILED(WICImagingFactory->CreateDecoderFromFilename(ImagePath, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &m_pWicDecoder)))
		return nullptr;
	auto* bitmap = LoadD2DImage_Internal(m_pWicDecoder, Width, Height);
	m_pWicDecoder->Release();
	return bitmap;
}

inline ID2D1Bitmap* LoadD2DImageFromResource(HRSRC hRresource, UINT* Width = 0, UINT* Height = 0)
{
	IWICBitmapDecoder* m_pWicDecoder = NULL;
	IStream* tmpStream;
	auto hG = LoadResource(GetModuleHandle(NULL), hRresource);
	LPVOID pData = LockResource(hG);
	DWORD resourceSize= SizeofResource(GetModuleHandle(NULL), hRresource);
	HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, resourceSize);
	LPVOID pMemData = GlobalLock(hMem);
	memcpy(pMemData, pData, resourceSize);
	UnlockResource(hG);
	GlobalUnlock(hMem);
	CreateStreamOnHGlobal(hMem, true, &tmpStream);
	if (FAILED(WICImagingFactory->CreateDecoderFromStream(tmpStream, nullptr, WICDecodeMetadataCacheOnDemand, &m_pWicDecoder)))
		return nullptr;
	auto* bitmap= LoadD2DImage_Internal(m_pWicDecoder, Width, Height);
	GlobalFree(hMem);
	m_pWicDecoder->Release();
	return bitmap;
}

inline void ReleaseD2D()
{
	WICImagingFactory->Release();
	D2D1HwndRenderTarget->Release();
	D2D1Factory->Release();
}

inline float GetDistance(D2D1_POINT_2F A, D2D1_POINT_2F B)
{
	float x = A.x - B.x;
	float y = A.y - A.y;
	return sqrtf(x * x + y * y);
}

inline ID2D1Bitmap* GetDesktopBackgroundBitmap()
{
	WCHAR UserNameBuffer[256];
	DWORD UserNameLength = 256;
	GetUserName(UserNameBuffer, &UserNameLength);
	std::wstring DesktopBGFolder = TEXT("C:\\Users\\") + std::wstring(UserNameBuffer) +
		TEXT("\\AppData\\Roaming\\Microsoft\\Windows\\Themes\\CachedFiles\\");

	WIN32_FIND_DATA findData = { NULL };
	FindFirstFile((DesktopBGFolder + TEXT("CachedImage*")).c_str(), &findData);
	if (std::wstring(findData.cFileName) != TEXT(""))
		return LoadD2DImage((DesktopBGFolder + findData.cFileName).c_str());
	return nullptr;
}