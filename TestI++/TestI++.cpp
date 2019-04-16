#include <stdio.h>
#include <iostream>
#include <windows.h>
#include <string>

std::string GetExePath(void)
{
	char szFilePath[MAX_PATH + 1] = { 0 };
	GetModuleFileNameA(NULL, szFilePath, MAX_PATH);
	(strrchr(szFilePath, '\\'))[0] = 0; // 删除文件名，只获得路径字串
	std::string path = szFilePath;

	return path;
}

int main()
{
	char CurDir[1024];
	char TargetDir[1024];
	strcpy(CurDir, GetExePath().c_str());
	printf("ASSAULT_SPY TransPatch Fixer\nDesigned By JackMyth\n\nn");
	printf("Execute File Path:%s\n",CurDir);
	strcpy(TargetDir, CurDir);
	strcat(TargetDir, "\\ASSAULT_SPY\\Binaries\\Win64\\ASSAULT_SPY-Win64-Shipping.exe");
	strcat(CurDir, "\\ASSAULT_SPY\\Binaries\\Win64");
	printf("Execute Game EXE:\n%s\nAt:\n%s\n",TargetDir,CurDir);
	HINSTANCE hi= ShellExecuteA(NULL, "Open", TargetDir, nullptr, CurDir, SW_NORMAL);
	if ((long long)hi<32)
	{
		char Msg[512];
		sprintf(Msg, "Error Code:%lld", (long long)hi);
		MessageBoxA(nullptr, Msg, "Shell Execute Failed!",MB_OK);
		return -1;
	}
	WaitForSingleObject(hi, INFINITE);
}