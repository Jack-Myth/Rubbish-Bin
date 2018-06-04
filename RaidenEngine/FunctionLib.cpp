#define  _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <initializer_list>
#include <string.h>
#include <stdlib.h>
#include <windows.h>
#include <locale.h>
#include <wincodec.h>
#include <d2d1.h>
#include <xaudio2.h>
#include "BaseInformation.h"
#include "FunctionLib.h"
#include <mmsystem.h>
#include "./Library/WaveFile.h"
#include "./Library/exprtk.hpp"
#include <mutex>

extern BaseInformation Sys;
extern exprtk::symbol_table<double> GlobalGameFlag;
std::mutex CaculaterLock;
exprtk::parser<double> ExpressionParser;
template <typename T>
bool IsArrayContains(T ElementToFind, std::initializer_list<T> Array)
{
	for (T x : Array)
	{
		if (x == ElementToFind)
			return true;
	}
	return false;
}

//Useing in ExecuteScript
/*
@param ScriptFile ScriptHandle
@param Array char list that should be ate
@return Return last ate char
*/
char EatChar(FILE* ScriptFile, std::initializer_list<char> Array = { ' ', '\t', '\n', '\r' })
{
	char ChrTmp = 0;
	do
	{
		ChrTmp = fgetc(ScriptFile);
	} while (IsArrayContains<char>(ChrTmp, Array));
	fseek(ScriptFile, -2, SEEK_CUR);
	return fgetc(ScriptFile);
}

int StrReplace(char strRes[], char from[], char to[])
{
	int i, flag = 0;
	char *p, *q, *ts;
	for (i = 0; strRes[i]; ++i)
	{
		if (strRes[i] == from[0])
		{
			p = strRes + i;
			q = from;
			while (*q && (*p++ == *q++));
			if (*q == '\0')
			{
				ts = (char *)malloc(strlen(strRes) + 1);
				strcpy(ts, p);
				strRes[i] = '\0';
				strcat(strRes, to);
				strcat(strRes, ts);
				free(ts);
				flag = 1;
			}
		}
	}
	return flag;
}

void VS_StrTrim(char* pStr)
{
	char *pTmp = pStr;
	while (*pStr != '\0')
	{
		if (*pStr != ' ')
			*pTmp++ = *pStr;
		++pStr;
	}
	*pTmp = '\0';
}

double CaculateExpression(char OriginExpression[], exprtk::symbol_table<double>& SymbolTable)
{
	exprtk::expression<double> m_expression;
	StrReplace(OriginExpression, "[", "(");
	StrReplace(OriginExpression, "]", ")");
	StrReplace(OriginExpression, "\\(", "[");
	StrReplace(OriginExpression, "\\)", "]");
	CaculaterLock.lock();
	m_expression.register_symbol_table(SymbolTable);
	m_expression.register_symbol_table(GlobalGameFlag);
	ExpressionParser.compile(std::string(OriginExpression), m_expression);
	CaculaterLock.unlock();
	return m_expression.value(); //表达式析构时会自动调用release()，所以不用手动调用
}

