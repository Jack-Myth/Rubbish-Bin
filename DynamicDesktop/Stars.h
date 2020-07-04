#pragma once
#include "IModelBase.h"
#include "D2DUtils.h"
#include <vector>
#include <algorithm>
#include <queue>
#include "resource.h"


extern HWND DesktopHwnd;
extern struct ID2D1Factory* D2D1Factory;
extern struct ID2D1HwndRenderTarget* D2D1HwndRenderTarget;
extern struct IWICImagingFactory* WICImagingFactory;
extern int ScreenWidth;
extern int ScreenHeight;
class Stars : public IModelBase
{
	struct Star
	{
		D2D1_POINT_2F Position;
		D2D1_POINT_2F Velocity;
		float Size;
		float LifeTime=0;
		float Rotation;
		float RotSpeed;
	};
	ID2D1Bitmap* DesktopBackground = NULL;
	ID2D1Bitmap* IMG_Star = NULL, *IMG_CStar = NULL;
	D2D1_POINT_2F IMG_StarHalfSize, IMG_CStarHalfSize;
	bool IsMouseDown=false;
	std::vector<Star> StarsQueue;
	std::vector<Star> ColorfulStarsQueue;
	int LastTryExitTime = 0;
	int TryExitCount = 0;
	ID2D1Bitmap* LoadBitmapFromResource(LPCWSTR Type,LPCWSTR Name,D2D1_POINT_2F* Size)
	{
		auto hRresource = FindResource(GetModuleHandle(NULL), Name, Type);
		UINT Width, Height;
		auto* bitmap = LoadD2DImageFromResource(hRresource,&Width,&Height);
		Size->x = (float)Width;
		Size->y = (float)Height;
		return bitmap;
	}
public:
	float DefSize = 5;
	float SizeRandomMax = 5;
	int RotSpeedMax = 6;
	void StartupModel() override
	{
		InitD2D();
		DesktopBackground = GetDesktopBackgroundBitmap();
		IMG_Star = LoadBitmapFromResource(TEXT("PNG"), MAKEINTRESOURCE(IDB_STAR),&IMG_StarHalfSize);
		IMG_StarHalfSize.x *= 0.5f;
		IMG_StarHalfSize.y *= 0.5f;
		IMG_CStar = LoadBitmapFromResource(TEXT("PNG"), MAKEINTRESOURCE(IDB_CSTAR), &IMG_CStarHalfSize);
		IMG_CStarHalfSize.x *= 0.5f;
		IMG_CStarHalfSize.y *= 0.5f;
	}


	void ShutdownModel() override
	{
		DesktopBackground->Release();
		ReleaseD2D();
	}

	void Render()
	{
		D2D1HwndRenderTarget->BeginDraw();
		D2D1HwndRenderTarget->SetTransform(D2D1::Matrix3x2F::Identity());
		//Draw DesktopBackground
		D2D1HwndRenderTarget->DrawBitmap(DesktopBackground, D2D1::RectF(0, 0,
			(float)ScreenWidth, (float)ScreenHeight), 1,
			D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
		//Draw small star
		for (Star& star:StarsQueue)
		{
			D2D1HwndRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(star.Rotation, star.Position));
			D2D1_RECT_F rect = D2D1::RectF(star.Position.x - star.Size,
				star.Position.y - star.Size,
				star.Position.x + star.Size,
				star.Position.y + star.Size);
			D2D1HwndRenderTarget->DrawBitmap(IMG_Star, rect, 1.f - star.LifeTime, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
		}
		//Draw Big Colorful Star
		for (Star& star : ColorfulStarsQueue)
		{
			D2D1HwndRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(star.Rotation, star.Position));
			D2D1_RECT_F rect = D2D1::RectF(star.Position.x - star.Size,
				star.Position.y - star.Size,
				star.Position.x + star.Size,
				star.Position.y + star.Size);
			D2D1HwndRenderTarget->DrawBitmap(IMG_CStar, rect, 1.f - star.LifeTime, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);
		}
		D2D1HwndRenderTarget->EndDraw();
	}

