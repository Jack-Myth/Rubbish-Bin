#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <vector>

class RoundQueue
{
	std::vector<int> Buffer;

	int Index=0;
	int Length=0;

public:
	RoundQueue(int Size)
	{
		Buffer.resize(Size);
	}

	bool IsEmpty()
	{
		return !Length;
	}

	bool Push(int Number)
	{
		if (Length>=Buffer.size())
			return false;
		Buffer[(Index+Length)%Buffer.size()] = Number;
		Length++;
		return true;
	}
	
	bool Pop(int& Result)
	{
		if (Length <= 0)
			return false;
		Result = Buffer[Index];
		Index=(Index+1)%Buffer.size();
		Length--;
		return true;
	}
};

int main()
{
	RoundQueue* rq = new RoundQueue(5);
	int n;
	while (true)
	{
		scanf("%d", &n);
		if (n==0)
			break;
		for (int i = 0; i < n; i++)
		{
			int x = 0;
			scanf("%d", &x);
			printf("%d,%s\n", x, rq->Push(x) ? "True" : "False");
		}
		scanf("%d", &n);
		for (int i = 0; i < n; i++)
		{
			int x;
			bool su = rq->Pop(x);
			if (su)
				printf("%s,%d\n", "True", x);
			else
				printf("False\n");
		}
		printf("%s\n", rq->IsEmpty() ? "True" : "False");
	}
}