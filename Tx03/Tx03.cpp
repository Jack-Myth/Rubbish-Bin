#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <vector>

class IIIII
{
	bool RedLight = false;
	float ti = 0;
	float timeRemain = 0;
public:
	IIIII(int& _ti) :ti(_ti), timeRemain(_ti) {};
	void Exec(float Time)
	{
		timeRemain -= Time;
		while (timeRemain<=0)
		{
			RedLight = !RedLight;
			timeRemain += ti;
		}
	}
	inline bool IsRedLight()
	{
		return RedLight;
	}

	inline float WaitToGreenLight()
	{
		return RedLight ? timeRemain : 0;
	}
};

int main()
{
	int n;
	scanf("%d", &n);
	float v;
	std::vector<IIIII*> IIIIIs;
	for (int i = 0; i < n; i++)
	{
		int tmpTi;
		scanf("%d", &tmpTi);
		IIIIIs.push_back(new IIIII(tmpTi));
	}
	scanf("%f", &v);
	float TotalTime = 0;
	for (int i=0;i<n;i++)
	{
		float CurrentUsedTime = 0;
		//Original to the door.
		CurrentUsedTime += 150 / v;
		IIIIIs[i]->Exec(CurrentUsedTime);
		CurrentUsedTime += IIIIIs[i]->WaitToGreenLight();
		for (int a=i+1;a<n;a++)
		{
			IIIIIs[a]->Exec(CurrentUsedTime);
		}
		TotalTime += CurrentUsedTime;
	}
	TotalTime += 150 / v;
	printf("%d", (int)TotalTime);
	return 0;
}