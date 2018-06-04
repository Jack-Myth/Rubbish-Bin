#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <string.h>
#include <stack>
#include <string>
#include <stdlib.h>

using std::stack;
using std::string;
char OperatorPriority(char Operator);
bool IsCharInCollection(char CharToTest, std::initializer_list<char> CharsList);
void InfixToPrefix(char* InfixExpression, char** PrefixExpression);
void InfixToPostfix(char* InfixExpression, char** PostfixExpression);
float CaculatePrefixExpression(char* PrefixExpression);
float CaculatePostfixExpression(char* PrefixExpression);
void AddCharToString(char* Dest, char CharToAdd);
char* gcvtTrim(char* StrToTrim);

int main()
{
	int mode;
	while (1)
	{
		printf("选择模式:\n1.中缀表达式转前缀表达式并求值\n2.中缀表达式转后缀表达式并求值\n3.前缀表达式求值\n4.后缀表达式求值.\n");
		scanf("%d%*c", &mode);
		char TargetExpression[1024];
		switch (mode)
		{
		case 1:
			printf("请输入要转换的中缀表达式:");
			scanf("%[^\n]%*c", TargetExpression);
			char* PrefixExpression;
			InfixToPrefix(TargetExpression, &PrefixExpression);
			printf("前缀表达式:%s=%f\n", PrefixExpression,CaculatePrefixExpression(PrefixExpression));
			delete PrefixExpression;
			break;
		case 2:
			printf("请输入要转换的中缀表达式:");
			scanf("%[^\n]%*c", TargetExpression);
			char* PostfixExpression;
			InfixToPostfix(TargetExpression, &PostfixExpression);
			printf("后缀表达式:%s=%f\n", PostfixExpression, CaculatePostfixExpression(PostfixExpression));
			delete PostfixExpression;
			break;
		case 3:
			printf("请输入要计算的前缀表达式:\n");
			scanf("%[^\n]%*c", TargetExpression);
			printf("=%f\n", CaculatePrefixExpression(TargetExpression));
			break;
		case 4:
			printf("请输入要计算的后缀表达式:\n");
			scanf("%[^\n]%*c", TargetExpression);
			printf("=%f\n", CaculatePostfixExpression(TargetExpression));
			break;
		}
		system("pause");
		system("cls");
	}
	return 0;
}

void InfixToPrefix(char* InfixExpression, char** PrefixExpression)
{
	stack<char> OperatorStack;
	stack<string> ExpressionTmp;
	*PrefixExpression = new char[1024];
	memset(*PrefixExpression, 0, 1024);
	char* InfixIndex = InfixExpression + strlen(InfixExpression);
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
			ExpressionTmp.push(string(gcvtTrim(_gcvt(Number, 6, NumberChars))));
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

void InfixToPostfix(char* InfixExpression, char** PostfixExpression)
{
	struct DoubleFlipData
	{
		bool IsOperator;
		float Number;
		char Operator;
	};
	stack<DoubleFlipData> ExpressionTmp;
	stack<char> OperatorStack;
	*PostfixExpression = new char[1024];
	memset(*PostfixExpression, 0, 1024);
	char* InfixIndex = InfixExpression + strlen(InfixExpression);
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
			ExpressionTmp.push({ true,0,*InfixIndex });
		}
		else
		{
			float Number;
			sscanf(InfixIndex, "%f", &Number);
			ExpressionTmp.push({ false,Number,0 });
		}
	}
	while(!ExpressionTmp.empty())
	{
		if (ExpressionTmp.top().IsOperator)
		{
			switch (ExpressionTmp.top().Operator)
			{
			case ')':
				while (OperatorStack.top() != '(')
				{
					AddCharToString(*PostfixExpression, OperatorStack.top());
					OperatorStack.pop();
				}
				OperatorStack.pop();
				break;
			case '(':
				OperatorStack.push(ExpressionTmp.top().Operator);
				break;
			default:
				while (!OperatorStack.empty() && OperatorPriority(OperatorStack.top()) >= OperatorPriority(ExpressionTmp.top().Operator))
				{
					AddCharToString(*PostfixExpression, OperatorStack.top());
					OperatorStack.pop();
				}
				OperatorStack.push(ExpressionTmp.top().Operator);
			}
		}
		else
		{
			char NumberChars[16];
			strcat(*PostfixExpression,gcvtTrim(_gcvt(ExpressionTmp.top().Number, 6, NumberChars)));
		}
		ExpressionTmp.pop();
	}
	while (!OperatorStack.empty())
	{
		AddCharToString(*PostfixExpression, OperatorStack.top());
		OperatorStack.pop();
	}
}

