#ifndef _UNICODE
#define _UNICODE
#endif
#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <locale.h>
#include <io.h>
#include <vector>
#include <functional>
#include <winnt.h>
#include <string.h>

using std::vector;
template<typename T>
class Binary_Tree
{
	void TravelElement(std::function<void(Binary_Tree*, int CurDepth, bool IsFromLeftChild, void*&TravelData)> TravelFunction, Binary_Tree* CurTree, int CurDepth, int IsFromLeftChild, void*& pTravelData);
public:
	Binary_Tree* LeftChild=NULL;
	Binary_Tree* RightChild=NULL;
	T* pData=NULL;
	Binary_Tree(T* InitalData);
	Binary_Tree() = default;
	void Travel(std::function<void(Binary_Tree*, int CurDepth, bool IsFromLeftChild, void*&TravelData)> TravelFunction);
	//void Travel(void (*TravelFunction)(Binary_Tree*,int CurDepth,bool IsFromLeftChild,void*&TravelData));
};

struct CharWidget
{
	unsigned char Index = 0;
	long Widget = 0;
};

struct BitFILE
{
	FILE* FileHandle;
	char BitHolder;
	unsigned char WorkingByte;
public:
	static BitFILE* fopenbit(char* FilePath, char *FileMode);
	void fputbit(char Bit);
	void fputbits(char* Bits, int BitLength);
	void fputstr(char* Str);
	void fputdata(char* Data, int Length);
	template<typename T>
	void fputvariable(T Variable);
	char fgetbit();
	template<typename T>
	T fgetvariable();
	void fclosebit();
};

DWORD WINAPI WaitingAnimation(LPVOID lpParameter);
void TravelHaffmanTree(Binary_Tree<CharWidget>* CurTree, int CurDepth, bool IsFromLeftChild, void*& TravelData);
void PutHaffmanCodeToFile(BitFILE* FileToPut,unsigned char OriginalChar);
//void SaveHaffmanTree(BitFILE* FileToSaveTree, Binary_Tree& TreeToSave);

HRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

RECT rc = { 0,0,500,500 };
HWND MainHwnd, Button_SelectFile;
BOOL Ziping = FALSE;
vector<unsigned char> CharMap[256];
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nShowCmd)
{
	WNDCLASSEX wnd = { NULL };
	wnd.cbSize = sizeof(WNDCLASSEX);
	wnd.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wnd.hCursor = LoadCursor(0, IDC_ARROW);
	wnd.hInstance = hInstance;
	wnd.lpfnWndProc = MainWndProc;
	wnd.lpszClassName = TEXT("ZipFile");
	RegisterClassEx(&wnd);
	AdjustWindowRect(&rc, WS_OVERLAPPEDWINDOW, FALSE);
	MainHwnd = CreateWindow(wnd.lpszClassName, TEXT("Zip File With Haffman Code"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);
	ShowWindow(MainHwnd, nShowCmd);
	MSG msg = { NULL };
	while (msg.message != WM_QUIT)
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
		GetMessage(&msg, NULL, NULL, NULL);
	}
	return 0;
}

int compare(const void *a, const void *b)
{
	return ((CharWidget*)a)->Widget > ((CharWidget*)b)->Widget;
}

