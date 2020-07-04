#pragma once
#include "IModelBase.h"
#include "D2DUtils.h"
#include <vector>
#include <algorithm>

extern HWND DesktopHwnd;
extern struct ID2D1Factory* D2D1Factory;
extern struct ID2D1HwndRenderTarget* D2D1HwndRenderTarget;
extern struct IWICImagingFactory* WICImagingFactory;
extern int ScreenWidth;
extern int ScreenHeight;
class LineAndCircle : public IModelBase
{
	struct Circle
	{
		float Size;
		D2D1_POINT_2F Position;
		D2D1_POINT_2F Velocity;
		D2D1_COLOR_F Color;
	};
	ID2D1Bitmap* DesktopBackground = NULL;
	std::vector<Circle> CirclesCollection;
	float CircleSize = 10;
	D2D1_COLOR_F CircleColor=D2D1::ColorF(0.4f,0.6f,1.f);
public:
	float LineWidth=1.f;
	void StartupModel() override
	{
		//Init D2D
		InitD2D();
		DesktopBackground = GetDesktopBackgroundBitmap();
		//Init Graphics Structure
		for (int i=0;i<20;i++)
		{
			Circle tmpCircle;
			tmpCircle.Position = D2D1::Point2F((float)(rand() % ScreenWidth), float(rand() % ScreenHeight));
			tmpCircle.Velocity= D2D1::Point2F(float(rand() % 1000 - 500),float( rand() % 1000 - 500));
			float VelocityLength = tmpCircle.Velocity.x * tmpCircle.Velocity.x + tmpCircle.Velocity.y * tmpCircle.Velocity.y;
			VelocityLength=sqrtf(VelocityLength);
			tmpCircle.Velocity.x = tmpCircle.Velocity.x / VelocityLength*3;
			tmpCircle.Velocity.y = tmpCircle.Velocity.y / VelocityLength*3;
			tmpCircle.Size = CircleSize;
			tmpCircle.Color = CircleColor;
			CirclesCollection.push_back(tmpCircle);
		}
	}

	void SetCircleSize(float NewSize)
	{
		CircleSize = NewSize;
		for (size_t i = 0; i < CirclesCollection.size(); i++)
			CirclesCollection[i].Size = CircleSize;
	}

	void SetCircleColor(D2D1_COLOR_F NewColor)
	{
		CircleColor = NewColor;
		for (size_t i = 0; i < CirclesCollection.size(); i++)
			CirclesCollection[i].Color = CircleColor;
	}

	void ShutdownModel() override
	{
		DesktopBackground->Release();
		ReleaseD2D();
	}

	void Render()
	{
		D2D1HwndRenderTarget->BeginDraw();
		//DrawBackground
		D2D1HwndRenderTarget->DrawBitmap(DesktopBackground, D2D1::RectF(0,0,
			(float)ScreenWidth, (float)ScreenHeight), 1,
			D2D1_BITMAP_INTERPOLATION_MODE_LINEAR);

		POINT CursorPos;
		GetCursorPos(&CursorPos);
		float ScreenMin = (float)min(ScreenHeight, ScreenWidth);
		for (size_t x = 0; x < CirclesCollection.size(); x++)
		{
			for (size_t y=x+1;y<CirclesCollection.size();y++)
			{
				if (GetDistance(CirclesCollection[x].Position,CirclesCollection[y].Position)> ScreenMin/3.f)
					continue;
				float DisA = GetDistance(D2D1::Point2F((float)CursorPos.x, (float)CursorPos.y), CirclesCollection[x].Position);
				float DisB = GetDistance(D2D1::Point2F((float)CursorPos.x, (float)CursorPos.y), CirclesCollection[y].Position);
				float Distance = max(DisA, DisB);
				Distance = std::clamp<float>(Distance, 100, 400);
				float Brightness = (500 - Distance) / 400.f;
				ID2D1SolidColorBrush* LineBrush = nullptr;
				D2D1HwndRenderTarget->CreateSolidColorBrush(D2D1::ColorF(1, 1, 1,Brightness), &LineBrush);
				D2D1HwndRenderTarget->DrawLine(CirclesCollection[x].Position, CirclesCollection[y].Position, LineBrush, LineWidth);
				LineBrush->Release();
			}
			float Distance = GetDistance(CirclesCollection[x].Position, D2D1::Point2F((float)CursorPos.x, (float)CursorPos.y));
			Distance = std::clamp<float>(Distance, 100, 400);
			float Brightness = (500 - Distance) / 400.f;
			CirclesCollection[x].Color.a = Brightness;
			ID2D1SolidColorBrush* CircleBrush = nullptr;
			D2D1HwndRenderTarget->CreateSolidColorBrush(CirclesCollection[x].Color, &CircleBrush);
			D2D1HwndRenderTarget->FillEllipse(D2D1::Ellipse(CirclesCollection[x].Position, CirclesCollection[x].Size, CirclesCollection[x].Size), CircleBrush);
			CircleBrush->Release();
		}
		D2D1HwndRenderTarget->EndDraw();
	}

	void Tick(float delta) override
	{
		//Move
		for (size_t i=0;i<CirclesCollection.size();i++)
		{
			if (CirclesCollection[i].Position.x + CirclesCollection[i].Velocity.x < 0 ||
				CirclesCollection[i].Position.x + CirclesCollection[i].Velocity.x > ScreenWidth)
				CirclesCollection[i].Velocity.x = -CirclesCollection[i].Velocity.x;
			CirclesCollection[i].Position.x += CirclesCollection[i].Velocity.x;
			if (CirclesCollection[i].Position.y + CirclesCollection[i].Velocity.y < 0 ||
				CirclesCollection[i].Position.y + CirclesCollection[i].Velocity.y > ScreenHeight)
				CirclesCollection[i].Velocity.y = -CirclesCollection[i].Velocity.y;
			CirclesCollection[i].Position.y += CirclesCollection[i].Velocity.y;
		}
		//Render
		Render();
	}


	void OnMouseDown(int VKey, bool IsOnDesktop) override
	{
		POINT CursorPos;
		GetCursorPos(&CursorPos);
		if (!IsOnDesktop)
			return;
		switch (VKey)
		{
		case VK_LBUTTON:
		{
			Circle tmpCircle;
			tmpCircle.Position = D2D1::Point2F((float)CursorPos.x, (float)CursorPos.y);
			tmpCircle.Velocity = D2D1::Point2F(float(rand() % 1000 - 500), float(rand() % 1000 - 500));
			float VelocityLength = tmpCircle.Velocity.x * tmpCircle.Velocity.x + tmpCircle.Velocity.y * tmpCircle.Velocity.y;
			VelocityLength = sqrtf(VelocityLength);
			tmpCircle.Velocity.x = tmpCircle.Velocity.x / VelocityLength * 3;
			tmpCircle.Velocity.y = tmpCircle.Velocity.y / VelocityLength * 3;
			tmpCircle.Size = CircleSize;
			tmpCircle.Color = CircleColor;
			CirclesCollection.push_back(tmpCircle);
		}
		break;
		case VK_RBUTTON:
		{
			CirclesCollection.pop_back();
			if (CirclesCollection.size()==0)
			{
				MessageBox(NULL, TEXT("Designed By JackMyth"), TEXT("Dynamic Desktop"), MB_OK);
				exit(0);
			}
		}
		break;
		default:
			break;
		}
	}


	void OnMouseUp(int VKey) override
	{
		
	}

};