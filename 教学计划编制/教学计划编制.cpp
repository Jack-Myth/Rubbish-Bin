#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <string>

using std::vector;
using std::string;

struct ClassInfo
{
	string Name;
	int Score=0;
};

struct GraphElement
{
	ClassInfo Data;
	vector<string> To;
	vector<string> From;
};

class Graph
{
	vector<GraphElement> Data;
public:
	void push_back(GraphElement GE);
	void Mixup();
	vector<ClassInfo> FindElementWith0FromNode();
	GraphElement& FindElementByData(string _Data);
	void RemoveElementByName(string _Data,bool ClearRef=true);
	bool IsElementExistByName(string Name);
	int GetElementNumber();
};

void AddClass();
void DeleteClass();
void Process();

Graph ClassGraph;
int main()
{
	while (1)
	{
		int mode;
		system("cls");
		printf("1.添加课程\n2.删除课程\n3.修改课程信息\n4.输出课程排布");
		scanf("%d", &mode);
		switch (mode)
		{
		case 1:
			AddClass();
			break;
		case 2:
			DeleteClass();
			break;
		case 3:
			AddClass();
			break; 
		case  4:
			Process();
			break;
		}
	}
}

void AddClass()
{
	printf("输入课程的名称，学分，和先导课程,课程名称输入0时停止\n");
	while (1)
	{
		char ClassName[10240] = { 0 };
		char TmpPreClass[256];
		scanf("%s", ClassName);
		if (!strcmp(ClassName, "0"))
			break;
		if (ClassGraph.IsElementExistByName(ClassName))
			ClassGraph.RemoveElementByName(ClassName,false);
		GraphElement TmpClass;
		TmpClass.Data.Name = ClassName;
		scanf("%d", &TmpClass.Data.Score);
		//scanf("%[^\n]", ClassName);
		while(getchar() != '\n')
		{
			scanf("%s", TmpPreClass);
			TmpClass.From.push_back(TmpPreClass);
		}
		ClassGraph.push_back(TmpClass);
	}
	ClassGraph.Mixup();
}

void DeleteClass()
{
	printf("输入要删除的课程,与其相关的课程都会断开与这门课的联系:");
	char ClassName[256];
	scanf("%s", ClassName);
	ClassGraph.RemoveElementByName(ClassName);
}

void Process()
{
	vector<vector<ClassInfo>> TermClass;
	Graph TmpClassGraph(ClassGraph);
	for (vector<ClassInfo> ZeroNode = TmpClassGraph.FindElementWith0FromNode(); ZeroNode.size(); ZeroNode = TmpClassGraph.FindElementWith0FromNode())
	{
		TermClass.push_back(vector<ClassInfo>());
		for (auto it = ZeroNode.begin(); it < ZeroNode.end(); ++it)
			TermClass[TermClass.size() - 1].push_back(*it);
		for (auto it = ZeroNode.begin(); it < ZeroNode.end(); ++it)
			TmpClassGraph.RemoveElementByName(it->Name);
	}
	if (TmpClassGraph.GetElementNumber())
	{
		printf("课程图中有回路，无法完成课程分析\n");
		system("pause");
		return;
	}
	for (int i = 0; i < TermClass.size(); i++)
	{
		int TermScore=0;
		for (auto &Info : TermClass[i])
			TermScore += Info.Score;
		printf("第%d学期的课程(共%d学分):\n", i+1,TermScore);
		for (int k = 0; k < TermClass[i].size(); k++)
			printf("%s\t\t%d学分\n", TermClass[i][k].Name.c_str(),TermClass[i][k].Score);
	}
	system("pause");
}

void Graph::push_back(GraphElement GE)
{
	Data.push_back(GE);
}

void Graph::Mixup()
{
	for (auto it = Data.begin(); it < Data.end(); ++it)
		it->To.clear();
	for (auto it = Data.begin(); it < Data.end(); ++it)
		for (auto it2 = Data.begin(); it2 < Data.end(); ++it2)
			if (find(it2->From.begin(), it2->From.end(), it->Data.Name) != it2->From.end())
				it->To.push_back(it2->Data.Name);
}

vector<ClassInfo> Graph::FindElementWith0FromNode()
{
	vector<ClassInfo> ZeroNode;
	for (auto it=Data.begin();it<Data.end();++it)
	{
		if (!it->From.size())
			ZeroNode.push_back(it->Data);
	}
	return ZeroNode;
}

GraphElement & Graph::FindElementByData(string _Data)
{
	for (auto it=Data.begin();it<Data.end();++it)
		if (it->Data.Name==_Data)
			return *it;
}

void Graph::RemoveElementByName(string _Data, bool ClearRef)
{
	for (auto it = Data.begin(); it < Data.end(); ++it)
		if (it->Data.Name == _Data)
		{
			if (ClearRef)
				for (auto & i : Data)
				{
					auto TmpIt = find(i.From.begin(), i.From.end(), _Data);
					if (TmpIt != i.From.end())
						i.From.erase(TmpIt);
				}
			Data.erase(it);
			break;
		}
	Mixup();
}

bool Graph::IsElementExistByName(string Name)
{
	for (auto it = Data.begin(); it < Data.end(); ++it)
		if (it->Data.Name == Name)
			return true;
	return false;
}

int Graph::GetElementNumber()
{
	return Data.size();
}
