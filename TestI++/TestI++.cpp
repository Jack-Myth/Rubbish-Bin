#include <stdio.h>
#include <vector>
#include <algorithm>

bool compaire(int& a, int& b)
{
	return a < b;
}

int main()
{
	std::vector<int> a;
	std::sort(a.begin(), a.end(), compaire);
	std::sort(a.begin(), a.end(), [](int& a, int& b) {return a < b; });
	auto tmpfunc = [](int& a, int& b)
	{
		return a < b;
	};

	[]() {}();

	int Max;
	for (int i = 0; i < a.size(); i++)
	{
		[&Max](int& a, int& b)
		{
			if (a > b)
			{
				if (a > Max)
					return a;
				return Max;
			}
		}(a[0], a[i]);
	}
}