HRESULT CALLBACK MainWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
		Button_SelectFile = CreateWindow(TEXT("BUTTON"), TEXT("选择文件"), WS_CHILDWINDOW | WS_VISIBLE, 0, 0, 100, 50, hwnd, NULL, NULL, NULL);
		break;
	case WM_COMMAND:
		if ((HWND)lParam == Button_SelectFile&&HIWORD(wParam) == BN_CLICKED)
		{
			WCHAR FileP[MAX_PATH] = { NULL };
			OPENFILENAME OpenFN = { NULL };
			OpenFN.lStructSize = sizeof(OPENFILENAME);
			OpenFN.hwndOwner = hwnd;
			OpenFN.lpstrFilter = TEXT("所有文件\0*.*\0Haffman压缩文件\0*.haffman\0");
			OpenFN.nMaxFile = MAX_PATH;
			OpenFN.lpstrFile = FileP;
			OpenFN.Flags = OFN_FILEMUSTEXIST;
			OpenFN.lpstrTitle = TEXT("待压缩/解压的文件");
			if (GetOpenFileName(&OpenFN))
			{
				AllocConsole();//分配控制台
				freopen("CONOUT$", "w", stdout);//重定向标准I/O流
				freopen("CONIN$", "r", stdin);
				CreateThread(NULL, 0, WaitingAnimation, NULL, 0, NULL);
				setlocale(LC_ALL, "");
				char FilePath[256];
				wcstombs(FilePath, OpenFN.lpstrFile, MAX_PATH);
				if (stricmp(FilePath+strlen(FilePath)-8,".haffman"))
				{//压缩
					printf("分析权值。。。");
					FILE* TargetFile = fopen(FilePath, "rb+");
					CharWidget CharMapTmp[256] = { 0 };
					for (int i = 0; i < 256; i++)
						CharMapTmp[i].Index = i;
					while (!feof(TargetFile))
						CharMapTmp[(unsigned char)fgetc(TargetFile)].Widget++;
					printf("\n构造哈夫曼树。。。");
					//qsort(CharMap, 256, sizeof(CharWidget), compare);
					vector<Binary_Tree<CharWidget>> TreeQueue;
					for (int i = 0; i < 256; i++)
						if (CharMapTmp[i].Widget)
							TreeQueue.push_back(Binary_Tree<CharWidget>(&CharMapTmp[i]));
					while (TreeQueue.size() > 1)
					{
						int Min = 0;
						int Second = 1;
						for (int i = 0; i < TreeQueue.size(); i++)
						{
							if (((CharWidget*)TreeQueue[i].pData)->Widget < ((CharWidget*)TreeQueue[Min].pData)->Widget)
							{
								Second = Min;
								Min = i;
							}
						}
						CharWidget TmpCharW = { 0,((CharWidget*)TreeQueue[Min].pData)->Widget + ((CharWidget*)TreeQueue[Second].pData)->Widget };
						Binary_Tree<CharWidget> TmpHaffmanTree(&TmpCharW);
						TmpHaffmanTree.LeftChild = new Binary_Tree<CharWidget>(TreeQueue[Min]);
						TmpHaffmanTree.RightChild = new Binary_Tree<CharWidget>(TreeQueue[Second]);
						//delete TreeQueue[Min].pData;
						TreeQueue[Min] = TmpHaffmanTree;
						TreeQueue.erase(TreeQueue.begin() + Second);
					}
					TreeQueue[0].Travel(TravelHaffmanTree);
					printf("\n编码文件。。。");
					fseek(TargetFile, 0, SEEK_SET);
					WCHAR FileP[MAX_PATH] = { NULL };
					OPENFILENAME OpenFN = { NULL };
					OpenFN.lStructSize = sizeof(OPENFILENAME);
					OpenFN.hwndOwner = hwnd;
					OpenFN.lpstrFilter = TEXT("压缩文件\0*.Haffman\0\0");
					OpenFN.nMaxFile = MAX_PATH;
					OpenFN.lpstrFile = FileP;
					OpenFN.Flags = OFN_FILEMUSTEXIST;
					OpenFN.lpstrTitle = TEXT("选择保存位置");
					if (GetSaveFileName(&OpenFN))
					{
						char FilePath[256];
						wcstombs(FilePath, OpenFN.lpstrFile, MAX_PATH);
						if (access(FilePath, 0))
							strcat(FilePath, ".haffman");
						BitFILE* ZipFile = BitFILE::fopenbit(FilePath, "wb+");
						TreeQueue[0].Travel([ZipFile](Binary_Tree<CharWidget>* CurTree, int CurDepth, bool IsFromLeftChild, void *&TravelData)         //Save Tree
						{
							if (!CurTree->LeftChild && !CurTree->RightChild)
							{
								ZipFile->fputbit(1);
								unsigned char TmpNumber = CurDepth;
								//ZipFile->fputvariable<unsigned char>(TmpNumber);
								ZipFile->fputvariable<unsigned char>(((CharWidget*)(CurTree->pData))->Index);
								//for (unsigned int i = 0; i < CharMap[((CharWidget*)(CurTree->pData))->Index].size(); i++)
									//ZipFile->fputbit(CharMap[((CharWidget*)(CurTree->pData))->Index][i]);
							}
							else
								ZipFile->fputbit(0);
						});
						fseek(TargetFile, 0, SEEK_END);
						ZipFile->fputvariable<long>(ftell(TargetFile));
						fseek(TargetFile, 0, SEEK_SET);
						while (!feof(TargetFile))
							PutHaffmanCodeToFile(ZipFile, (unsigned char)fgetc(TargetFile));
						ZipFile->fclosebit();
						Ziping = FALSE;
						printf("Finish");
						MessageBox(hwnd, TEXT("压缩完成"), TEXT(""), MB_OK);
					}
					else
					{
						MessageBox(hwnd, TEXT("放弃输出文件"), TEXT("ZipFile With HaffmanCode"), MB_OK);
					}
				}
				else
				{//解压
					printf("构造哈夫曼树...");
					Binary_Tree<unsigned char> HaffmanTree;
					HaffmanTree.LeftChild = new Binary_Tree<unsigned char>();
					HaffmanTree.RightChild = new Binary_Tree<unsigned char>();
					auto ZipFile= BitFILE::fopenbit(FilePath, "rb+");
					//ZipFile->fgetbit();
					HaffmanTree.Travel([ZipFile](Binary_Tree<unsigned char>* CurTree, int CurDepth, bool IsFromLeftChild, void *&TravelData)
					{
						if (ZipFile->fgetbit())
						{//叶子
							CurTree->pData = new unsigned char(ZipFile->fgetvariable<unsigned char>());
						}
						else
						{
							CurTree->LeftChild = new Binary_Tree<unsigned char	>();
							CurTree->RightChild = new Binary_Tree<unsigned char>();
						}
					});
					long FileSize = ZipFile->fgetvariable<long>();
					printf("\n解压文件...");
					WCHAR FileP[MAX_PATH] = { NULL };
					OPENFILENAME OpenFN = { NULL };
					OpenFN.lStructSize = sizeof(OPENFILENAME);
					OpenFN.hwndOwner = hwnd;
					OpenFN.lpstrFilter = TEXT("原始文件\0*.*\0\0");
					OpenFN.nMaxFile = MAX_PATH;
					OpenFN.lpstrFile = FileP;
					OpenFN.Flags = OFN_FILEMUSTEXIST;
					OpenFN.lpstrTitle = TEXT("选择保存位置");
					if (GetSaveFileName(&OpenFN))
					{
						char FilePath[256];
						wcstombs(FilePath, OpenFN.lpstrFile, MAX_PATH);
						auto TargetFile= fopen(FilePath, "wb+");
						while (ftell(TargetFile)<FileSize)
						{
							Binary_Tree<unsigned char>* TmpTreeRef = &HaffmanTree;
							while (1)
							{
								TmpTreeRef = ((!ZipFile->fgetbit())? TmpTreeRef->LeftChild: TmpTreeRef->RightChild);
								if (!TmpTreeRef->LeftChild && !TmpTreeRef->RightChild)
								{
									fputc(*(TmpTreeRef->pData), TargetFile);
									break;
								}
							}
						}
						ZipFile->fclosebit();
						fclose(TargetFile);
						printf("\n解压完成");
					}
					else
						MessageBox(hwnd, TEXT("放弃输出文件"), TEXT("ZipFile With HaffmanCode"), MB_OK);
				}
			}
			break;
		}
	case WM_DESTROY:
		PostQuitMessage(0);
	}
	return DefWindowProc(hwnd, message, wParam, lParam);
}

