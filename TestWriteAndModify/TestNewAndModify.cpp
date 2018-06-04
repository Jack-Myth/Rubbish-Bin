#include <stdio.h>
#include <chrono>
#include <stdlib.h>

int main()
{
	printf("Input Test File Size(MB).");
	float tmpSize;
	scanf("%f", &tmpSize);
	long long Size= tmpSize * (long long)1024*( long long)1024;
	char Buffer[102400] = { 'a' };
	char EmptyBuffer[102400] = { "0" };
	FILE* TestFile = fopen("TestFile.tf", "wb+");
	auto LastTime = std::chrono::steady_clock::now();
	for (long long i=0;i<Size/ 102400;i++)
	{
		fwrite(EmptyBuffer, 102400, 1, TestFile);
	}
	fclose(TestFile);
	long long Time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - LastTime).count();
	TestFile= fopen("TestFile.tf", "rb+");
	printf("Create New Use:%lldms\n", Time);
	fseek(TestFile, 0, SEEK_SET);
	LastTime = std::chrono::steady_clock::now();
	for (long long i = 0; i < Size / 102400; i++)
	{
		fwrite(Buffer, 102400, 1, TestFile);
	}
	Time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::steady_clock::now() - LastTime).count();
	fclose(TestFile);
	printf("Write Exist Use:%lldms\n", Time);
	system("pause");
	A c;
	c.func();
}

void A::func()
{

}