float CaculatePrefixExpression(char* PrefixExpression)
{
	stack<float> NumberStack;
	char* PrefixIndex = PrefixExpression + strlen(PrefixExpression);
	while (PrefixIndex >= PrefixExpression)
	{
		while ((PrefixIndex--), PrefixIndex >= PrefixExpression && *PrefixIndex == ' ');
		PrefixIndex++;
		char *PrefixIndexTmp = PrefixIndex;
		while ((PrefixIndex--), PrefixIndex >= PrefixExpression && !IsCharInCollection(*PrefixIndex, { '+','-','*','/','%','(',')' ,' ' }));
		if (PrefixIndexTmp - ++PrefixIndex == 0)
		{
			if (PrefixIndex <= PrefixExpression)       //中缀表达式操作符不会出现在表达式最前方
				break;
			PrefixIndex--;
			float NumberA = NumberStack.top();
			NumberStack.pop();
			float NumberB = NumberStack.top();
			NumberStack.pop();
			switch (*PrefixIndex)
			{
			case '+':
				NumberStack.push(NumberA + NumberB);
				break;
			case '-':
				NumberStack.push(NumberA - NumberB);
				break;
			case '*':
				NumberStack.push(NumberA * NumberB);
				break;
			case '/':
				NumberStack.push(NumberA / NumberB);
				break;
			case '%':
				NumberStack.push((int)NumberA % (int)NumberB);
				break;
			}
		}
		else
		{
			float Number;
			sscanf(PrefixIndex, "%f", &Number);
			NumberStack.push(Number);
		}
	}
	return NumberStack.top();
	return 0;
}

float CaculatePostfixExpression(char* PostfixExpression)
{
	char* PostfixExpressionIndex= PostfixExpression;
	stack<float> NumberStack;
	char TmpChar[16];
	do 
	{
		sscanf(PostfixExpressionIndex, "%s", TmpChar);
		while (*PostfixExpressionIndex == ' ')
			PostfixExpressionIndex++;
		if (IsCharInCollection(*TmpChar, { '+','-','*','/','%' }))
		{
			float NumberB = NumberStack.top();
			NumberStack.pop();
			float NumberA = NumberStack.top();
			NumberStack.pop();
			switch (*TmpChar)
			{
			case '+':
				NumberStack.push(NumberA + NumberB);
				break;
			case '-':
				NumberStack.push(NumberA - NumberB);
				break;
			case '*':
				NumberStack.push(NumberA * NumberB);
				break;
			case '/':
				NumberStack.push(NumberA / NumberB);
				break;
			case '%':
				NumberStack.push((int)NumberA % (int)NumberB);
				break;
			}
			PostfixExpressionIndex++;
		}
		else
		{
			{
				char* TmpCharIndex=TmpChar;
				int TmpCharLen = strlen(TmpChar);
				while (!IsCharInCollection(*TmpCharIndex, { '+','-','*','/','%' }) && TmpCharLen > 0)
					TmpCharIndex++,TmpCharLen--;
				*TmpCharIndex = 0;
			}
			NumberStack.push(atof(TmpChar));
			PostfixExpressionIndex += strlen(TmpChar);
		}
	} while (PostfixExpressionIndex<PostfixExpression+strlen(PostfixExpression));
	return NumberStack.top();
}

char OperatorPriority(char Operator)
{
	if (IsCharInCollection(Operator, { '(',')' }))
		return 0;
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

void AddCharToString(char* Dest, char CharToAdd)
{
	char Tmp[2];
	Tmp[0] = CharToAdd;
	Tmp[1] = 0;
	strcat(Dest, Tmp);
}

char* gcvtTrim(char* StrToTrim)
{
	if (StrToTrim[strlen(StrToTrim) - 1] == '.')
		StrToTrim[strlen(StrToTrim) - 1] = ' ';
	else
	{
		StrToTrim[strlen(StrToTrim)+1] = 0;
		StrToTrim[strlen(StrToTrim)] = ' ';
	}
	return StrToTrim;
}