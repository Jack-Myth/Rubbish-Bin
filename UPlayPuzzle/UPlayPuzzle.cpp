#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <vector>

int main()
{
	char C[100];
	int k[100];
	scanf("%s", C);
	for (int a = 0;; a++)
	{
		scanf("%d", &k[a]);
		if (k[a] == 0)
			break;
	}
	std::vector<char> mm;
	for (int a = 1; a<=44; a++)
		for (int i=1;i<=44;i++)
		{
			if (a==k[i-1])
			{
				mm.push_back(C[i - 1]);
				break;
			}
		}
	for (int a = 0;; a++)
	{
		if (k[a] != 0)
		{
			printf("%c", mm[a]);
		}
		else
		{
			break;
		}
	}
	printf("\n");
	system("pause");
}