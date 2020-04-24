#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <string.h>

int WinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd )
{
	ShellExecute(nullptr, "open", "shell:AppsFolder\\Microsoft.WindowsTerminal_8wekyb3d8bbwe!App", 
		lpCmdLine, nullptr, nShowCmd);
	/*SHELLEXECUTEINFO sei = {NULL};
	sei.cbSize = sizeof(SHELLEXECUTEINFO);
	sei.hInstApp = hPrevInstance;
	sei.lpParameters = lpCmdLine;
	sei.lpFile = "shell:AppsFolder\Microsoft.WindowsTerminal_8wekyb3d8bbwe!App";
	sei.nShow = nShowCmd;
	ShellExecuteEx(&sei);*/
	return 0;
}