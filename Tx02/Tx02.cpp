#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <math.h>
#include <vector>

//1+2=3
bool IsSuit(std::vector<int>& Num1,std::vector<int>& Num2,std::vector<int>& Num3,int x)
{
	for (int i=0;i<Num1.size();i++)
	{
		if (Num1[i] >= x)
			return false;
	}
	for (int i = 0; i < Num2.size(); i++)
	{
		if (Num2[i] >= x)
			return false;
	}
	for (int i = 0; i < Num3.size(); i++)
	{
		if (Num3[i] >= x)
			return false;
	}
	//Convert to 10
	int NumA = 0, NumB = 0, NumC = 0;
	for (int i=Num1.size()-1;i>=0;i--)
		NumA += Num1[i] * (int)pow(x, Num1.size() - i - 1);
	for (int i = Num2.size() - 1; i >= 0; i--)
		NumB += Num2[i] * (int)pow(x, Num2.size() - i - 1);
	for (int i = Num3.size() - 1; i >= 0; i--)
		NumC += Num3[i] * (int)pow(x, Num3.size() - i - 1);
	return NumA + NumB == NumC;
}

void ConvertTemp(const char* InputStr,std::vector<int>& ConvertResult)
{
	ConvertResult.clear();
	while (*InputStr)
	{
		const char& CurrentChar = *InputStr;
		if (CurrentChar >='A'&& CurrentChar <='Z')
		{
			ConvertResult.push_back(CurrentChar - 'A' + 10);
		}
		else if (CurrentChar >= 'a' && CurrentChar <= 'z')
		{
			ConvertResult.push_back(CurrentChar - 'a' + 10);
		}
		else if (CurrentChar >= '0' && CurrentChar <= '9')
		{
			ConvertResult.push_back(CurrentChar - '0');
		}
		//else 
		InputStr++;
	}
}

int main()
{
	char Number1[128];
	char Number2[128];
	char Number3[128];
	scanf("%[^+]+%[^=]=%[^\n]", Number1, Number2, Number3);
	std::vector<int> A, B, C;
	ConvertTemp(Number1, A);
	ConvertTemp(Number2, B);
	ConvertTemp(Number3, C);
	char IsSuitArray[21] = { 0 };
	int SuitCount = 0;
	for (int i = 2; i <= 20; i++)
	{
		if (IsSuit(A, B, C, i))
		{
			IsSuitArray[i] = 1;
			SuitCount++;
		}
	}
	printf("%d\n", SuitCount);
	for (int i=0;i<sizeof(IsSuitArray)/sizeof(char);i++)
	{
		if (IsSuitArray[i])
			printf("%d\n", i);
	}
	return 0;
}