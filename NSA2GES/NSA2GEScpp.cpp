#include <stdio.h>
#include <string>

int main()
{
	char SourceFile[512];
	char TargetFile[512];
	puts("输入文件：");
	scanf("%s", SourceFile);
	puts("输出文件：");
	scanf("%s", TargetFile);
	FILE* FileS = fopen(SourceFile,"r+");
	FILE* FileT = fopen(TargetFile,"w+");
	while (!feof(FileS))
	{
		char Line[10240];
		char Cmd[10240];
		fscanf(FileS, "%[^\n]", Line);
		sscanf(Line, "%s", Cmd);
		std::string CmdStr(Cmd);
		if(CmdStr.length()<=0)
			continue;
		if (CmdStr=="csp")
		{
			sscanf("csp%s", Cmd);
			
		}
	}
}