#include <stdio.h>
#include <windows.h>
#include <tlhelp32.h>
#include <string>
#include <tchar.h>
#include <Psapi.h>
#include <conio.h>
#pragma comment (lib,"Psapi.lib")

BOOL DosPathToNtPath(LPTSTR pszDosPath, LPTSTR pszNtPath)
{
	TCHAR            szDriveStr[500];
	TCHAR            szDrive[3];
	TCHAR            szDevName[100];
	INT                cchDevName;
	INT                i;

	//检查参数
	if (!pszDosPath || !pszNtPath)
		return FALSE;

	//获取本地磁盘字符串
	if (GetLogicalDriveStrings(sizeof(szDriveStr), szDriveStr))
	{
		for (i = 0; szDriveStr[i]; i += 4)
		{
			if (!lstrcmpi(&(szDriveStr[i]), _T("A:\\")) || !lstrcmpi(&(szDriveStr[i]), _T("B:\\")))
				continue;

			szDrive[0] = szDriveStr[i];
			szDrive[1] = szDriveStr[i + 1];
			szDrive[2] = '\0';
			if (!QueryDosDevice(szDrive, szDevName, 100))//查询 Dos 设备名
				return FALSE;

			cchDevName = lstrlen(szDevName);
			if (_tcsnicmp(pszDosPath, szDevName, cchDevName) == 0)//命中
			{
				lstrcpy(pszNtPath, szDrive);//复制驱动器
				lstrcat(pszNtPath, pszDosPath + cchDevName);//复制路径

				return TRUE;
			}
		}
	}

	lstrcpy(pszNtPath, pszDosPath);

	return FALSE;
}

BOOL GetProcessFullPath(DWORD dwPID, TCHAR pszFullPath[MAX_PATH])
{
	TCHAR        szImagePath[MAX_PATH];
	HANDLE        hProcess;
	if (!pszFullPath)
		return FALSE;

	pszFullPath[0] = '\0';
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, 0, dwPID);
	if (!hProcess)
		return FALSE;

	if (!GetProcessImageFileName(hProcess, szImagePath, MAX_PATH))
	{
		CloseHandle(hProcess);
		return FALSE;
	}

	if (!DosPathToNtPath(szImagePath, pszFullPath))
	{
		CloseHandle(hProcess);
		return FALSE;
	}

	CloseHandle(hProcess);

	_tprintf(_T("%d,%s \r\n"), dwPID, pszFullPath);
	return TRUE;
}

#define ProcessBasicInformation 0  
#define ProcessWow64Information 26

typedef struct
{
	DWORD ExitStatus;
	DWORD PebBaseAddress;
	DWORD AffinityMask;
	DWORD BasePriority;
	ULONG UniqueProcessId;
	ULONG InheritedFromUniqueProcessId;
}   PROCESS_BASIC_INFORMATION;


// ntdll!NtQueryInformationProcess (NT specific!)  
//  
// The function copies the process information of the  
// specified type into a buffer  
//  
// NTSYSAPI  
// NTSTATUS  
// NTAPI  
// NtQueryInformationProcess(  
//    IN HANDLE ProcessHandle,              // handle to process  
//    IN PROCESSINFOCLASS InformationClass, // information type  
//    OUT PVOID ProcessInformation,         // pointer to buffer  
//    IN ULONG ProcessInformationLength,    // buffer size in bytes  
//    OUT PULONG ReturnLength OPTIONAL      // pointer to a 32-bit  
//                                          // variable that receives  
//                                          // the number of bytes  
//                                          // written to the buffer   
// ); 

typedef LONG(__stdcall *PROCNTQSIP)(HANDLE, UINT, PVOID, ULONG, PULONG);
DWORD GetParentProcessID(DWORD dwProcessId)
{
	LONG                        status;
	DWORD                        dwParentPID = (DWORD)-1;
	HANDLE                        hProcess;
	PROCESS_BASIC_INFORMATION    pbi;

	PROCNTQSIP NtQueryInformationProcess = (PROCNTQSIP)GetProcAddress(
		GetModuleHandle("ntdll"), "NtQueryInformationProcess");

	if (NULL == NtQueryInformationProcess)
	{
		return (DWORD)-1;
	}
	// Get process handle
	hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, dwProcessId);
	if (!hProcess)
	{
		return (DWORD)-1;
	}
	// Retrieve information
	status = NtQueryInformationProcess(hProcess,
									   ProcessWow64Information,
									   (PVOID)&pbi,
									   sizeof(PROCESS_BASIC_INFORMATION),
									   NULL
	);
	// Copy parent Id on success
	if (!status)
	{
		dwParentPID = pbi.InheritedFromUniqueProcessId;
	}
	CloseHandle(hProcess);
	return dwParentPID;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPSTR lpCmdLine, _In_ int nShowCmd)
{
	TCHAR pProcessName[1024];
	GetProcessFullPath(GetParentProcessID(GetCurrentProcessId()), pProcessName);
	long k;
	for (k=strlen(pProcessName)-1;k>=0;k--)
	{
		if (pProcessName[k] == '/' || pProcessName[k] == '\\')
		{
			k++;
			break;
		}
	}
	SHELLEXECUTEINFO ShellInfo = { 0 };
	ShellInfo.cbSize = sizeof(SHELLEXECUTEINFO);
	ShellInfo.hwnd = NULL;
	ShellInfo.lpVerb = "open";
	if (!strcmp(pProcessName + k, "explorer.exe"))
	{
		ShellInfo.lpFile = R"(C:\Windows\System32\WindowsPowerShell\v1.0\powershell.exe)";
	}
	else
	{
		ShellInfo.lpFile = R"(C:\Windows\System32\ocmd.exe)";
	}
	ShellInfo.lpParameters = lpCmdLine;
	ShellInfo.nShow = SW_SHOWNORMAL;
	ShellInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
	//ShellExecuteEx(&ShellInfo);
	STARTUPINFO si = { sizeof(si) };
	PROCESS_INFORMATION pi;
	si.dwFlags = STARTF_USESHOWWINDOW;//指定wShowWindow成员有效
	si.wShowWindow = TRUE;//此成员设为TRUE的话则显示新建进程的主窗口
	CreateProcess(ShellInfo.lpFile, (char*)ShellInfo.lpParameters, NULL, NULL, FALSE, NULL, NULL, NULL, &si, &pi);
	exit(0);
}
