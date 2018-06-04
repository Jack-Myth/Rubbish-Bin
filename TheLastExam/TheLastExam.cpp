#include <stdio.h>
#include <vector>
#include <utility>
#include <string>
#include <time.h>
#include <algorithm>

using std::vector;
vector<std::pair<std::string, int>> SubjectName({ {"语文",0},{"数学",0 },{"英语",0 },{"物理",0 },{"化学",0 },{"生物",0 },{"政治",0 },{"地理",0 },{"历史",0 },{"大计基",0 } });

struct Strudent
{
	long Number;
	std::string  Name;
	int Class = 0;
	std::pair<int, int> Score[3];
};

vector<Strudent> ClassInstance[10];


void Func0();
void Func1();
void Func2();
void Func3();
void Func4();
void Func5();
void Func6();
void Func7();
void Func8();

int main()
{
	srand(time(NULL));
	for (int c = 0; c < 10; c++)
	{
		ClassInstance[c].resize(rand() % 20 + 20);
		ClassInstance[c][0].Score[0].first = rand() % 10;
		SubjectName[ClassInstance[c][0].Score[0].first].second++;
		ClassInstance[c][0].Score[0].first = rand() % 10;
		do
		{
			ClassInstance[c][0].Score[1].first = rand() % 10;
		} while (ClassInstance[c][0].Score[1].first == ClassInstance[c][0].Score[0].first);
		SubjectName[ClassInstance[c][0].Score[1].first].second++;
		do
		{
			ClassInstance[c][0].Score[2].first = rand() % 10;
		} while (ClassInstance[c][0].Score[1].first == ClassInstance[c][0].Score[2].first || ClassInstance[c][0].Score[2].first == ClassInstance[c][0].Score[0].first);
		SubjectName[ClassInstance[c][0].Score[2].first].second++;
		for (int i = 0; i < ClassInstance[c].size(); i++)
		{
			ClassInstance[c][i].Class = c;
			ClassInstance[c][i].Number = 2016060000 + c * 100 + i;
			ClassInstance[c][i].Score[0].first = ClassInstance[c][0].Score[0].first;
			ClassInstance[c][i].Score[0].second = rand() % 100;
			ClassInstance[c][i].Score[1].first = ClassInstance[c][0].Score[1].first;
			ClassInstance[c][i].Score[1].second = rand() % 100;
			ClassInstance[c][i].Score[2].first = ClassInstance[c][0].Score[2].first;
			ClassInstance[c][i].Score[2].second = rand() % 100;
		}
	}
	vector<std::pair<std::string, int>> SubjectCopy(SubjectName);
	std::sort(SubjectCopy.begin(), SubjectCopy.end(), [](const std::pair<std::string, int> a, const std::pair<std::string, int> b) {return a.second > b.second; });
	printf("考试顺序:\n");
	for (auto it = SubjectCopy.begin(); it < SubjectCopy.end(); ++it)
		printf("%s->", it->first.c_str());
	system("pause");
	system("cls");
	int mod;
	while (1)
	{
		printf("1.输出所有（班级）\n2.输出单个（班级）\n3.输出所有（学生）\n4.统计分数段（所有）\n5.统计分数段（按班级）\n6.统计分数段(按科目)\n7.查找学生(按班级)\n8.查找学生(按分数)\n9.查找学生（按学号）:");
		scanf("%d", &mod);
		switch (mod - 1)
		{
		case 0:
			Func0();
			break;
		case 1:
			Func1();
			break;
		case 2:
			Func2();
			break;
		case 3:
			Func3();
			break;
		case 4:
			Func4();
			break;
		case 5:
			Func5();
			break;
		case 6:
			Func6();
			break;
		case 7:
			Func7();
			break;
		case 8:
			Func8();
		}
		printf("\n");
		system("pause");
		system("cls");
	}
	return 0;
}