	void Tick(float delta) override
	{
		LastTryExitTime += (int)(delta * 1000);
		//Create new Star
		POINT CursorPos;
		GetCursorPos(&CursorPos);
		for (int i = 0; i < 10; i++)
		{
			Star tmpStar;
			tmpStar.Position.x = (float)CursorPos.x;
			tmpStar.Position.y = (float)CursorPos.y;
			tmpStar.Rotation = (float)(rand() % 360);
			tmpStar.Velocity = D2D1::Point2F(float(rand() % 1000 - 500), float(rand() % 1000 - 500));
			float VelocityLength = tmpStar.Velocity.x * tmpStar.Velocity.x + tmpStar.Velocity.y * tmpStar.Velocity.y;
			VelocityLength = sqrtf(VelocityLength);
			tmpStar.Velocity.x = tmpStar.Velocity.x / VelocityLength * 3;
			tmpStar.Velocity.y = tmpStar.Velocity.y / VelocityLength * 3;
			tmpStar.Size = DefSize+ rand() % 1000 / 1000.f * SizeRandomMax;
			tmpStar.RotSpeed = (float)(rand() % 2 * RotSpeedMax - RotSpeedMax);
			StarsQueue.push_back(tmpStar);
		}
		if (IsMouseDown)
		{
			Star tmpStar;
			tmpStar.Position.x = (float)CursorPos.x;
			tmpStar.Position.y = (float)CursorPos.y;
			tmpStar.Rotation = 0;
			tmpStar.Velocity = D2D1::Point2F(float(rand() % 1000 - 500), float(rand() % 1000 - 500));
			float VelocityLength = tmpStar.Velocity.x * tmpStar.Velocity.x + tmpStar.Velocity.y * tmpStar.Velocity.y;
			VelocityLength = sqrtf(VelocityLength);
			tmpStar.Velocity.x = tmpStar.Velocity.x / VelocityLength * 3;
			tmpStar.Velocity.y = tmpStar.Velocity.y / VelocityLength * 3;
			tmpStar.Size = DefSize*5 + rand() % 1000 / 1000.f * SizeRandomMax;
			tmpStar.RotSpeed = (float)(rand() % (RotSpeedMax/2));
			ColorfulStarsQueue.push_back(tmpStar);
		}
		Render();
		for (Star& starItem : StarsQueue)
		{
			starItem.Position.x += starItem.Velocity.x;
			starItem.Position.y += starItem.Velocity.y;
			starItem.Rotation += starItem.RotSpeed;
			starItem.LifeTime += delta;
		}
		for (Star& starItem : ColorfulStarsQueue)
		{
			starItem.Position.x += starItem.Velocity.x;
			starItem.Position.y += starItem.Velocity.y;
			starItem.Rotation += starItem.RotSpeed;
			starItem.LifeTime += delta * 0.5f;
		}
		int Block = 0;
		for (size_t i = 0; i < StarsQueue.size(); i++)
		{
			if (StarsQueue[i].LifeTime <= 1.f)
				continue;
			else
				Block = i;
		}
		if (Block)
			StarsQueue.erase(StarsQueue.begin(), StarsQueue.begin() + Block);
		Block = 0;
		for (size_t i = 0; i < ColorfulStarsQueue.size(); i++)
		{
			if (ColorfulStarsQueue[i].LifeTime <= 1.f)
				continue;
			else
				Block = i;
		}
		if (Block)
			ColorfulStarsQueue.erase(ColorfulStarsQueue.begin(), ColorfulStarsQueue.begin() + Block);
	}


	void OnMouseDown(int VKey,bool IsOnDesktop) override
	{
		if (!IsOnDesktop)
			return;
		switch (VKey)
		{
		case VK_LBUTTON:
			IsMouseDown = true;
			break;
		case VK_MBUTTON:
			if (LastTryExitTime < 1000)
				TryExitCount += 1;
			else
				TryExitCount = 0;
			LastTryExitTime = 0;
			if (TryExitCount >= 5)
			{
				MessageBox(NULL, TEXT("Designed By JackMyth"), TEXT("Dynamic Desktop"), MB_OK);
				exit(0);
			}
			break;
		default:
			break;
		}
	}


	void OnMouseUp(int VKey) override
	{
		switch (VKey)
		{
		case VK_LBUTTON:
			IsMouseDown = false;
		default:
			break;
		}
	}

};