#include <stdio.h>

class A
{
public:
	int a;
};

class B :public A
{
public:
	int aa;
	virtual void VFunc() {};
};

class I
{
public:
	int ii;
	
	virtual void VFunc2() {};
	virtual void VFunc3() {};
	virtual void VFunc4() {};
	virtual void VFunc5() {};
};

class C :public A, public I
{
public:
	int cc;
};

int main()
{
	B* b = new B();
	A* a = (A*)b;
	printf("%d  %d", sizeof(I),sizeof(B));
}