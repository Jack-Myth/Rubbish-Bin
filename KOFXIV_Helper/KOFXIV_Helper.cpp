#include <stdio.h>
#include <windows.h>
#include <conio.h>
#include <vk_def.h>
#include <vector>
#include <queue>
#include <utility>
#include <thread>
#include <chrono>
#include <list>
#define KEY_DOWN(VK_NONAME) ((GetAsyncKeyState(VK_NONAME) & 0x8000) ? 1:0)

using namespace std;
HHOOK  g_keyhook = NULL;
LRESULT CALLBACK KeyboardProc(int code, WPARAM wParam, LPARAM lParam);
HRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
inline void QueuePush(char Cmd);
inline void PressKey(char KeyToPress);
inline void PressKey(char KeyToPress);
inline void ReleaseKey(char KeyToPress);
inline bool CheckAndFire();

HWND MainHwnd,Victim;
char KeyValue[10] = { 0 };
bool KeyStatus[10] = { false };
std::list<char> CmdQueue;
std::vector<std::pair<std::vector<char>, std::vector<char>>> CmdList;
bool Actioning;
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	WNDCLASSEX wnd = { NULL };
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wnd.hCursor = LoadCursor(0, IDC_ARROW);
	wnd.hIcon = LoadIcon(0, IDC_ICON);
	wnd.hInstance = hInstance;
	wnd.lpfnWndProc = MainWndProc;
	wnd.lpszClassName = TEXT("Blank Win32 Window");
	RegisterClassEx(&wnd);
	RECT rc = { 0,0,600,500 };
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);
	MainHwnd = CreateWindow(wnd.lpszClassName, TEXT("KOF XIV Helper"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
	ShowWindow(MainHwnd, nShowCmd);
	Victim=FindWindow(NULL,TEXT("The King Of Fighters XIV"));
	MSG msg = { NULL };
	Actioning = false;
	FILE* FileHandle = fopen("OriginCmd", "r");
	for (int i = 0; i < 3; i++)
		fscanf(FileHandle, "%c", &KeyValue[i]);
	while (!feof(FileHandle))
	{
		char TmpCmdList[200] = {NULL};
		fscanf(FileHandle, "%s", TmpCmdList);
		char* TTmpCL = TmpCmdList;
		std::pair<std::vector<char>, std::vector<char>> STDP;
		while (*TTmpCL)
		{
			STDP.first.push_back(*TTmpCL);
			TTmpCL++;
		}
		CmdList.push_back(STDP);
	}
	fclose(FileHandle);
	FileHandle = fopen("SimpleCmd", "r");
	int k = 0;
	while (!feof(FileHandle))
	{
		char TmpCmdList[200] = { NULL };
		fscanf(FileHandle, "%s", TmpCmdList);
		char* TTmpCL = TmpCmdList;
		while (*TTmpCL)
		{
			CmdList[k].second.push_back(*TTmpCL);
			TTmpCL++;
		}
		k++;
	}
	fclose(FileHandle);
	g_keyhook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, 0, 0);
	if (g_keyhook)
		MessageBox(MainHwnd, TEXT("Hook Successful"), TEXT(""), MB_OK);
	else
	{
		MessageBox(MainHwnd, TEXT("Hook Faild"), TEXT(""), MB_OK);
		exit(-1);
	}
	SetTimer(MainHwnd, 0, 200, NULL);
	while (msg.message != WM_QUIT)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		GetMessage(&msg, 0, 0, 0);
	}
	return 0;
}

HRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	case WM_TIMER:
		if (CmdQueue.size()>0)
			CmdQueue.pop_back();
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

LRESULT CALLBACK KeyboardProc(
	int code,       // hook code
	WPARAM wParam,  // virtual-key code
	LPARAM lParam   // keystroke-message information
)
{
	if (Actioning)
		return CallNextHookEx(g_keyhook, code, wParam, lParam);
	if (wParam==WM_KEYDOWN)
	{
		/*for (int i=0;i<3;i++)
		{
			if (((tagKBDLLHOOKSTRUCT*)lParam)->vkCode==KeyValue[i])
			{
				QueuePush(KeyValue[i]);
				thread([i, code,wParam,lParam]()kakdasd
				{
					//Sleep(70);
					this_thread::sleep_for(chrono::milliseconds(70));
					CallNextHookEx(g_keyhook, code, wParam, lParam);
				}).detach();
				return -1;
			}
		}*/
		QueuePush(((tagKBDLLHOOKSTRUCT*)lParam)->vkCode);
		if (CheckAndFire())
			return -1;
	}
	return CallNextHookEx(g_keyhook, code, wParam, lParam);
	//如何处理键盘消息
	//如果钩子处理的消息，您可以返回一个非0值，防止系统把消息传递给钩子链中的下一个钩
	//子， //或者把消息发送到目标窗口。
}

inline void QueuePush(char Cmd)
{
	CmdQueue.push_front(Cmd);
	if (CmdQueue.size() > 20)
		CmdQueue.pop_back();
}

inline void PressKey(char KeyToPress)
{
	keybd_event(VK_PROCESSKEY, MapVirtualKey(KeyToPress, MAPVK_VK_TO_VSC), 0, 0);
	Sleep(20);
}

void ReleaseKey(char KeyToRelease)
{
	keybd_event(VK_PROCESSKEY, MapVirtualKey(KeyToRelease, MAPVK_VK_TO_VSC), KEYEVENTF_KEYUP, 0);
	Sleep(20);
}

inline bool CheckAndFire()
{
	bool FindFlag = true;
	for (int i = 0; i < CmdList.size(); i++)
	{
		FindFlag = true;
		auto ItList = CmdQueue.begin();
		for (int k=CmdList[i].second.size()-1;k>=0;k--)
		{
			if (CmdList[i].second.size() >CmdQueue.size()||ItList==CmdQueue.end()||CmdList[i].second[k]!=*ItList)
			{
				FindFlag = false;
				break;
			}
			ItList++;
		}
		if (!FindFlag)
			continue;
		else
		{
			Actioning = true;
			for (int a = 0; a < CmdList[i].first.size(); a++)
			{
				/*if (KEY_DOWN(VK_CONTROL))
				{
					Actioning = false;
					return true;
				}*/
				if (CmdList[i].first[a] == '_')
				{
					a++;
					ReleaseKey(CmdList[i].first[a]);
				}
				else if (CmdList[i].first[a] == '^')
					keybd_event(VK_PROCESSKEY, MapVirtualKey(CmdList[i].first[a], MAPVK_VK_TO_VSC), 0, 0);
				else if (CmdList[i].first[a] == '*')
				{
					int TimeToDelay;
					sscanf(CmdList[i].first.data() + a + 1, "%d", &TimeToDelay);
					Sleep(TimeToDelay);
					while (TimeToDelay)
					{
						TimeToDelay = (int)(TimeToDelay/10);
						a++;
					}
				}
				else if (CmdList[i].first[a] == '/')
					continue;
				else
				{
					PressKey(CmdList[i].first[a]);
				}
			}
			Actioning = false;
			return true;
		}
	}
	return false;
}

