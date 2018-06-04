#define _CRT_SECURE_NO_WARNINGS
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argv,char* argc[])
{
	if (argv <1)
		exit(-1);
	char pathx[10240] = "explorer shell:AppsFolder\\";
	strcat(pathx, argc[1]);
	system(pathx);
	system("pause");
}
//Microsoft.SpartaUWP.886d4fbfa1_8wekyb3d8bbwe!GearGameShippingPublic