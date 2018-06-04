#include <stdio.h>
#include <string.h>
#include <stack>
#include <string>

using std::stack;
using std::string;
char OperatorPriority(char Operator);
bool IsCharInCollection(char CharToTest, std::initializer_list<char> CharsList);
void InfixToPrefix(char* InfixExpression, char** PrefixExpression);

int main()
{
	int mode;
	printf("选择模式:\n1.中缀表达式转前缀表达式并求值\n2.中缀表达式转后缀表达式并求值\n3.前缀表达式求值\n4.后缀表达式求值.");
	scanf("%d", &mode);
	char TargetExpression[1024];
	switch (mode)
	{
	case 1:
		printf("请输入要转换的中缀表达式:");
		scanf("%[^\n]", TargetExpression);
		char* PrefixExpression;
		InfixToPrefix(TargetExpression, &PrefixExpression);
		printf("%s", PrefixExpression);
		delete PrefixExpression;
		break;
	}
	return 0;
}

void InfixToPrefix(char* InfixExpression, char** PrefixExpression)
{
	stack<char> OperatorStack;
	stack<string> ExpressionTmp;
	*PrefixExpression = new char[1024];
	memset(*PrefixExpression, 0, 1024);
	char* InfixIndex = InfixExpression + strlen(InfixExpression) + 1;
	while (InfixIndex >= InfixExpression)
	{
		while ((InfixIndex--), InfixIndex >= InfixExpression && *InfixIndex == ' ');
		InfixIndex++;
		char *InfixIndexTmp = InfixIndex;
		while ((InfixIndex--), InfixIndex >= InfixExpression && !IsCharInCollection(*InfixIndex, { '+','-','*','/','%','(',')' ,' ' }));
		if (InfixIndexTmp - ++InfixIndex == 0)
		{
			if (InfixIndex <= InfixExpression)       //中缀表达式操作符不会出现在表达式最前方
				break;
			InfixIndex--;
			switch (*InfixIndex)
			{
			case '(':
				while (OperatorStack.top() != ')')
				{
					ExpressionTmp.push(string(1, OperatorStack.top()));
					OperatorStack.pop();
				}
				OperatorStack.pop();
				break;
			case ')':
				OperatorStack.push(*InfixIndex);
				break;
			default:
				while (!OperatorStack.empty() && OperatorPriority(OperatorStack.top()) > OperatorPriority(*InfixIndex))
				{
					ExpressionTmp.push(string(1, OperatorStack.top()));
					OperatorStack.pop();
				}
				OperatorStack.push(*InfixIndex);
			}
		}
		else
		{
			float Number;
			char NumberChars[16];
			sscanf(InfixIndex, "%f", &Number);
			ExpressionTmp.push(_gcvt(Number, 0, NumberChars));
		}
	}
	while (!OperatorStack.empty())
	{
		ExpressionTmp.push(string(1, OperatorStack.top()));
		OperatorStack.pop();
	}
	while (!ExpressionTmp.empty())
	{
		strcat(*PrefixExpression, ExpressionTmp.top().c_str());
		ExpressionTmp.pop();
	}
}

char OperatorPriority(char Operator)
{
	if (IsCharInCollection(Operator, { '+','-' }))
		return 1;
	else
		return 2;
}

bool IsCharInCollection(char CharToTest, std::initializer_list<char> CharsList)
{
	for (auto it = CharsList.begin(); it < CharsList.end(); ++it)
	{
		if (CharToTest == *it)
			return true;
	}
	return false;
}