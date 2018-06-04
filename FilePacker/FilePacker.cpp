#include <stdio.h>
#include <direct.h> 
#include <windows.h>
#include <stack>
#include <vector>
#include "Shlobj.h"
#include <shlwapi.h>
#include <locale.h>
#pragma comment(lib,"Shlwapi.lib")

typedef struct Find_File_DataEx
{
	WIN32_FIND_DATA FileFileData;
	std::wstring FilePath;
};

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
void AppendLog(const WCHAR* LogToAppend, int LogIntToAppend, BOOL NewLine);
void ProcessPak(WCHAR* szBuffer, WCHAR* FileP);
void ProcessUnPak(WCHAR* szBuffer, WCHAR* FileP);
void fReadUntilZero(char* byteBuffer, FILE* FileToRead);
void SearchAndAppend(std::stack<HANDLE>& FolderHandle, std::vector<std::wstring>& PathArray, std::vector<Find_File_DataEx>& FileStack, FILE* FileHandle, WIN32_FIND_DATA LastFindFileData);
inline std::wstring GetAbsolutePath(std::vector<std::wstring>& PathArray, std::wstring FileName, BOOL FolderOnly = FALSE);
inline std::wstring GetLocalPath(std::vector<std::wstring>& PathArray, std::wstring FileName, BOOL FolderOnly = FALSE);

