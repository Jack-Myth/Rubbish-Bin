#include <Windows.h>
#include <stdio.h>
#include <TlHelp32.h>

int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	HWND CurrentWindow = GetForegroundWindow();
	if (CurrentWindow)
	{
		char ClassName[256];
		do
		{
			GetClassNameA(CurrentWindow, ClassName, 255);
			//MessageBoxA(nullptr, ClassName, "", MB_OK);
			if (!_stricmp("photoshop", ClassName)||!_stricmp("sflRootWindow",ClassName))
			{
				INPUT input[4];
				memset(input, 0, sizeof(input));
				input[0].type = input[1].type = input[2].type = input[3].type = INPUT_KEYBOARD;  //设置键盘模式  
				input[0].ki.wVk = input[3].ki.wVk = VK_CONTROL;
				input[1].ki.wVk = input[2].ki.wVk = 'Z';
				input[2].ki.dwFlags = input[3].ki.dwFlags = KEYEVENTF_KEYUP;
				SendInput(4, input, sizeof(INPUT));
				return 0;
			}
		} while (CurrentWindow = GetParent(CurrentWindow));
	}
	else
	{
		//MessageBoxA(nullptr, "ActiveWindow为空", "", MB_OK);
	}
	//发送截图快捷键
	INPUT input[6];
	memset(input, 0, sizeof(input));
	input[0].type = input[1].type = input[2].type = input[3].type = INPUT_KEYBOARD;  //设置键盘模式  
	input[4].type = input[5].type = INPUT_KEYBOARD;
	input[0].ki.wVk = input[3].ki.wVk = VK_LWIN;
	input[1].ki.wVk = input[4].ki.wVk = VK_SHIFT;
	input[2].ki.wVk = input[5].ki.wVk = 'S';
	input[3].ki.dwFlags = input[4].ki.dwFlags =input[5].ki.dwFlags = KEYEVENTF_KEYUP; 
	SendInput(6, input, sizeof(INPUT));
	return 0;
}