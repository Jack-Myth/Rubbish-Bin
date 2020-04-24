#include<stdio.h>
#include <Windows.h>
#include <string>

std::string& replace_str(std::string& str, const std::string& to_replaced, const std::string& newchars);
int WINAPI WinMain(
	HINSTANCE hInstance,
	HINSTANCE hPrevInstance,
	LPSTR lpCmdLine,
	int nCmdShow
)
{
#if _DEBUG
	for (int i = 0; i < __argc; i++)
		MessageBox(NULL, __argv[i], "Args", MB_OK);
#endif
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
	DWORD UseCmdlineAsArgv0=FALSE;
	if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_LOCAL_MACHINE, RegPath.c_str(), 0, KEY_READ, &hKEY))
	{
		DWORD szType = REG_SZ;
		DWORD dwType = REG_DWORD;
		DWORD fp_size = MAX_PATH;
		RegQueryValueEx(hKEY, "ForwardPath",0, &szType, (LPBYTE)ForwardPath, &fp_size);
		fp_size = MAX_PATH;
		RegQueryValueEx(hKEY, "WorkingPath",0, &szType, (LPBYTE)WorkingPath, &fp_size);
		fp_size = sizeof(UseCmdlineAsArgv0);
		RegQueryValueEx(hKEY, "UseCmdlineAsArgv0", 0, &dwType, (LPBYTE)&UseCmdlineAsArgv0, &fp_size);
	}
	else
	{
		return -1;
	}
	RegCloseKey(hKEY);
	std::string ForwardCmdLine=lpCmdLine;
	if (UseCmdlineAsArgv0)
	{
		replace_str(ForwardCmdLine, "\"", "\\\"");
		ForwardCmdLine = "\"" + ForwardCmdLine;
		ForwardCmdLine += "\"";
	}
	ShellExecute(nullptr, "open", ForwardPath, ForwardCmdLine.c_str(), WorkingPath, nCmdShow);
	return 0;
}

std::string& replace_str(std::string& str, const std::string& to_replaced, const std::string& newchars)
{
	for (std::string::size_type pos(0); pos != std::string::npos; pos += newchars.length())
	{
		pos = str.find(to_replaced, pos);
		if (pos != std::string::npos)
			str.replace(pos, to_replaced.length(), newchars);
		else
			break;
	}
	return  str;
}


