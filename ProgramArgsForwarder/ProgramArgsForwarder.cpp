#include<stdio.h>
#include <Windows.h>
#include <string>

int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
)
{
	const char* EXEName;
	char ForwardPath[MAX_PATH];
	char WorkingPath[MAX_PATH];
	EXEName = __argv[0] + strlen(__argv[0]) - 1;
	for (; *EXEName != '\\'; EXEName--);
	EXEName++; // "\\EXEName.exe",Add 1 to "EXEName.exe"
	//Read REG
	HKEY hKEY;
	std::string RegPath = "SOFTWARE\\JackMyth\\ProgramArgsForwarder\\";
	RegPath += EXEName;
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegPath.c_str(), 0, KEY_READ, &hKEY))
	{
		DWORD dwType = REG_SZ;
		DWORD fp_size = MAX_PATH;
		RegQueryValueEx(hKEY, "ForwardPath",0, &dwType, (LPBYTE)ForwardPath, &fp_size);
		fp_size = MAX_PATH;
		RegQueryValueEx(hKEY, "WorkingPath",0, &dwType, (LPBYTE)WorkingPath, &fp_size);
	}
	else
	{
		return -1;
	}
	RegCloseKey(hKEY);
	ShellExecute(nullptr, "open", ForwardPath, lpCmdLine, WorkingPath, nCmdShow);
	return 0;
}


