#pragma once
#include <stdio.h>
#include <initializer_list>
#include "BaseInformation.h"
#include "./Library/exprtk.hpp"

template <typename T>
bool IsArrayContains(T ElementToFind, std::initializer_list<T> Array);
char EatChar(FILE* ScriptFile, std::initializer_list<char> Array);
int StrReplace(char strRes[], char from[], char to[]);
double CaculateExpression(char OriginExpression[], exprtk::symbol_table<double>& SymbolTable);
//int CaculateExpression(char Originexpression[], int GameFlag[]);
void* GetMinPoint(void* A, void* B);
void FindElsePlace(FILE* FileHandle);
void FindCloseCase(FILE* FileHandle, char BeginCase[], char CloseCase[]);
bool LoadD2DImage(char ImagePath[], D2D1BitmapReference** BitmapRefrence);
D2D1BitmapReference* D2D1BitmapCpyRef(D2D1BitmapReference** Dest, D2D1BitmapReference* Source);
bool LoadSound(char SoundPathA[], SoundReference** SoundRef);
SoundReference* SoundCpyRef(SoundReference** Dest, SoundReference* Source);
void PlayXAudio2Sound(SoundReference* SoundRef);
ScriptExecutePoint GetExecutePoint(char FilePath[], char TagName[]);