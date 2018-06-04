#include <stdio.h>
#include <stdlib.h>
#include "ChinesC.h"

整型 主函数()
{
	整型 第一个变量 = 0;
	整型 第二个变量 = 0;
	格式化输入("%d %d", &第一个变量, &第二个变量);
	格式化输出("%d", 第一个变量 + 第二个变量);
	命令行(暂停);
	返回 0;
}