void TravelHaffmanTree(Binary_Tree<CharWidget>* CurTree, int CurDepth, bool IsFromLeftChild, void*& TravelData)
{
	if (!TravelData)
	{
		TravelData = new vector<unsigned char>(255);
		return;
	}
	vector<unsigned char>* pTravelData = (vector<unsigned char>*) TravelData;
	if (IsFromLeftChild)
		(*pTravelData)[CurDepth-2]=0;
	else
		(*pTravelData)[CurDepth-2]=1;
	if (!CurTree->LeftChild&&!CurTree->RightChild)
	{
		//叶子
		CharMap[CurTree->pData->Index].resize(CurDepth-1);
		for (int i=0;i<CurDepth-1;i++)
			CharMap[CurTree->pData->Index][i] = (*pTravelData)[i];
		//((vector<char>*)TravelData)-> (((vector<char>*)TravelData)->begin() + ((vector<char>*)TravelData)->size() - 1);
	}
}

DWORD WINAPI WaitingAnimation(LPVOID lpParameter)
{
	Ziping = TRUE;
	while (!Ziping)
	{
		printf("--\b");
		Sleep(200);
		if (!Ziping)
			return 0;
		printf("/\b");
		Sleep(200);
		if (!Ziping)
			return 0;
		printf("|\b");
		Sleep(200);
		if (!Ziping)
			return 0;
		printf("\\\b");
		Sleep(200);
		if (!Ziping)
			return 0;
	}
}

