#include <stdio.h>
#include <vector>
#include <time.h>
#include <windows.h>
#include <algorithm>

double ProcessBySequence(int* Top, int* theTop);
double ProcessBySequenceClassic(int* Top, int* theTop);
double ProcessByChampionship(int* Top, int* theTop);
double ProcessByHeapSort(int* Top, int* theTop);
void HeapAdjust(std::vector<int>& PlayerIndex, int i, int m);
std::vector<int> Player(512);
long long CPUTickStart,CPUTickEnd;
LARGE_INTEGER CPUFrequency;
int main()
{
	srand(time(NULL));
	for ( int i=0;i<512;i++)
	{
		Player[i] = rand() % 1000;
		printf("%d\t",Player[i]);
	}
	printf("\n\n");
	int Top=0,theTop=0;
	double TimeRequest;
	QueryPerformanceFrequency(&CPUFrequency);
	TimeRequest=ProcessBySequence(&Top, &theTop);
	printf("Use Sequence In %.6lfms:",TimeRequest);
	printf("the Top:Player%d(%d),Top:Player%d(%d)\n", theTop,Player[theTop], Top,Player[Top]);
	TimeRequest = ProcessBySequenceClassic(&Top, &theTop);
	printf("Use Sequence Classic In %.6lfms:", TimeRequest);
	printf("the Top:Player%d(%d),Top:Player%d(%d)\n", theTop, Player[theTop], Top, Player[Top]);
	TimeRequest = ProcessByChampionship(&Top, &theTop);
	printf("Use Championship In %.6lfms:", TimeRequest);
	printf("the Top:Player%d(%d),Top:Player%d(%d)\n", theTop, Player[theTop], Top, Player[Top]);
	TimeRequest = ProcessByHeapSort(&Top,&theTop);
	printf("Use HeapSort In %.6lfms:", TimeRequest);
	printf("the Top:Player%d(%d),Top:Player%d(%d)\n", theTop, Player[theTop], Top, Player[Top]);
	system("pause");
}

double ProcessBySequenceClassic(int* Top,int* theTop)
{
	QueryPerformanceCounter((LARGE_INTEGER*)&CPUTickStart);
	for (int i=0;i<512;i++)
	{
		if (Player[i] >= Player[*theTop])
			*theTop = i;
	}
	for (int i = 0; i < 512; i++)
	{
		if (Player[i] >=Player[*Top]&&i!=*theTop)
			*Top = i;
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&CPUTickEnd);
	return (CPUTickEnd - CPUTickStart) / (double)CPUFrequency.QuadPart;
}

double ProcessBySequence(int* Top, int* theTop)
{
	QueryPerformanceCounter((LARGE_INTEGER*)&CPUTickStart);
	for (int i = 0; i < 512; i++)
	{
		if (Player[i] >=Player[*theTop])
		{
			*Top = *theTop;
			*theTop = i;
		}
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&CPUTickEnd);
	return (CPUTickEnd - CPUTickStart) / (double)CPUFrequency.QuadPart;
}

double ProcessByChampionship(int* Top, int* theTop)
{
	QueryPerformanceCounter((LARGE_INTEGER*)&CPUTickStart);
	std::vector<int> PlayerIndex(512);
	for (int i = 0; i < 512; i++)
		PlayerIndex[i] = i;
	while (PlayerIndex.size() > 1)
	{
		int Siz = PlayerIndex.size() / 2;
		for (int i=0;i<PlayerIndex.size()/2;i++)
		{
			PlayerIndex[i] = Player[PlayerIndex[i]] >= Player[PlayerIndex[i + Siz]] ? PlayerIndex[i] : PlayerIndex[i + Siz];
		}
		PlayerIndex.resize(Siz);
	}
	*theTop = PlayerIndex[0];
	PlayerIndex.resize(512);
	for (int i = 0; i < 512; i++)
		PlayerIndex[i] = i;
	int k = *theTop;
	while (PlayerIndex.size() > 1)
	{
		int Siz = PlayerIndex.size() / 2;
		for (int i = 0; i < PlayerIndex.size() / 2; i++)
		{
			if (i == k)
				PlayerIndex[i] = PlayerIndex[i + Siz];
			else if (i + Siz == k)
				continue;
			else
				PlayerIndex[i] = Player[PlayerIndex[i]] >= Player[PlayerIndex[i + Siz]] ? PlayerIndex[i] : PlayerIndex[i + Siz];
		}
		PlayerIndex.resize(Siz);
	}
	QueryPerformanceCounter((LARGE_INTEGER*)&CPUTickEnd);
	return (CPUTickEnd - CPUTickStart) / (double)CPUFrequency.QuadPart;
}

double ProcessByHeapSort(int *Top,int *theTop)
{
	QueryPerformanceCounter((LARGE_INTEGER*)&CPUTickStart);
	int i;
	std::vector<int> PlayerIndex(512);
	for (int i = 0; i < 512; i++)
		PlayerIndex[i] = i;
	std::make_heap(PlayerIndex.begin(), PlayerIndex.end(), [](const int a, const int b) {return Player[a] > Player[b]; });
	sort_heap(PlayerIndex.begin(), PlayerIndex.end(), [](const int a, const int b) {return Player[a] > Player[b]; });
	*theTop = PlayerIndex[0];
	*Top = PlayerIndex[1];
	QueryPerformanceCounter((LARGE_INTEGER*)&CPUTickEnd);
	return (CPUTickEnd - CPUTickStart) / (double)CPUFrequency.QuadPart;
}