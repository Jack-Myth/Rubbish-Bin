#include<stdio.h>
#include <string>
#include <windows.h>
#include <wincodec.h>
#include <d2d1.h>
#include "liblua/lua.hpp"
#include <locale.h>
#ifdef _DEBUG
#pragma comment(lib,"liblua_d.lib")
#else
#pragma comment(lib,"liblua.lib")
#endif
#pragma comment(lib, "d2d1.lib")

#define  CheckAndPostErrorMsg(X,...) if(!X){printf(...);exit(-1);}
#define  OUTPUT(...) if(ShowDebugOutput){printf(__VA_ARGS__);}
#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);
int IsKeyDown(lua_State * L);
int Lua_LoadImage(lua_State * L);
int InitD2D(lua_State * L);

bool ShowDebugOutput = false;
lua_State* _Env;
std::string TickFunctionName;
HWND ViewportHWND=NULL;
ID2D1Factory* D2D1Factory = NULL;
ID2D1HwndRenderTarget* D2D1HwndRenderTarget = NULL;
IWICImagingFactory* WICImagingFactory = NULL;
IDWriteTextFormat* BaseTextFormat = NULL;
UINT32 ViewportHeight=0, ViewportWidth=0;
int main(int argc, char* argv[])
{
	setlocale(LC_ALL, "");
	_Env = luaL_newstate();
	lua_register(_Env, "Quit", Lua_Exit);
	lua_register(_Env, "init_d2d", InitD2D);
	lua_register(_Env, "IsKeyDown", IsKeyDown);
	lua_register(_Env, "DrawImage", Lua_DrawImage);
	

	luaL_loadfile(_Env, "./main.lua");

	MSG msg = { NULL };
	while (msg.message != WM_QUIT)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		GetMessage(&msg, 0, 0, 0);
	}
	return 0;
}

int InitD2D(lua_State* L)
{
	//boolean init_d2d(int resX,int resY,string title,int fps,string TickFunctionName)
	int ResX = (int)luaL_checkinteger(L,1);
	int ResY = (int)luaL_checkinteger(L,2);
	const char* title = luaL_checkstring(L, 3);
	int fps = (int)luaL_checknumber(L, 4);
	TickFunctionName = luaL_checkstring(L, 5);
	//Begin Init Window for Game Viewport
	RECT WndRC = { 0,0,ResX,ResY };
	ViewportWidth = ResX;
	ViewportHeight = ResY;
	WNDCLASSEX wnd = { NULL };
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.hCursor = LoadCursor(0, IDC_ARROW);
	wnd.hIcon = LoadIcon(0, IDC_ICON);
	wnd.lpfnWndProc = MainWndProc;
	wnd.lpszClassName = TEXT("RAIDEN ENGINE VIEWPORT");
	RegisterClassEx(&wnd);
	AdjustWindowRect(&WndRC, WS_OVERLAPPEDWINDOW, FALSE);
	ViewportHWND = CreateWindow(wnd.lpszClassName, TEXT("RAIDEN ENGINE"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WndRC.right - WndRC.left, WndRC.bottom - WndRC.top, NULL, NULL, NULL, NULL);
	CheckAndPostErrorMsg(ViewportHWND, "Create Window as Viewport Failed,Exit...");
	ShowWindow(ViewportHWND,SW_NORMAL);
	//End Init Window For Game Viewport
	//Begin Init Direct2D
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &D2D1Factory);
	CheckAndPostErrorMsg(D2D1Factory, "D2D1Factory Failed to Create,Exit...");
	D2D1Factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT), D2D1::HwndRenderTargetProperties(ViewportHWND, D2D1::SizeU(WndRC.right - WndRC.left, WndRC.bottom - WndRC.top)), &D2D1HwndRenderTarget);
	CheckAndPostErrorMsg(!D2D1HwndRenderTarget, "D2D1 RenderTarget Failed to Create,Exit...");
	//End Init Direct2D
	//Begin Init WICImageComponent
	CoInitializeEx(NULL, COINIT_MULTITHREADED);
	CoCreateInstance(CLSID_WICImagingFactory, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&WICImagingFactory));
	CheckAndPostErrorMsg(WICImagingFactory,  "WICImageComponent Failed to Create,Exit...");
	//End Init WICImageComponent
	//Set Timer For FrameTick
	SetTimer(ViewportHWND, 0, 1000/fps, NULL);
	lua_pushboolean(L, true);
	return 1;
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
	switch (msg)
	{
	case WM_CLOSE:
		return 0;
	case WM_TIMER:
		//Call LUA Timer Function
		lua_getglobal(_Env, TickFunctionName.c_str());
		lua_pcall(_Env, 0, 0, NULL);
		return 1;
	}
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