void PutHaffmanCodeToFile(BitFILE* FileToPut,unsigned char OriginalChar)
{
	for (int i=0;i<CharMap[OriginalChar].size();i++)
		FileToPut->fputbit(CharMap[OriginalChar][i] ? 1 : 0);
}

void SaveHaffmanKey(FILE* TargetFile)
{
	for (int i=0;i<256;i++)
	{
		unsigned short HaffmanKey= CharMap[i].size();
		for (int k = 0; k < CharMap[i].size();k++)
			(HaffmanKey <<= 1) += (CharMap[i][k]-'0');
		HaffmanKey <<= 13 - CharMap[i].size();
		fputc(*(char*)&HaffmanKey, TargetFile);
		fputc(*((char*)&HaffmanKey+1), TargetFile);
	}
}

template<typename T>
void Binary_Tree<T>::TravelElement(std::function<void(Binary_Tree<T>*, int CurDepth, bool IsFromLeftChild, void*&TravelData)> TravelFunction,Binary_Tree* CurTree, int CurDepth,int IsFromLeftChild, void*& pTravelData)
{
	TravelFunction(CurTree, CurDepth, IsFromLeftChild, pTravelData);
	if (CurTree->LeftChild)
		TravelElement(TravelFunction,CurTree->LeftChild, CurDepth+1, TRUE, pTravelData);
	if (CurTree->RightChild)
		TravelElement(TravelFunction,CurTree->RightChild, CurDepth + 1, FALSE, pTravelData);
}

template<typename T>
Binary_Tree<T>::Binary_Tree(T * InitalData)
{
	pData = new T();
	*pData = *InitalData;
}

template<typename T>
void Binary_Tree<T>::Travel(std::function<void(Binary_Tree<T>*, int CurDepth, bool IsFromLeftChild, void*&TravelData)> TravelFunction)
{
	void* pTravelData=NULL;
	TravelElement(TravelFunction,this,1,FALSE,pTravelData);
	try
	{
		delete pTravelData;
	}
	catch(...){}
}

BitFILE* BitFILE::fopenbit(char * FilePath, char * FileMode)
{
	BitFILE* TmpBitFile=new BitFILE();
	TmpBitFile->BitHolder = 0;
	TmpBitFile->FileHandle = fopen(FilePath, FileMode);
	TmpBitFile->WorkingByte = 0;
	return TmpBitFile;
}

void BitFILE::fputbit(char Bit)
{
	WorkingByte += (Bit-'0'>=0?Bit-'0':Bit) << (7 - BitHolder);
	BitHolder = (BitHolder + 1) % 8;
	if (!BitHolder)
	{
		fputc(WorkingByte, FileHandle);
		WorkingByte = 0;
	}
}

void BitFILE::fputbits(char * Bits, int BitLength)
{
	for (int i=0;i<BitLength;i++)
		fputbit((*(Bits +(int)(i/8)))&(1 << (7 - i%8)));
}

void BitFILE::fputstr(char * Str)
{
	while (*Str)
	{
		for (int i=0;i<8;i++)
			fputbit((*Str)&(1 << (7 - i)));
		Str++;
	}
}

void BitFILE::fputdata(char * Data, int Length)
{
	for (int k=0;k<Length;k++)
	{
		for (int i = 0; i < 8; i++)
			fputbit((*Data)&(1 << (7 - i)));
		Data++;
	}
}

char BitFILE::fgetbit()
{
	unsigned char TmpC= (unsigned char)fgetc(FileHandle);
	TmpC = TmpC >> (7 - BitHolder) & 1;
	BitHolder = (BitHolder + 1) % 8;
	if (BitHolder)
		fseek(FileHandle, -1, SEEK_CUR);
	return (char)TmpC;
}

void BitFILE::fclosebit()
{
	if (BitHolder)
		fputc(WorkingByte, FileHandle);
	fclose(FileHandle);
	delete this;
}

template<typename T>
void BitFILE::fputvariable(T Variable)
{
	for (int k = 1; k <= sizeof(T)*8; k++)
	{
			fputbit(Variable>>(sizeof(T) * 8 -k)&1);
	}
}

template<typename T>
T BitFILE::fgetvariable()
{
	T K = 0;
	for (int i = 0; i < sizeof(T)*8; i++)
	{
		K <<= 1;
		K += fgetbit();
	}
	return K;
}
