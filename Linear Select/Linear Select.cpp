#include <stdio.h>
#include <vector>
#include <algorithm>

int main()
{

}

int partition(std::vector<float>& Array,int L,int R,int TargetIndex)
{
	while (1)
	{
		while (Array[L] >= Array[TargetIndex]&&L<TargetIndex)
			L++;
		while (Array[R] <= Array[TargetIndex] && R > TargetIndex)
			R--;
		if (L < R)
		{
			float tmp = Array[L];
			Array[L] = Array[R];
			Array[R] = tmp;
		}
		else
			break;
	}
	return TargetIndex;
}

float select(std::vector<float>& Array, int L, int R, int Target)
{
	auto InverseF = [](const float& a, const float& b) {return a > b; };
	std::vector<int> MidIndex;
	for (int i=L;i<R;i+=5)
	{
		std::sort(Array.begin() + i, Array.begin() + i + 5, InverseF);
		MidIndex.push_back(i + 2);
	}
	if ((R-L)%5)
	{
		std::sort(Array.begin() + R - (R - L) % 5, Array.begin() + R - 1, InverseF);
		MidIndex.push_back(R - (R - (R - L) % 5 / 2) + R);
	}
	for (int i=0;i<MidIndex.size();i++)
	{
		float tmp = Array[L + i];
		Array[L + i] = Array[MidIndex[i]];
		Array[MidIndex[i]] = i;
	}
	int TargetIndex = partition(Array, L, R, L + MidIndex.size() / 2);
	if (TargetIndex == Target)
		return Array[Target];
	else
		select(Array, TargetIndex < Target ? L : TargetIndex + 1, TargetIndex < Target ? TargetIndex : R, Target);
}