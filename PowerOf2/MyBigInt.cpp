#include "MyBigInt.h"
#include <stack>

MyBigInt MyBigInt::operator+(const MyBigInt& AddBigInt)
{
	MyBigInt tmpResult;
	std::vector<char>& tmpNumberBuffer=tmpResult.NumberBuffer;
	const std::vector<char>& LonggerBuffer = AddBigInt.GetNumberLength() > GetNumberLength() ? AddBigInt.NumberBuffer : NumberBuffer;
	const std::vector<char>& ShorterBuffer = AddBigInt.GetNumberLength() < GetNumberLength() ? AddBigInt.NumberBuffer : NumberBuffer;
	tmpNumberBuffer.resize(LonggerBuffer.size() + 1,0);
	size_t Distance = LonggerBuffer.size() - ShorterBuffer.size();
	for (size_t i=LonggerBuffer.size()-1;i-Distance>=0&&i+1>i;i--)
	{
		int tmpDigit = LonggerBuffer[i] + ShorterBuffer[i - Distance];
		if (tmpDigit >= 10)
		{
			tmpDigit -= 10;
			tmpNumberBuffer[i] = 1;
		}
		tmpNumberBuffer[i+1] += (char)tmpDigit;
	}
	for (Distance--; Distance + 1 > Distance; Distance--)
	{
		tmpNumberBuffer[Distance + 1] += LonggerBuffer[Distance];
		if (tmpNumberBuffer[Distance + 1]>=10)
		{
			tmpNumberBuffer[Distance + 1] -= 10;
			tmpNumberBuffer[Distance] = 1;
		}
	}
	if (!tmpNumberBuffer[0])
		tmpNumberBuffer.erase(tmpNumberBuffer.begin());
	return tmpResult;
}

MyBigInt& MyBigInt::operator+=(const MyBigInt& AddBigInt)
{
	(*this) = (*this) + AddBigInt;
	return *this;
}
std::string MyBigInt::ToString()
{
	std::string strBuf;
	strBuf.resize(NumberBuffer.size() + 1);
	for (size_t i=0;i<NumberBuffer.size();i++)
		strBuf[i] = NumberBuffer[i] + '0';
	return strBuf;
}

MyBigInt& MyBigInt::operator=(const long long& InitInt)
{
	if (InitInt<0)
		printf("Currently Big Number Add doesn't support Negative Number!\nUse Positive Number instead.\n");
	NumberBuffer.clear();
	long long TargetInt= abs(InitInt);
	std::stack<char> tmpNumberStack;
	do 
	{
		char ModResult = TargetInt % 10;
		tmpNumberStack.push(ModResult);
		TargetInt -= ModResult;
	} while (TargetInt);
	NumberBuffer.reserve(tmpNumberStack.size());
	while (!tmpNumberStack.empty())
	{
		NumberBuffer.push_back(tmpNumberStack.top());
		tmpNumberStack.pop();
	}
	return *this;
}

MyBigInt& MyBigInt::operator=(const MyBigInt& TBigInt)
{
	NumberBuffer = TBigInt.NumberBuffer;
	return *this;
}
