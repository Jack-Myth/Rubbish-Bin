#include <stdio.h>
#include <stdlib.h>
#include <conio.h>
#include <time.h>
#include <windows.h>

int main()
{
	srand(time(NULL));
#ifndef _DEBUG
	IStream* iStream;
	HRSRC hrsc = FindResource(NULL, MAKEINTRESOURCE(100), TEXT("R"));
	HGLOBAL hglo = LoadResource(NULL, hrsc);
	LPVOID LPV = LockResource(hglo);
	ULARGE_INTEGER Ularge = { NULL };
	Ularge.QuadPart = SizeofResource(NULL, hrsc);
	/*DWORD TmpLong = 0;
	CreateStreamOnHGlobal(NULL, true, &iStream);
	iStream->SetSize(Ularge);
	if (FAILED(iStream->Write(LPV, SizeofResource(NULL, hrsc), &TmpLong)))
	{
		MessageBox(NULL, TEXT("Load Resource Failed,Quit"), TEXT("!"), MB_OK);
		return -1;
	}*/
	long pIndex = 0;
	if (!LPV)
		return 0;
	while (Ularge.QuadPart>0)
	{
		switch (getch())
		{
		case '\b':
			printf("\b \b");
			break;
		default:
		{
			char C[3];
			unsigned long Tmps;
			C[0] = *((char*)LPV + pIndex);
			pIndex++;
			Ularge.QuadPart--;
			if (C[0]>0)
				printf("%c", C[0]);
			else
			{
				C[1] = *((char*)LPV + pIndex);
				pIndex++;
				Ularge.QuadPart--;
				C[2] = 0;
				printf("%s", C);
			}
			break;
		}
		}
	}
#endif
}