#include <stdio.h>

int main()
{
	char Number[10240];
	printf("Input a Number:");
	scanf("%s", Number);
	int k = 0;
	for (char* pN = Number; k++,*pN; pN++);
	int Ne = 0;
	Number[k - 2]++;
	for (int i = k - 2; i >= 0; i--)
	{
		Number[i] += Ne;
		Ne = 0;
		if (Number[i] > '9')
		{
			Number[i] -= 10;
			Ne = 1;
		}
	}
	if (Ne)
		printf("%d",Ne);
	printf("%s", Number);
	while (1);
}