void Func0()
{
	for (int i = 0; i < 10; i++)
	{
		printf("%d班:\n", i + 1);
		for (auto it = ClassInstance[i].begin(); it < ClassInstance[i].end(); ++it)
			printf("%ld:\n%s%d\n%s%d\n%s%d\n", it->Number, SubjectName[it->Score[0].first].first.c_str(), it->Score[0].second, SubjectName[it->Score[1].first].first.c_str(), it->Score[1].second, SubjectName[it->Score[2].first].first.c_str(), it->Score[2].second);
	}
}

void Func1()
{
	int i = 0;
	printf("输入要查询的班级:");
	scanf("%d", &i);
	printf("%d班:\n", i);
	for (auto it = ClassInstance[i - 1].begin(); it < ClassInstance[i - 1].end(); ++it)
		printf("%ld:\n%s%d\n%s%d\n%s%d\n", it->Number, SubjectName[it->Score[0].first].first.c_str(), it->Score[0].second, SubjectName[it->Score[1].first].first.c_str(), it->Score[1].second, SubjectName[it->Score[2].first].first.c_str(), it->Score[2].second);
}

void Func2()
{
	for (int i = 0; i < 10; i++)
		for (auto it = ClassInstance[i].begin(); it < ClassInstance[i].end(); ++it)
			printf("%ld:\n%s%d\n%s%d\n%s%d\n", it->Number, SubjectName[it->Score[0].first].first.c_str(), it->Score[0].second, SubjectName[it->Score[1].first].first.c_str(), it->Score[1].second, SubjectName[it->Score[2].first].first.c_str(), it->Score[2].second);
}

void Func3()
{
	for (int i = 0; i < 10; i++)
	{
		printf("%d班：\n", i + 1);
		for (int s = 0; s < 10; s++)
		{
			printf("%s:\n", SubjectName[s].first.c_str());
			vector<std::pair<long, int>> ScoreArray[3];
			for (auto it = ClassInstance[i].begin(); it < ClassInstance[i].end(); ++it)
			{
				for (int ss = 0; ss < 3; ss++)
				{
					if (it->Score[ss].first == s)
						if (it->Score[ss].second >= 90)
							ScoreArray[0].push_back({ it->Number ,it->Score[ss].second });
						else if (it->Score[ss].second >= 60)
							ScoreArray[1].push_back({ it->Number ,it->Score[ss].second });
						else
							ScoreArray[2].push_back({ it->Number ,it->Score[ss].second });
				}
			}
			printf(">=90:\n");
			for (auto it = ScoreArray[0].begin(); it < ScoreArray[0].end(); ++it)
				printf("%ld:%d\n", it->first, it->second);
			printf(">=60:\n");
			for (auto it = ScoreArray[1].begin(); it < ScoreArray[1].end(); ++it)
				printf("%ld:%d\n", it->first, it->second);
			printf("<60:\n");
			if (ScoreArray[1].size())
				for (auto it = ScoreArray[2].begin(); it < ScoreArray[2].end(); ++it)
					printf("%ld:%d\n", it->first, it->second);
		}
	}
}

void Func4()
{
	int i = 0;
	printf("输入要查询的班级:");
	scanf("%d", &i);
	printf("%d班：\n", i);
	i--;
	for (int s = 0; s < 10; s++)
	{
		printf("%s:\n", SubjectName[s].first.c_str());
		vector<std::pair<long, int>> ScoreArray[3];
		for (auto it = ClassInstance[i].begin(); it < ClassInstance[i].end(); ++it)
		{
			for (int ss = 0; ss < 3; ss++)
			{
				if (it->Score[ss].first == s)
					if (it->Score[ss].second >= 90)
						ScoreArray[0].push_back({ it->Number ,it->Score[ss].second });
					else if (it->Score[ss].second >= 60)
						ScoreArray[1].push_back({ it->Number ,it->Score[ss].second });
					else
						ScoreArray[2].push_back({ it->Number ,it->Score[ss].second });
			}
		}
		printf(">=90:\n");
		if (ScoreArray[0].size())
			for (auto it = ScoreArray[0].begin(); it < ScoreArray[0].end(); ++it)
				printf("%ld:%d\n", it->first, it->second);
		printf(">=60:\n");
		if (ScoreArray[1].size())
			for (auto it = ScoreArray[1].begin(); it < ScoreArray[1].end(); ++it)
				printf("%ld:%d\n", it->first, it->second);
		printf("<60:\n");
		if (ScoreArray[2].size())
			for (auto it = ScoreArray[2].begin(); it < ScoreArray[2].end(); ++it)
				printf("%ld:%d\n", it->first, it->second);
	}
}

