#include <stdio.h>
#include <windows.h>
#include <stdlib.h>

int WINAPI wWinMain( _In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd )
{
	FILE* tf = fopen("Warning.s", "r");
	if (tf)
	{
		char TmpMsg[10240] = "在";
		char Message[10240] = { 0 };
		fscanf(tf, "%[^\n]", Message);
		fclose(tf);
		strcat(TmpMsg, Message);
		strcat(TmpMsg, "左右，有人试图以错误的密码登陆电脑，请检查照片。");
		MessageBoxA(NULL, TmpMsg, "警告", MB_OK);
		system("start .\\");
		remove("Warning.s");
	}
	return 0;
}
