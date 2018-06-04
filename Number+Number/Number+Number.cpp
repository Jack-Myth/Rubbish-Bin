#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
	char NumberA[1024] = { 0 };
	char NumberB[1024] = { 0 };
	char R[1024] = { 0 };
	scanf("%s%s", NumberA, NumberB);
	int LengthA = strlen(NumberA);
	int LengthB = strlen(NumberB);
	int BeginPosition = LengthA > LengthB ? LengthA : LengthB;
	int LastNumber = 0;
	for (int i=BeginPosition-1;i>=0;i--)
	{
		if ((LengthA>LengthB?LengthB:LengthA)>i)
		{
			R[i + 1] = NumberA[i]-'0' + NumberB[i]-'0' + LastNumber;
			LastNumber = R[i + 1] >=10?(int)R[i + 1]/10 : 0;
			R[i + 1] = R[i + 1] >=10?R[i + 1] % 10+'0' : R[i + 1]+'0';
		}
		else
		{
			R[i + 1] = LengthA > LengthB ? NumberA[i] : NumberB[i];
		}
	}
	if (LastNumber)
	{
		R[0] = LastNumber+'0';
		printf("%s\n", R);
	}
	else
		printf("%s\n", R+1);
	system("pause");
}