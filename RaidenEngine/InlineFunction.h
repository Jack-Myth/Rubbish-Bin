#pragma once
#define  _CRT_SECURE_NO_WARNINGS
#include <math.h>

inline void CheckAndPostErrorMsg(bool Value, WCHAR* ErrorMessage)
{
	if (Value)
	{
		MessageBox(NULL, ErrorMessage, TEXT("Error"), MB_OK);
		exit(-1);
	}
}

inline float GetPointDistance(D2D1_POINT_2F pA, D2D1_POINT_2F pB)
{
	pA.y *= 2, pB.y *= 2;
	return sqrtf((pB.x - pA.x)*(pB.x - pA.x) + (pB.y - pA.y)*(pB.y - pA.y));
}

inline D2D1_POINT_2F GetUnitVector(D2D1_POINT_2F Vector2D)
{
	float vecM = sqrtf(Vector2D.x*Vector2D.x + Vector2D.y*Vector2D.y);
	return { Vector2D.x / vecM,Vector2D.y / vecM };
}

inline bool IsOutofBound(D2D1_POINT_2F Location, float Radius)
{
	return Location.x + Radius < 0 || Location.x - Radius>1 || Location.y + Radius / 2 < 0 || Location.y - Radius / 2 > 1?true:false;
}