int IsKeyDown(lua_State* L)
{
	int VK_Key = (int)luaL_checkinteger(L, 1);
	lua_pushboolean(L, KEY_DOWN(VK_Key));
	return 1;
}

bool LoadD2DImage(WCHAR* ImagePath, ID2D1Bitmap** BitmapRefrence)
{
	IWICBitmap* m_pWicBitmap = NULL;
	IWICBitmapDecoder* m_pWicDecoder = NULL;
	IWICBitmapFrameDecode* m_pWicFrameDecoder = NULL;
	if (FAILED(WICImagingFactory->CreateDecoderFromFilename(ImagePath, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &m_pWicDecoder)))
		return false;
	m_pWicDecoder->GetFrame(0, &m_pWicFrameDecoder);
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
	D2D1HwndRenderTarget->CreateBitmapFromWicBitmap(m_pWicBitmap, NULL, BitmapRefrence);
	pCovert->Release();
	m_pWicDecoder->Release();
	m_pWicFrameDecoder->Release();
	return true;
}

int Lua_LoadImage(lua_State* L)
{
	//Img LoadImage(string FilePath)
	OUTPUT("LoadImage:")
	std::string ImgPath = luaL_checkstring(L, 1);
	OUTPUT("%s......",ImgPath.c_str())
	WCHAR* ImgPathW = new WCHAR[ImgPath.length()+1];
	size_t converted;
	mbstowcs_s(&converted, ImgPathW,ImgPath.length(), ImgPath.c_str(),ImgPath.length());
	ID2D1Bitmap* Bitmap=nullptr;
	LoadD2DImage(ImgPathW,&Bitmap);
	delete[] ImgPathW;
	lua_pushlightuserdata(L, Bitmap);
	OUTPUT("OK\n");
	return 1;
}

int Lua_LoadImageInfo(lua_State* L)
{
	//ImageInfo LoadImageInfo(ImagePath)
	//ImageInfo {size{width,height},PixelData}
	std::string ImagePath = luaL_checkstring(L, 1);
	WCHAR* ImagePathW = new WCHAR[ImagePath.length()+1];
	size_t converted;
	mbstowcs_s(&converted, ImagePathW, ImagePath.length() + 1, ImagePath.c_str(), ImagePath.length() + 1);
	IWICBitmap* m_pWicBitmap = NULL;
	IWICBitmapDecoder* m_pWicDecoder = NULL;
	IWICBitmapFrameDecode* m_pWicFrameDecoder = NULL;
	if (FAILED(WICImagingFactory->CreateDecoderFromFilename(ImagePathW, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &m_pWicDecoder)))
		return false;
	m_pWicDecoder->GetFrame(0, &m_pWicFrameDecoder);
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

	UINT Width, Height;
	m_pWicBitmap->GetSize(&Width, &Height);
	auto* PixelDataBuffer = (BYTE*)lua_newuserdata(L, Width * Height * 4);
	WICRect rclock = { 0,0,Width,Height };
	IWICBitmapLock* WICLock;
	m_pWicBitmap->Lock(&rclock, WICBitmapLockRead, &WICLock);
	UINT BufferSize;
	BYTE* pbyte;
	WICLock->GetDataPointer(&BufferSize, &pbyte);
	memcpy_s(PixelDataBuffer, Width * Height * 4, pbyte, Width * Height * 4);
	lua_createtable(L, 2, 0);
	lua_pushstring(L, "size");
	lua_createtable(L, 2, 0);
	lua_pushstring(L, "width");
	lua_pushinteger(L, Width);
	lua_settable(L, -3);
	lua_pushstring(L, "height");
	lua_pushinteger(L, Height);
	lua_settable(L, -3);
	lua_settable(L, -3);
	lua_pushstring(L, "PixelData");
	lua_pushlightuserdata(L, PixelDataBuffer);
	lua_settable(L, -3);
	return 1;
}