HWND MainHwnd = NULL, HwndCollection[5] = {NULL};
RECT WindowRect = { 0,0,1024,768 };
std::wstring MainPath = TEXT("");
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	WNDCLASSEX wnd = { NULL };
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wnd.hCursor = LoadCursor(0, IDC_ARROW);
	wnd.hIcon = LoadIcon(0, IDC_ICON);
	wnd.hInstance = hInstance;
	wnd.lpfnWndProc = MainWndProc;
	wnd.lpszClassName = TEXT("Win32 Window");
	RegisterClassEx(&wnd);
	AdjustWindowRect(&WindowRect, WS_OVERLAPPEDWINDOW, false);
	MainHwnd = CreateWindow(wnd.lpszClassName, TEXT("File Packer"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, WindowRect.right - WindowRect.left, WindowRect.bottom - WindowRect.top, NULL, NULL, hInstance, NULL);
	ShowWindow(MainHwnd, nShowCmd);
	setlocale(LC_ALL, "");
	MSG msg = {NULL};
	while (msg.message!=WM_QUIT)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		GetMessage(&msg, NULL, NULL, NULL);
	}
}

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		HwndCollection[0]= CreateWindow(TEXT("BUTTON"), TEXT("打包"), WS_CHILDWINDOW|WS_VISIBLE, 0, 0, 75, 50, hwnd, nullptr, nullptr, NULL);
		HwndCollection[1] = CreateWindow(TEXT("BUTTON"), TEXT("提取"), WS_CHILDWINDOW | WS_VISIBLE, 0, 50, 75, 50, hwnd, nullptr, nullptr, NULL);
		HwndCollection[2] = CreateWindow(TEXT("EDIT"), TEXT(""),WS_CHILDWINDOW|WS_VISIBLE| ES_MULTILINE,100,0,500,768,hwnd,nullptr,nullptr,NULL);
		return DefWindowProc(hwnd,message,wParam,lParam);
	case WM_COMMAND:
		if ((HWND)lParam == HwndCollection[0] && HIWORD(wParam) == BN_CLICKED)
		{
			TCHAR szBuffer[MAX_PATH] = { 0 };
			BROWSEINFO bi;
			ZeroMemory(&bi, sizeof(BROWSEINFO));
			bi.hwndOwner = NULL;
			bi.pszDisplayName = szBuffer;
			bi.lpszTitle = TEXT("选择需要打包的文件夹：");
			bi.ulFlags = BIF_RETURNFSANCESTORS;
			LPITEMIDLIST idl = SHBrowseForFolder(&bi);
			if (NULL == idl)
				return 0;
			if (SHGetPathFromIDList(idl, szBuffer))
			{
				WCHAR FileP[MAX_PATH] = { NULL };
				OPENFILENAME OpenFN = { NULL };
				OpenFN.lStructSize = sizeof(OPENFILENAME);
				OpenFN.hwndOwner = MainHwnd;
				OpenFN.lpstrFilter = TEXT("FilePak文件\0*.FilePak\0\0");
				OpenFN.nMaxFile = MAX_PATH;
				OpenFN.lpstrFile = FileP;
				OpenFN.lpstrTitle = TEXT("选择输出路径");
				if (GetSaveFileName(&OpenFN))
				{
					AppendLog(szBuffer, 0, TRUE);
					ProcessPak(szBuffer, FileP);
				}
			}
		}
		else if ((HWND)lParam == HwndCollection[1] && HIWORD(wParam) == BN_CLICKED)
		{
			TCHAR szBuffer[MAX_PATH] = { 0 };
			BROWSEINFO bi;
			ZeroMemory(&bi, sizeof(BROWSEINFO));
			bi.hwndOwner = NULL;
			bi.pszDisplayName = szBuffer;
			bi.lpszTitle = TEXT("选择解包的目标文件夹：");
			bi.ulFlags = BIF_RETURNFSANCESTORS;
			WCHAR FileP[MAX_PATH] = { NULL };
			OPENFILENAME OpenFN = { NULL };
			OpenFN.lStructSize = sizeof(OPENFILENAME);
			OpenFN.hwndOwner = MainHwnd;
			OpenFN.lpstrFilter = TEXT("FilePak文件\0*.FilePak\0\0");
			OpenFN.nMaxFile = MAX_PATH;
			OpenFN.Flags = OFN_FILEMUSTEXIST;
			OpenFN.lpstrFile = FileP;
			OpenFN.lpstrTitle = TEXT("选择包裹路径");
			if (GetOpenFileName(&OpenFN))
			{
				LPITEMIDLIST idl = SHBrowseForFolder(&bi);
				if (NULL == idl)
					return 0;
				if (SHGetPathFromIDList(idl, szBuffer))
				{
					AppendLog(szBuffer, 0, TRUE);
					ProcessUnPak(szBuffer, FileP);
				}
			}
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
		break;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

void AppendLog(const WCHAR* LogToAppend, int LogIntToAppend, BOOL NewLine)
{
	WCHAR Logs[10240] = {NULL};
	GetWindowText(HwndCollection[2], Logs, 10240);
	if (LogToAppend)
		wcscat(Logs, LogToAppend);
	else
	{
		WCHAR Numb[10];
		swprintf(Numb, TEXT("%d"), LogIntToAppend);
		wcscat(Logs, Numb);
	}
	if (NewLine)
		wcscat(Logs, TEXT("\r\n"));
	SetWindowText(HwndCollection[2], Logs);
}

void ProcessPak(WCHAR* szBuffer,WCHAR* FileP)
{
	MainPath = szBuffer;
	WIN32_FIND_DATA FindFileData;
	Find_File_DataEx FFDEx;
	std::stack<HANDLE> HStack;
	std::vector<std::wstring> PathArray;
	std::vector<Find_File_DataEx> FindFileDataArray;
	PathArray.push_back(TEXT(""));
	HStack.push(FindFirstFile((MainPath + TEXT("\\*")).c_str(), &FindFileData));
	FILE* FileHandle = _wfopen(FileP, TEXT("wb+"));
	if (!FileHandle)
	{
		AppendLog(TEXT("输出路径访问失败，终止！"), 0, TRUE);
		return ;
	}
	fprintf(FileHandle, "FP_");
	SearchAndAppend(HStack, PathArray, FindFileDataArray, FileHandle, FindFileData);
	fputc(0, FileHandle);
	int DataOffset=0;
	for (Find_File_DataEx FileStack : FindFileDataArray)
	{
		char FilePath[1024];
		wcstombs(FilePath, FileStack.FilePath.c_str(), 1024);
		DataOffset += strlen(FilePath);
		DataOffset += (sizeof(INT64)*2+1);
	}
	DataOffset += ftell(FileHandle);
	DataOffset++;
	INT64 DataLengthCount=0;
	INT64 InfoIndex = 0;
	FILE* FileTmp = NULL;
	for (Find_File_DataEx FileStack:FindFileDataArray)
	{
		if (FileTmp=_wfopen((MainPath+ FileStack.FilePath).c_str(),TEXT("rb+")))
		{
			INT64 NumInfo = 0;
			INT64 FileSize = 0;
			char FilePath[1024];
			wcstombs(FilePath, FileStack.FilePath.c_str(), 1024);
			fprintf(FileHandle, "%s", FilePath);
			fputc(0, FileHandle);
			InfoIndex = ftell(FileHandle);
			fseek(FileHandle, DataOffset + DataLengthCount, SEEK_SET);
			fseek(FileTmp, 0, SEEK_END);
			FileSize = ftell(FileTmp);
			rewind(FileTmp);
			char* FileBuffer = new char[FileSize];
			fread(FileBuffer, FileSize, 1, FileTmp);
			fwrite(FileBuffer, FileSize, 1, FileHandle);
			delete[] FileBuffer;
			fseek(FileHandle, InfoIndex, SEEK_SET);
			NumInfo = DataOffset + DataLengthCount;
			fwrite(&NumInfo, sizeof(INT64), 1, FileHandle);
			DataLengthCount += FileSize;
			fwrite(&FileSize, sizeof(INT64), 1, FileHandle);
			fclose(FileTmp);
		} 
		else
		{
			AppendLog(TEXT("打开文件失败："), 0, FALSE);
		}
		AppendLog((MainPath + TEXT("\\") + FileStack.FilePath).c_str(), 0, TRUE);
	}
	fputc(0, FileHandle);
	fclose(FileHandle);
}

void SearchAndAppend(std::stack<HANDLE>& FolderHandle,std::vector<std::wstring>& PathArray, std::vector<Find_File_DataEx>& FileStack,FILE* FileHandle,WIN32_FIND_DATA LastFindFileData)
{
	BOOL ShouldSpace = FALSE;
	for (std::wstring FolderName:PathArray)
		if (FolderName != TEXT(""))
		{
			char fPath[256];
			wcstombs(fPath, FolderName.c_str(), 256);
			fprintf(FileHandle, "\\%s", fPath);
			ShouldSpace = TRUE;
		}
	if(ShouldSpace)
		fputc(0, FileHandle);
	WIN32_FIND_DATAW FindDataW = LastFindFileData;
	Find_File_DataEx FFDEx;
	do
	{
		if (FindDataW.cFileName[0]=='.')
			continue;
		if (PathIsDirectory(GetAbsolutePath(PathArray,FindDataW.cFileName).c_str()))
		{
			PathArray.push_back((std::wstring)FindDataW.cFileName);
			FolderHandle.push(FindFirstFile((GetAbsolutePath(PathArray, FindDataW.cFileName,TRUE)+TEXT("\\*")).c_str(), &FindDataW));
			SearchAndAppend(FolderHandle, PathArray, FileStack, FileHandle, FindDataW);
			continue;
		}
		FFDEx.FileFileData = FindDataW;
		FFDEx.FilePath = GetLocalPath(PathArray, std::wstring(FindDataW.cFileName));
		FileStack.push_back(FFDEx);
	} while (FindNextFile(FolderHandle.top(), &FindDataW));
	FindClose(FolderHandle.top());
	FolderHandle.pop();
	PathArray.pop_back();
}

inline std::wstring GetAbsolutePath(std::vector<std::wstring>& PathArray, std::wstring FileName,BOOL FolderOnly)
{
	std::wstring TmpWStr=MainPath;
	for (std::wstring WStr:PathArray)
		if (WStr!=TEXT(""))
			TmpWStr += (TEXT("\\") + WStr);
	if (!FolderOnly)
		TmpWStr += (TEXT("\\") + FileName);
	return TmpWStr;
}

inline std::wstring GetLocalPath(std::vector<std::wstring>& PathArray, std::wstring FileName, BOOL FolderOnly)
{
	std::wstring TmpWStr = TEXT("");
	for (std::wstring WStr : PathArray)
		if (WStr != TEXT(""))
			TmpWStr += (TEXT("\\") + WStr);
	if (!FolderOnly)
		TmpWStr += (TEXT("\\") + FileName);
	return TmpWStr;
}

void ProcessUnPak(WCHAR* szBuffer, WCHAR* FileP)
{
	char MainPathA[1024];
	wcstombs(MainPathA, szBuffer, 1024);
	FILE* FileHandle = _wfopen(FileP, TEXT("rb+"));
	if (fgetc(FileHandle)=='F'&&fgetc(FileHandle) == 'P'&&fgetc(FileHandle) == '_')
	{
		while (1)
		{
			char FP[1024];
			fReadUntilZero(FP, FileHandle);
			if (!*FP)
				break;
			_mkdir((std::string(MainPathA) + FP).c_str());
			long F = ftell(FileHandle);
			if (!fgetc(FileHandle))
				break;
			F = ftell(FileHandle);
			if (fseek(FileHandle, (long)-1, SEEK_CUR))
			{
				F = errno;
			}
			F = ftell(FileHandle);
		}
		while (1)
		{
			char FP[1024];
			INT64 BufferSize = 0;
			INT64 BufferPosition = 0;
			long InfoIndex = 0;
			fReadUntilZero(FP, FileHandle);
			//fscanf(FileHandle, "%^[]", FP);
			//fgetc(FileHandle);
			FILE* TmpFile=fopen((std::string(MainPathA) + FP).c_str(), "wb+");
			fread(&BufferPosition, sizeof(INT64), 1, FileHandle);
			fread(&BufferSize, sizeof(INT64), 1, FileHandle);
			InfoIndex = ftell(FileHandle);
			fseek(FileHandle, BufferPosition, SEEK_SET);
			char* FBuffer = new char[BufferSize];
			fread(FBuffer, 1, BufferSize, FileHandle);
			fwrite(FBuffer, 1, BufferSize, TmpFile);
			fclose(TmpFile);
			delete[] FBuffer;
			fseek(FileHandle, InfoIndex, SEEK_SET);
			WCHAR wChr[1024];
			mbstowcs(wChr, FP, 1024);
			AppendLog(wChr, 0, TRUE);
			if (!fgetc(FileHandle))
				break;
			fseek(FileHandle, (long)-1, SEEK_CUR);
		}
		fclose(FileHandle);
	}
}

void fReadUntilZero(char* byteBuffer, FILE* FileToRead)
{
	int i = -1;
	do
	{
		i++;
		*(byteBuffer + i) = fgetc(FileToRead);
	} while (*(byteBuffer + i));
}