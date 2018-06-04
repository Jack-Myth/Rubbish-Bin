#include <windows.h>
#include <stdio.h>
#include <locale.h>
#include <atlimage.h>

HRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

HWND Button_SelectFile, Button_Progress,EditInfo;
char FilePath[100] = {NULL};
HWND MainHwnd;
void AppendWindowText(HWND hwnd, char* TextToAppend, bool ShouldEndl = false);
bool IsTag(char Tag[]);
void BeginProcess(bool ShouldCloseFile);
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
	RECT rc = { 0,0,600,500};
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, false);
	MainHwnd =CreateWindow(wnd.lpszClassName, TEXT("MP3Progress"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right-rc.left,rc.bottom-rc.top, NULL, NULL, hInstance, NULL);
	ShowWindow(MainHwnd, nShowCmd);
	MSG msg = {NULL};
	while (msg.message!=WM_QUIT)
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
	case WM_CREATE:
		Button_SelectFile = CreateWindow(TEXT("BUTTON"), TEXT("选择文件"), WS_CHILDWINDOW | WS_VISIBLE, 0, 0, 100, 50, hwnd, NULL, NULL, NULL);
		Button_Progress= CreateWindow(TEXT("BUTTON"), TEXT("处理"), WS_CHILDWINDOW | WS_VISIBLE, 100, 0, 100, 50, hwnd, NULL, NULL, NULL);
		EditInfo = CreateWindow(TEXT("Edit"), TEXT(""), WS_CHILDWINDOW | WS_VISIBLE|WS_VSCROLL| WS_EX_CLIENTEDGE | WS_EX_OVERLAPPEDWINDOW | ES_MULTILINE | ES_NOHIDESEL,0, 50, 200, 450, hwnd, NULL, NULL, NULL);
		break;
	case WM_COMMAND:
		if ((HWND)lParam== Button_SelectFile&&HIWORD(wParam) == BN_CLICKED)
		{
			WCHAR FileP[MAX_PATH] = {NULL};
			OPENFILENAME OpenFN = { NULL };
			OpenFN.lStructSize = sizeof(OPENFILENAME);
			OpenFN.hwndOwner = hwnd;
			OpenFN.lpstrFilter = TEXT("MP3文件\0*.mp3\0\0");
			OpenFN.nMaxFile = MAX_PATH;
			OpenFN.lpstrFile = FileP;
			OpenFN.Flags = OFN_FILEMUSTEXIST;
			OpenFN.lpstrTitle = TEXT("选择音乐文件");
			if (GetOpenFileName(&OpenFN))
			{
				setlocale(LC_ALL, "");
				wcstombs (FilePath,OpenFN.lpstrFile,MAX_PATH);
				AppendWindowText(EditInfo, FilePath,true);
				AppendWindowText(EditInfo, "*MP3路径已载入*", true);
			}
		} 
		else if ((HWND)lParam == Button_Progress&&HIWORD(wParam) == BN_CLICKED)
		{
			BeginProcess(true);
		}
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hwnd, message, wParam, lParam);
		break;
	}
	return 0;
}

void AppendWindowText(HWND hwnd,char* TextToAppend,bool ShouldEndl)
{
	char TEXTO[65535];
	GetWindowTextA(hwnd, TEXTO, 65535);
	strcat(TEXTO, TextToAppend);
	if (ShouldEndl==true)
		strcat(TEXTO, "\r\n");
	SetWindowTextA(hwnd, TEXTO);
	SendMessage(hwnd, WM_VSCROLL, SB_BOTTOM, 0);
}

bool IsTag(char Tag[])
{
	for (int i=0;i<4;i++)
	{
		if ((Tag[i] - '0' < 0 && Tag[i] - '0'>9) || (Tag[i] - 'a' < 0 && Tag[i] - 'z'>0) || (Tag[i] - 'A' < 0 || Tag[i] - 'Z'>0))
			return false;
	}
	return true;
}

