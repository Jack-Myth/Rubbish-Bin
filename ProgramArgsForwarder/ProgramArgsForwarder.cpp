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
	FILE* fp;
	fopen_s(&fp, "PAF.cfg", "r");
	if (!fp)
	{
		std::string  argStr = lpCmdLine;
		MessageBoxA(nullptr, argStr.c_str(), "ProgramArgsForwarder", MB_OK);
		return 0;
	}
	char FilePath[1024];
	char FullPath[2048];
	std::string ParentPath;
	fscanf_s(fp, "%s", FilePath, 1024);
	_fullpath(FullPath, FilePath, 2048);
	ParentPath = FullPath;
	ParentPath = ParentPath.substr(0, ParentPath.find_last_of("\\"));
	ShellExecute(nullptr, "open", FilePath, lpCmdLine, ParentPath.c_str(),SW_NORMAL);
	return 0;
}


