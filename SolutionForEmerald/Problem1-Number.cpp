#include <stdio.h>
#include <string>
#include <string.h>
#include <algorithm>
#include <set>
#include <stdlib.h>

std::string FindSmallest(char* NumberChar, int Length);
std::string FindSmallestIncludeZero(char* NumberChar, int Length);
std::string FindSmall(char* NumberChar, int Length, int k);
std::string FindLargest(char* NumberChar, int Length);
std::string FindLarge(char* NumberChar, int Length, int k);

int main()
{
	char NumberChar[20] = { 0 };
	int NumberK;
	scanf("%[^,],%d", NumberChar, &NumberK);
	int lens = strlen(NumberChar);
	printf("%s\n", FindSmallest(NumberChar, lens).c_str());
	printf("%s\n", FindLargest(NumberChar, lens).c_str());
	printf("%s\n", FindLarge(NumberChar, lens,50).c_str());
	printf("%s\n", FindSmall(NumberChar, lens, NumberK).c_str());
	system("pause>nul");
}

std::string FindSmallest(char* NumberChar,int Length)
{
	char StrTmp[20];
	strcpy(StrTmp, NumberChar);
	std::sort(StrTmp, StrTmp + Length);
	char* TargetStr = StrTmp;
	for (; *TargetStr == '0'; TargetStr++);
	return std::string(TargetStr);
}

std::string FindSmallestIncludeZero(char* NumberChar, int Length)
{
	char StrTmp[20];
	strcpy(StrTmp, NumberChar);
	std::sort(StrTmp, StrTmp + Length);
	return std::string(StrTmp);
}

std::string FindLargest(char* NumberChar, int Length)
{
	char StrTmp[20];
	strcpy(StrTmp, NumberChar);
	std::sort(StrTmp, StrTmp + Length, [](char& a, char& b){return a > b;});
	return std::string(StrTmp);
}

std::string FindLarge(char* NumberChar, int Length, int k)
{
	std::string NumberStr = FindLargest(NumberChar, Length);
	for (int i=0;i<k-1;i++)
	{
		std::next_permutation((char*)NumberStr.data(), (char*)NumberStr.data() + Length, [](char& a,char& b) {return a > b; });
	}
	return NumberStr;
}

std::string FindSmall(char* NumberChar, int Length, int k)
{
	std::string NumberStr = FindSmallestIncludeZero(NumberChar, Length);
	for (int i = 0; i < k - 1; i++)
	{
		std::next_permutation((char*)NumberStr.data(), (char*)NumberStr.data() + Length);
	}
	//Clear Zero
	char* TargetStr = (char*)NumberStr.c_str();
	for (; *TargetStr == '0'; TargetStr++);
	return std::string(TargetStr);
}