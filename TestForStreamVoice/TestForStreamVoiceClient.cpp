#include <stdio.h>
#include <Windows.h>  
#include <winsock.h> 
#pragma comment(lib, "ws2_32.lib")  
#pragma comment(lib, "winmm.lib")    

#define WAVEQ 32000
HWAVEIN hWaveIn;
WAVEFORMATEX WaveFormat = {NULL};
WAVEHDR WaveHDR = {NULL};
WSADATA mWSAData;

int FindSize(BYTE* B);
int main()
{
	WaveFormat.cbSize = 0;
	WaveFormat.nChannels = 1;
	WaveFormat.nSamplesPerSec = 8000;
	WaveFormat.wBitsPerSample = 8;
	WaveFormat.nAvgBytesPerSec = WAVEQ;
	WaveFormat.nBlockAlign = 1;
	WaveFormat.wFormatTag = WAVE_FORMAT_PCM;
	if (WSAStartup(MAKEWORD(1, 1), &mWSAData))
	{
		printf("WSA Startup Failed");
		system("pause>nul");
		exit(-1);
	}
	SOCKET mSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (!mSocket)
	{
		printf("create Socket Failed");
		system("pause>nul");
		exit(-1);
	}
	SOCKADDR_IN Addr = { NULL };
	Addr.sin_family = AF_INET;
	Addr.sin_port = htons(1703);
	Addr.sin_addr.S_un.S_addr = inet_addr("115.159.152.245");
	if (connect(mSocket, (SOCKADDR*)&Addr, sizeof(SOCKADDR)))
	{
		printf("connect Failed");
		system("pause>nul");
		//exit(-1);
	}
	auto WaveData = new BYTE[WAVEQ];
	auto R = waveInOpen(&hWaveIn, 0, &WaveFormat, NULL, NULL, CALLBACK_NULL);
	if (MMSYSERR_NOERROR!=R)
	{
		printf("waveInOpen Failed,Err Info is %d",R);
		system("pause>nul");
		exit(-1);
	}
	WaveHDR.dwBufferLength = WAVEQ;
	WaveHDR.dwBytesRecorded = 0;
	WaveHDR.dwLoops = 1;
	WaveHDR.lpData = (LPSTR)WaveData;
	FILE* fp=fopen("a.pcm", "w+");
	while (1)
	{
		WaveHDR.dwBytesRecorded = 0;
		WaveHDR.dwLoops = 1;
		waveInPrepareHeader(hWaveIn, &WaveHDR, sizeof(WaveHDR));
		waveInAddBuffer(hWaveIn, &WaveHDR, sizeof(WAVEHDR));
		auto ER = waveInStart(hWaveIn);
		Sleep(1000);
		waveInReset(hWaveIn);//Í£Ö¹Â¼Òô  
		int Sze = WaveHDR.dwBytesRecorded;	
		fwrite(WaveData, 1, Sze, fp);
		send(mSocket, (char*)WaveData, Sze, 0);
		printf("Package Sent,Size:%d\n",Sze);
	}
	fclose(fp);
}

int FindSize(BYTE* B)
{
	for (int i = 31999; i > 0; i--)
		if (B[i-1])
			return i;
	return 0;
}