void Func5()
{
	int s;
	char Subj[20];
	printf("输入要统计的学科：");
	scanf("%s", Subj);
	for (auto it = SubjectName.begin(); it < SubjectName.end(); ++it)
	{
		if (it->first == Subj)
		{
			s = it - SubjectName.begin();
			break;
		}
	}
	for (int i = 0; i < 10; i++)
	{
		printf("%d班：\n", i + 1);
		printf("%s:\n", SubjectName[s].first.c_str());
		vector<std::pair<long, int>> ScoreArray[3];
		for (auto it = ClassInstance[i].begin(); it < ClassInstance[i].end(); ++it)
		{
			for (int ss = 0; ss < 3; ss++)
			{
				if (it->Score[ss].first == s)
					if (it->Score[ss].second >= 90)
						ScoreArray[0].push_back({ it->Number ,it->Score[ss].second });
					else if (it->Score[ss].second >= 60)
						ScoreArray[1].push_back({ it->Number ,it->Score[ss].second });
					else
						ScoreArray[2].push_back({ it->Number ,it->Score[ss].second });
			}
		}
		printf(">=90:\n");
		if (ScoreArray[0].size())
			for (auto it = ScoreArray[0].begin(); it < ScoreArray[0].end(); ++it)
				printf("%ld:%d\n", it->first, it->second);
		printf(">=60:\n");
		if (ScoreArray[1].size())
			for (auto it = ScoreArray[1].begin(); it < ScoreArray[1].end(); ++it)
				printf("%ld:%d\n", it->first, it->second);
		printf("<60:\n");
		if (ScoreArray[2].size())
			for (auto it = ScoreArray[2].begin(); it < ScoreArray[2].end(); ++it)
				printf("%ld:%d\n", it->first, it->second);
	}
}

void Func6()
{
	int i = 0;
	printf("输入要查询的班级:");
	scanf("%d", &i);
	printf("%d班:\n", i);
	for (auto it = ClassInstance[i - 1].begin(); it < ClassInstance[i - 1].end(); ++it)
		printf("%ld\n", it->Number);
}

void Func7()
{
	char aa[10];
	int mod = 0;
	printf("输入查询的分数段:");
	scanf("%s", aa);
	if (!strcmp(aa, ">=90"))
		mod = 0;
	else if (!strcmp(aa, ">=60"))
		mod = 1;
	else if (!strcmp(aa, "<60"))
		mod = 2;
	for (int i = 0; i < 10; i++)
	{
		for (auto it = ClassInstance[i].begin(); it < ClassInstance[i].end(); ++it)
			for (int k = 0; k < 3; k++)
				switch (mod)
				{
				case 0:
					if (it->Score[k].second >= 90)
					{
						printf("%ld\n", it->Number);
						k = 9;
					}
					break;
				case 1:
					if (it->Score[k].second >= 60)
					{
						printf("%ld\n", it->Number);
						k = 9;
					}
					break;
				case 2:
					if (it->Score[k].second < 60)
					{
						printf("%ld\n", it->Number);
						k = 9;
					}
					break;
				}
	}
}

void Func8()
{
	long s = 0;
	printf("输入要查询的学号:");
	scanf("%ld", &s);
	for (int i = 0; i < 10; i++)
		for (auto it = ClassInstance[i].begin(); it < ClassInstance[i].end(); ++it)
			if (it->Number == s)
			{
				printf("%ld:%d班\n", s, i);
				return;
			}
}