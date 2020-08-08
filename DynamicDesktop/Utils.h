#pragma once
#include <windows.h>
#include "resource1.h"

inline LPVOID CreateBufferFromResource(LPCWSTR Type,int Resource,int* pLength=nullptr)
{
	auto hResource = FindResource(GetModuleHandle(NULL), MAKEINTRESOURCE(Resource),Type);
	auto hG = LoadResource(GetModuleHandle(NULL), hResource);
	LPVOID pData = LockResource(hG);
	DWORD resourceSize = SizeofResource(GetModuleHandle(NULL), hResource);
	LPVOID pMemData = new char[resourceSize];
	memcpy(pMemData, pData, resourceSize);
	UnlockResource(hG);
	FreeResource(hG);
	if (pLength)
		*pLength = resourceSize;
	return pMemData;
}