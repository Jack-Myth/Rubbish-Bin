#include<stdio.h>
#include<iostream>
//import std.core;

int main()
{
	int m = 0;
	scanf("%d", &m);
	int k = 0;
	while (k < m)
	{
		printf("Fibonacci[%d]:1\n", k);
		k++;
	}
	if (m > 2)
	{
		int a = 1, b = 1;
		while (int i = 3; i <= m)
		{
			int k = a + b;
			b = a;
			a = k;
			i++;
			std::cout << "Fibonacci[" << i << "]:" << k << std::endl;
		}
	}
	//system("pause");
}