int Lua_DrawImage(lua_State* L)
{
	//void DrawImage(Img image,Table Rect)
	D2D1_RECT_F rect;
	auto* d2dimg = (ID2D1Bitmap*)lua_touserdata(L, 1);
	lua_geti(L, 2, 1);
	rect.left = (float)lua_tonumber(L, -1) * ViewportWidth;
	lua_geti(L, 2, 2);
	rect.top = (float)lua_tonumber(L, -1) * ViewportHeight;
	lua_geti(L, 2, 3);
	rect.right = (float)lua_tonumber(L, -1) * ViewportHeight;
	lua_geti(L, 2, 4); 
	rect.bottom = (float)lua_tonumber(L, -1) * ViewportHeight;
	lua_pop(L, 4);
	float opacity = (float)luaL_checknumber(L, 3);
	D2D1HwndRenderTarget->DrawBitmap(d2dimg, rect, opacity);
	return 0;
}

int Lua_DrawRectangle(lua_State* L)
{
	//void DrawRectangle(Rect,Color,bHollow,StrokeWidth)
	
	D2D1_RECT_F rect;
	D2D1_COLOR_F color;
	lua_geti(L, 1, 1);
	rect.left = lua_tonumber(L, -1)*ViewportWidth;
	lua_geti(L, 1, 2);
	rect.top = lua_tonumber(L, -1) * ViewportHeight;
	lua_geti(L, 1, 3);
	rect.right = lua_tonumber(L, -1)* ViewportWidth;
	lua_geti(L, 1, 4);
	rect.bottom = lua_tonumber(L, -1)* ViewportHeight;
	lua_pop(L, 4);
	lua_getfield(L, 2, "r");
	lua_getfield(L, 2, "g");
	lua_getfield(L, 2, "b");
	lua_getfield(L, 2, "a");
	color.r = lua_tointeger(L, -4) / 255.f;
	color.g = lua_tointeger(L, -3) / 255.f;
	color.b = lua_tointeger(L, -2) / 255.f;
	color.a = lua_tointeger(L, -1) / 255.f;
	lua_pop(L, 4);
	bool Hollow = false;
	float StrokeWidth = 1.f;
	if (lua_gettop(L) > 2)
	{
		Hollow = lua_toboolean(L, 3);
		if (Hollow)
			StrokeWidth = (float)luaL_checknumber(L, 4);
	}
	ID2D1SolidColorBrush* colorBrush = nullptr;
	D2D1HwndRenderTarget->CreateSolidColorBrush(color, &colorBrush);
	if (Hollow)
		D2D1HwndRenderTarget->DrawRectangle(rect, colorBrush,StrokeWidth);
	else 
		D2D1HwndRenderTarget->FillRectangle(rect, colorBrush);
	colorBrush->Release();
}