/*int CaculateExpression(char Originexpression[],int GameFlag[])
{
	char expression[200];
	int A, B;
	char C;
	char* FindPoint = 0;
	int FlagNumber = 0;
	char TmpStr[20] = { NULL };
	strcpy(expression, Originexpression);
	VS_StrTrim(expression);
	//if (expression[0] == '-')
	//{
	//	char TmpZero[200] = "0";
	//	strcat(TmpZero, expression);
	//	strcmp(expression, TmpZero);
	//}
	while (FindPoint = strstr(expression, "flag["))
	{
		sscanf(FindPoint, "flag[%d]", &FlagNumber);
		sprintf(TmpStr, "flag[%d]", FlagNumber);
		sprintf(&TmpStr[10], "%d", GameFlag[FlagNumber]);
		StrReplace(expression, TmpStr, &TmpStr[10]);
	}
	if (expression[0] == '-')
	{
		char TmpZero[200] = "0";
		strcat(TmpZero, expression);
		strcmp(expression, TmpZero);
	}
	while (FindPoint = (char*)GetMinPoint(strchr(expression, '*'), strchr(expression, '/')))
	{
		while ((INT64)(--FindPoint) >= (INT64)expression && ('0' <= *FindPoint&&*FindPoint <= '9'));
		FindPoint++;
		sscanf(FindPoint, "%d%c%d", &A, &C, &B);
		sprintf(TmpStr, "%d%c%d", A, C, B);
		switch (C)
		{
		case '*':
			sprintf(&TmpStr[15], "%d", A * B);
			break;
		case '/':
			sprintf(&TmpStr[15], "%d", A / B);
			break;
		}
		StrReplace(expression, TmpStr, &TmpStr[15]);
	}
	while (FindPoint = (char*)GetMinPoint(strchr(expression, '+'), strchr(expression, '-')))
	{
		while ((INT64)(--FindPoint) >= (INT64)expression && ('0' <= *FindPoint&&*FindPoint <= '9'));
		FindPoint++;
		sscanf(FindPoint, "%d%c%d", &A, &C, &B);
		sprintf(TmpStr, "%d%c%d", A, C, B);
		switch (C)
		{
		case '+':
			sprintf(&TmpStr[15], "%d", A + B);
			break;
		case '-':
			sprintf(&TmpStr[15], "%d", A - B);
			break;
		}
		StrReplace(expression, TmpStr, &TmpStr[15]);
	}
	FindPoint = expression;
	do
	{
		if (!(strchr(expression, '<') || strchr(expression, '>') || strchr(expression, '=')))
			break;
		if (*FindPoint == '&')
			*FindPoint = '*';
		if (*FindPoint == '|')
			*FindPoint = '/';
		if (*FindPoint == '/' || *FindPoint == '*')
			FindPoint++;
		sscanf(FindPoint, "%d%c%d", &A, &C, &B);
		sprintf(TmpStr, "%d%c%d", A, C, B);
		switch (C)
		{
		case '<':
			StrReplace(expression, TmpStr, _itoa((int)(A < B), &TmpStr[15], 10));
			break;
		case '>':
			StrReplace(expression, TmpStr, _itoa((int)(A > B), &TmpStr[15], 10));
			break;
		case '=':
			StrReplace(expression, TmpStr, _itoa((int)(A == B), &TmpStr[15], 10));
			break;
		}
	} while (FindPoint = (char*)GetMinPoint(strchr(expression, '&'), strchr(expression, '|')));
	while (strchr(expression, '/') || strchr(expression, '*'))
	{
		sscanf(expression, "%d%c%d", &A, &C, &B);
		sprintf(TmpStr, "%d%c%d", A, C, B);
		switch (C)
		{
		case '*':
			StrReplace(expression, TmpStr, _itoa((int)(A && B), &TmpStr[15], 10));
			break;
		case '/':
			StrReplace(expression, TmpStr, _itoa((int)(A || B), &TmpStr[15], 10));
			break;
		}
	}
	return atoi(expression);
}*/

void* GetMinPoint(void* A, void* B)
{
	if ((INT64)A == 0)
		return B;
	if ((INT64)B == 0)
		return A;
	if ((INT64)A > (INT64)B)
		return B;
	else
		return A;
}

void FindElsePlace(FILE* FileHandle)
{
	char Command[50] = { NULL };
	int IfStack = 0;
	while (1)
	{
		if ((Command[0] = fgetc(FileHandle)) != '\n'&&Command[0] != ' '&&Command[0] != '\t')
			fseek(FileHandle, -1, SEEK_CUR);
		else
			continue;
		if (fgetc(FileHandle) != '#')
			fseek(FileHandle, -1, SEEK_CUR);
		else
		{
			fseek(FileHandle, -1, SEEK_CUR);
			fscanf(FileHandle, "%*s");
			continue;
		}
		fscanf(FileHandle, "%[^(]", Command);
		if (!strcmp(Command, "endif"))
		{
			IfStack--;
			if (IfStack < 0)
			{
				fscanf(FileHandle, "%*s");
				return;
			}
		}
		else if (!strcmp(Command, "else"))
		{
			if (IfStack <= 0)
			{
				fscanf(FileHandle, "%*s");
				return;
			}
		}
		else if (!strcmp(Command, "if"))
		{
			IfStack++;
		}
		fscanf(FileHandle, "%*[^\n]");
	}
}