void BeginProcess(bool ShouldCloseFile)
{
	char ITOABuffer[30] = { NULL };
	char Header[4] = { NULL }; /*必须为"ID3"否则认为标签不存在*/
	char Ver; /*版本号ID3V2.3 就记录3*/
	char Revision; /*副版本号此版本记录为0*/
	char Flag; /*存放标志的字节，这个版本只定义了三位，稍后详细解说*/
	char Size[4]; /*标签大小，包括标签头的10 个字节和所有的标签帧的大小*/
	int total_size;
	FILE *File = fopen(FilePath, "rb");
	if (!File)
	{
		AppendWindowText(EditInfo, "文件打开失败", true);
		return ;
	}
	fscanf(File, "%3c", Header);
	if (strcmp(Header, "ID3"))
	{
		MessageBox(MainHwnd, TEXT("该MP3没有ID3标签"), TEXT("No ID3 Tag"), MB_OK);
		fclose(File);
		return ;
	}
	fscanf(File, "%c%c%c", &Ver, &Revision, &Flag);
	fscanf(File, "%4c", Size);
	total_size = (Size[0] & 0x7F) * 0x200000 + (Size[1] & 0x7F) * 0x400 + (Size[2] & 0x7F) * 0x80 + (Size[3] & 0x7F);
	AppendWindowText(EditInfo, "ID3V2.");
	AppendWindowText(EditInfo, itoa((int)Ver, ITOABuffer, 30), true);
	AppendWindowText(EditInfo, "Tag Total Size:");
	sprintf(ITOABuffer, "%d", total_size);
	AppendWindowText(EditInfo, ITOABuffer, true);
	char ID[5] = { NULL }; /*用四个字符标识一个帧，说明其内容，稍后有常用的标识对照表*/
	char Flags[3]; /*存放标志，只定义了6 位，稍后详细解说*/
	int FSize;
	unsigned char* Pic;
	for (int i = 0;; /*i < total_size;i += 10*/)
	{
		fscanf(File, "%4c", ID);
		fscanf(File, "%4c", Size);
		fscanf(File, "%3c", Flags);
		if (!IsTag(ID))
			break;
		FSize = Size[0] * 0x100000000 + Size[1] * 0x10000 + Size[2] * 0x100 + Size[3];
		AppendWindowText(EditInfo, "TagID:");
		AppendWindowText(EditInfo, ID, true);
		if (FSize == 0)
			continue;
		Pic = new unsigned char[FSize - 1];
		//Pic = new  unsigned char[FSize];
		if (!fread(Pic, 1, FSize - 1, File))
			MessageBox(MainHwnd, TEXT("FileReadError"), TEXT("Error"), MB_OK);
		if (!strcmp(ID, "APIC"))																			//处理专辑图片
		{
			AppendWindowText(EditInfo, "发现专辑封面", true);
			AppendWindowText(EditInfo, "抽取图片...", true);
			HDC hdc = NULL;
			__try
			{
				IStream* stream = NULL;
				CreateStreamOnHGlobal(NULL, false, &stream);
				stream->Write(&Pic[13], FSize - 14, NULL);
				CImage* CI = new CImage();
				CI->Load(stream);
				stream->Release();
				/*FILE* AlbumPic = fopen("E:\\AlbumPic.jpg", "wb");
				if (!fwrite(&Pic[Offset],1, FSize- Offset, AlbumPic))
				MessageBox(hwnd, TEXT("FileWriteError"), TEXT("Error"), MB_OK);
				fclose(AlbumPic);
				CI->Load(TEXT("E:\\AlbumPic.jpg"));*/
				hdc = GetDC(MainHwnd);
				SetStretchBltMode(hdc, HALFTONE);
				SetBrushOrgEx(hdc, 0, 0, NULL);
				CI->StretchBlt(hdc, { 200,100,600,500 });
				ReleaseDC(MainHwnd, hdc);
				delete CI;
			}
			__except (EXCEPTION_EXECUTE_HANDLER)
			{
				MessageBox(NULL, TEXT("异常超出预期，终止查找"), TEXT("Exception Occurred"), MB_OK);
				ReleaseDC(MainHwnd, hdc);
				break;
			}
		}
		else if (!strcmp(ID, "TIT2") || !strcmp(ID, "TPE1") || !strcmp(ID, "TALB") || !strcmp(ID, "TCON") || !strcmp(ID, "TIT3"))												//处理标题歌手这类玩意儿
		{
			AppendWindowText(EditInfo, (char*)Pic, true);
		}
		delete[] Pic;
		i += FSize;
	}
	AppendWindowText(EditInfo, "查找结束", true);
	fclose(File);
}