int Lua_DrawEclipse(lua_State* L)
{
	//void DrawRectangle(Rect,Color,bHollow,StrokeWidth)

	D2D1_RECT_F rect;
	D2D1_COLOR_F color;
	lua_geti(L, 1, 1);
	rect.left = lua_tonumber(L, -1) * ViewportWidth;
	lua_geti(L, 1, 2);
	rect.top = lua_tonumber(L, -1) * ViewportHeight;
	lua_geti(L, 1, 3);
	rect.right = lua_tonumber(L, -1) * ViewportWidth;
	lua_geti(L, 1, 4);
	rect.bottom = lua_tonumber(L, -1) * ViewportHeight;
	lua_pop(L, 4);
	D2D1_ELLIPSE eclipse;
	eclipse.point.x = (rect.right - rect.left)*0.5+rect.left;
	eclipse.point.y = (rect.bottom - rect.top)*0.5+rect.top;
	eclipse.radiusX = (rect.right - rect.left) * 0.5;
	eclipse.radiusY = (rect.bottom - rect.top) * 0.5;
	lua_getfield(L, 2, "r");
	lua_getfield(L, 2, "g");
	lua_getfield(L, 2, "b");
	lua_getfield(L, 2, "a");
	color.r = lua_tointeger(L, -4) / 255.f;
	color.g = lua_tointeger(L, -3) / 255.f;
	color.b = lua_tointeger(L, -2) / 255.f;
	color.a = lua_tointeger(L, -1) / 255.f;
	lua_pop(L, 4);
	bool Hollow = false;
	float StrokeWidth = 1.f;
	if (lua_gettop(L) > 2)
	{
		Hollow = lua_toboolean(L, 3);
		if (Hollow)
			StrokeWidth = (float)luaL_checknumber(L, 4);
	}
	ID2D1SolidColorBrush* colorBrush = nullptr;
	D2D1HwndRenderTarget->CreateSolidColorBrush(color, &colorBrush);
	if (Hollow)
		D2D1HwndRenderTarget->DrawEllipse(eclipse, colorBrush,StrokeWidth);
	else
		D2D1HwndRenderTarget->FillEllipse(eclipse, colorBrush);
	colorBrush->Release();
}

int Lua_GenImageFromBuffer(lua_State* L)
{
	//Image GenImageFromBuffer(DataBuffer, Width, Height)
	 int Width = (int)luaL_checkinteger(L, 2);
	 int Height = (int)luaL_checkinteger(L, 3);
	 unsigned char* Buffer = new unsigned char[Width * Height * 4];
	for (int y=0;y<Height;y++)
	{
		for (int x=1;x<=Width;x++)
		{
			lua_geti(L, 1, y * Width + x);
			auto xxxxrgba = lua_tointeger(L, -1);
			int PixelIndex = 4 * (y * Width + x - 1);
			//Convert to BGRA
			Buffer[PixelIndex+2] = (unsigned char)(xxxxrgba & 0xFF);
			xxxxrgba <<= 2;
			Buffer[PixelIndex+1] = (unsigned char)(xxxxrgba & 0xFF);
			xxxxrgba <<= 2;
			Buffer[PixelIndex] = (unsigned char)(xxxxrgba & 0xFF);
			xxxxrgba <<= 2;
			Buffer[PixelIndex] = (unsigned char)(xxxxrgba & 0xFF);
			xxxxrgba <<= 2;
		}
		lua_pop(L, Width);
	}
	IWICBitmap* bitmap = nullptr;
	WICImagingFactory->CreateBitmapFromMemory(Width, Height,
		GUID_WICPixelFormat32bppPBGRA,
		Width * 4,
		Width * Height * 4,Buffer, &bitmap);
	ID2D1Bitmap* d2d1bitmap=nullptr;
	D2D1HwndRenderTarget->CreateBitmapFromWicBitmap(bitmap, &d2d1bitmap);
	lua_pushlightuserdata(L, d2d1bitmap);
	return 1;
}

int Lua_GetPixel(lua_State* L)
{
	//RGBA GetPixel(ImgInfo,{X,Y})
	lua_getfield(L,1,"PixelData");
	auto* PixelData = (char*)lua_touserdata(L, -1);
	lua_geti(L, 2, 1);
	int X = (int)lua_tointeger(L, -1);
	lua_geti(L, 2, 2);
	int Y = (int)lua_tointeger(L, -1);
	lua_pop(L, 3);
	lua_createtable(L, 4, 0);
	int tableIndex = lua_gettop(L);
	lua_pushstring(L, "b");
	lua_pushinteger(L, *(unsigned char*)PixelData);
	lua_settable(L, tableIndex);
	lua_pushstring(L, "g");
	lua_pushinteger(L, *(unsigned char*)(PixelData + 1));
	lua_settable(L, tableIndex);
	lua_pushstring(L, "r");
	lua_pushinteger(L, *(unsigned char*)(PixelData + 2));
	lua_settable(L, tableIndex);
	lua_pushstring(L, "a");
	lua_pushinteger(L, *(unsigned char*)(PixelData + 3));
	lua_settable(L, tableIndex);
	return 1;
}

int Lua_Exit(lua_State* L)
{
	printf("Lua Instruction, Exit...");
	exit(0);
}