void FindCloseCase(FILE* FileHandle,char BeginCase[], char CloseCase[])
{
	char Command[50] = { NULL };
	int IfStack = 0;
	while (1)
	{
		if ((Command[0] = fgetc(FileHandle)) != '\n'&&Command[0] != ' '&&Command[0] != '\t')
			fseek(FileHandle, -1, SEEK_CUR);
		else
			continue;
		if (fgetc(FileHandle) != '#')
			fseek(FileHandle, -1, SEEK_CUR);
		else
		{
			fseek(FileHandle, -1, SEEK_CUR);
			fscanf(FileHandle, "%*s");
			continue;
		}
		fscanf(FileHandle, "%[^(]", Command);
		if (!strcmp(Command, CloseCase))
		{
			IfStack--;
			if (IfStack < 0)
			{
				fscanf(FileHandle, "%*s");
				return;
			}
		}
		else if (!strcmp(Command, BeginCase))
		{
			IfStack++;
		}
		fscanf(FileHandle, "%*[^\n]");
	}
}

bool LoadD2DImage(WCHAR ImagePath[], D2D1BitmapReference** BitmapRefrence)
{
	if (!wcscmp(_wcsupr(ImagePath), TEXT("NULL"))) //此处利用Windows不区分文件大小写的特性，可以直接在源字符串上转换
	{
		D2D1BitmapCpyRef(BitmapRefrence, NULL);
		return false;
	}
	char ImagePathA[256];
	wcstombs(ImagePathA, ImagePath, 256);
	{
		auto it = D2D1BitmapPool.find(std::string(ImagePathA));
		if (it !=D2D1BitmapPool.end())
		{
			D2D1BitmapReference* BitmapR = &it->second;
			D2D1BitmapCpyRef(BitmapRefrence, BitmapR);
			return true;
		}
	}
	IWICBitmap* m_pWicBitmap = NULL;
	IWICBitmapDecoder* m_pWicDecoder = NULL;
	IWICBitmapFrameDecode* m_pWicFrameDecoder = NULL;
	if (FAILED(Sys.WICImagingFactory->CreateDecoderFromFilename(ImagePath, nullptr, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &m_pWicDecoder)))
		return false;
	m_pWicDecoder->GetFrame(0, &m_pWicFrameDecoder);
	IWICBitmapSource* pWicSource = NULL;
	m_pWicFrameDecoder->QueryInterface(IID_PPV_ARGS(&pWicSource));
	IWICFormatConverter* pCovert = NULL;
	Sys.WICImagingFactory->CreateFormatConverter(&pCovert);
	pCovert->Initialize(
		pWicSource,
		GUID_WICPixelFormat32bppPBGRA,
		WICBitmapDitherTypeNone,
		NULL,
		0.f,
		WICBitmapPaletteTypeCustom
	);
	Sys.WICImagingFactory->CreateBitmapFromSource(pCovert, WICBitmapCacheOnDemand, &m_pWicBitmap);
	D2D1BitmapReference BitmapRTmp;
	Sys.D2D1HwndRenderTarget->CreateBitmapFromWicBitmap(m_pWicBitmap, NULL, &BitmapRTmp.D2DBitmap);
	BitmapRTmp.PicturePath = std::string(ImagePathA);
	BitmapRTmp.ReferenceCount = 0;
	D2D1BitmapPool.insert_or_assign(BitmapRTmp.PicturePath, BitmapRTmp);
	pCovert->Release();
	m_pWicDecoder->Release();
	m_pWicFrameDecoder->Release();
	return LoadD2DImage(ImagePath, BitmapRefrence);
}

bool LoadD2DImage(char ImagePath[], D2D1BitmapReference** BitmapRefrence)
{
	WCHAR WCSImagePath[256] = {NULL};
	mbstowcs(WCSImagePath, ImagePath, 256);
	RenderLock.lock();
	bool returnValue = LoadD2DImage(WCSImagePath, BitmapRefrence);
	RenderLock.unlock();
	return returnValue;
}

D2D1BitmapReference* D2D1BitmapCpyRef(D2D1BitmapReference** Dest, D2D1BitmapReference* Source)
{
	if (*Dest == Source)
		return Source;
	if (*Dest)
	{
		(*Dest)->ReferenceCount--;
		if ((*Dest)->ReferenceCount <= 0)
		{
			(*Dest)->D2DBitmap->Release();
			D2D1BitmapPool.erase((*Dest)->PicturePath);
		}
	}
	*Dest = Source;
	if (Source)
		Source->ReferenceCount++;
	return Source;
}

