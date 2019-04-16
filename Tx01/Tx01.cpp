#define  _CRT_SECURE_NO_WARNINGS
#include <stdio.h>

int main()
{
	int n;
	int ai[1024];
	for (int i = 0; i < sizeof(ai)/sizeof(int); i++)
		ai[i] = 0;
	int r=0;
	scanf("%d",&n);
	int tmp;
	for (int i = 0; i < n; i++)
	{
		scanf("%d", &tmp);
		ai[tmp]++;
	}
	for (int i=0;i< sizeof(ai) / sizeof(int);i++)
	{
		if (ai[i] != 0)
		{
			if (ai[i] % i == 0)
				r += ai[i] / i;
			else
			{
				r = -1;
				break;
			}
		}
	}
	printf("%d", r);
}