#pragma once
#include <vector>

class MyBigInt
{
public:
	std::vector<char> NumberBuffer;
	MyBigInt operator+(const MyBigInt& AddBigInt);
	MyBigInt& operator=(const MyBigInt& TBigInt);
	MyBigInt& operator=(const long long& InitInt);
	MyBigInt& operator+=(const MyBigInt& AddBigInt);
	std::string ToString();
	inline long long GetNumberLength() const 
	{
		return NumberBuffer.size();
	}
};