SoundReference* SoundCpyRef(SoundReference** Dest, SoundReference* Source)
{
	if (*Dest == Source)
		return Source;
	if (*Dest)
	{
		(*Dest)->ReferenceCount--;
		if ((*Dest)->ReferenceCount <= 0)
		{
			delete (*Dest)->SoundBuffer;
			SoundPool.erase((*Dest)->SoundPath);
		}
	}
	*Dest = Source;
	if (Source)
		Source->ReferenceCount++;
	return Source;
}

bool LoadSound(WCHAR SoundPath[], SoundReference** SoundRef)
{
	if (!wcscmp(_wcsupr(SoundPath), TEXT("NULL"))) //此处利用Windows不区分文件大小写的特性，可以直接在源字符串上转换
	{
		SoundCpyRef(SoundRef, NULL);
		return false;
	}
	char SoundPathA[256] = {NULL};
	wcstombs(SoundPathA, SoundPath,256);
	{
		auto it = SoundPool.find(std::string(SoundPathA));
		if (it!=SoundPool.end())
		{
			SoundReference* SoundR = &it->second;
			SoundCpyRef(SoundRef, SoundR);
			return true;
		}
	}
	CWaveFile m_WaveFile;
	if (FAILED(m_WaveFile.Open(SoundPath, NULL, WAVEFILE_READ)))
		return false;
	SoundReference SoundRTmp;
	SoundRTmp.BufferLength = m_WaveFile.GetSize();
	SoundRTmp.WaveFormat = *m_WaveFile.GetFormat();
	BYTE* SoundBuffer = new BYTE[SoundRTmp.BufferLength];
	DWORD ReadedLength;
	if (FAILED(m_WaveFile.Read(SoundBuffer, SoundRTmp.BufferLength, &ReadedLength)))
		return false;
	SoundRTmp.SoundBuffer = SoundBuffer;
	SoundRTmp.ReferenceCount = 0;
	SoundRTmp.SoundPath = std::string(SoundPathA);
	SoundPool.insert_or_assign(SoundRTmp.SoundPath, SoundRTmp);
	m_WaveFile.Close();
	return LoadSound(SoundPath, SoundRef);
}

bool LoadSound(char SoundPathA[], SoundReference** SoundRef)
{
	WCHAR SoundPath[256];
	mbstowcs(SoundPath, SoundPathA, 256);
	return LoadSound(SoundPath, SoundRef);
}

void PlayXAudio2Sound(SoundReference* SoundRef)
{
	IXAudio2SourceVoice* SourceV=NULL;
	VoiceCallback* CallbackInstance = new VoiceCallback(SoundRef, new XAUDIO2_BUFFER({NULL}));
	CallbackInstance->XAudio2Buffer->AudioBytes = SoundRef->BufferLength;
	CallbackInstance->XAudio2Buffer->pAudioData = SoundRef->SoundBuffer;
	SoundRef->ReferenceCount++;
	Sys.XAudio2Engine->CreateSourceVoice(&SourceV, &SoundRef->WaveFormat,0, XAUDIO2_DEFAULT_FREQ_RATIO, CallbackInstance,NULL,NULL);
	CallbackInstance->SourceVoice = SourceV;
	SourceV->SubmitSourceBuffer(CallbackInstance->XAudio2Buffer);
	SourceV->Start();
}

ScriptExecutePoint GetExecutePoint(char FilePath[], char TagName[])
{
	ScriptExecutePoint SEPTmp;
	FILE* ScriptHandle = fopen(FilePath,"r+");
	if (!ScriptHandle)
		return SEPTmp;
	if (!strcmp(TagName, "NULL"))
	{
		fclose(ScriptHandle);
		strcpy(SEPTmp.FilePath, FilePath);
		SEPTmp.ScriptPoint = 0;
		return SEPTmp;
	}
	char CharTmp[256];
	while (!feof(ScriptHandle))
	{
		if (fgetc(ScriptHandle) != '#')
			continue;
		fscanf(ScriptHandle, "%s", CharTmp);
		if (!strcmp(TagName, CharTmp))
		{
			SEPTmp.ScriptPoint = ftell(ScriptHandle);
			break;
		}
	}
	fclose(ScriptHandle);
	strcpy(SEPTmp.FilePath, FilePath);
	return SEPTmp;
}