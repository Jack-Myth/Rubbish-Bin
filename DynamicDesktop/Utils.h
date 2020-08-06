#pragma once
#include <windows.h>

inline LPVOID CreateBufferFromResource(LPCWSTR Name,int Type,int* pLength=nullptr)
{
	auto hRresource = FindResource(GetModuleHandle(NULL), Name, MAKEINTRESOURCE(Type));
	auto hG = LoadResource(GetModuleHandle(NULL), hRresource);
	LPVOID pData = LockResource(hG);
	DWORD resourceSize = SizeofResource(GetModuleHandle(NULL), hRresource);
	LPVOID pMemData = new char[resourceSize];
	memcpy(pMemData, pData, resourceSize);
	UnlockResource(hG);
	if (pLength)
		*pLength = resourceSize;
	return pMemData;
}