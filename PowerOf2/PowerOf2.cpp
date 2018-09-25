#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include "MyBigInt.h"

int main()
{
	long long PowerValule;
	MyBigInt BigNumber;
	while (1)
	{
		printf("Please Input the power of 2:");
		scanf("%lld", &PowerValule);
		BigNumber = 1;
		clock_t PreClock = clock();
		for (; PowerValule > 0; PowerValule--)
			BigNumber += BigNumber;
		printf("Result:%s\nin %lfs\n", BigNumber.ToString().c_str(), (double)(clock()- PreClock) / (double)1000);
	}
}