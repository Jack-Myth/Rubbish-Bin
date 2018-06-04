#include <iostream>
#include <stdio.h>
#include <vector>
#include <list>
#include <utility>

struct Graph
{
	int ID;
	int Data;
	std::list<std::pair<Graph*, int>> GraphPointer;
};

std::vector<Graph*> Graphs;
int main()
{
	while (1)
	{
		int Number, Data, Count;
		std::list<std::pair<Graph*, int>> GraphList;
		std::cout << "输入图的标号，数据，图的度，连接的其他节点的标号，权值,标号已存在时停止输入";
		std::cin >> Number >> Data >> Count;
		while (Count)
		{
			int tmpID,length;
			std::cin >> tmpID>>length;
			Graph* Gra = FindGraphByID(tmpID);
			if (Gra)
			{
				GraphList.push_back(std::pair<Graph*, int>(Gra, length));
			}
		}
		if (FindGraphByID(Number))
		{
			std::cout << "标号已存在，停止输入";
			break;
		}
		auto Gra = new Graph;
		Gra->Data = Data;
		Gra->ID = Number;
		Gra->GraphPointer = GraphList;
		Graphs.push_back(Gra);
	}
	auto MST = GetMST();
}

Graph* FindGraphByID(int ID)
{
	for (int i=0;i<Graphs.size();i++)
	{
		if (Graphs[i]->ID == ID)
			return Graphs[i];
	}
}

std::vector<Graph*> GetMST()
{
	auto GraCopy1 = CopyGraph(Graphs);
	auto GraCopy2 = CopyGraph(Graphs);
	for (int i=0;i<GraCopy1.size();i++)
	{
		GraCopy1[i]->GraphPointer = std::list<std::pair<Graph*, int>>();
	}
	std::vector<Graph*> GraCopy3=CopyGraph(GraCopy1);
	for (int i=0;i<GraCopy2.size()-1;)
	{
		int themin = INT_MAX, aa;
		Graph* g;
		for (int a=0;a<GraCopy2.size();i++)
		{
			for (auto it=GraCopy2[a]->GraphPointer.begin();it!=GraCopy2[a]->GraphPointer.end();++it)
			{
				if (themin>it->second)
				{
					themin = it->second;
					g = it->first;
				}
			}
		}
		GraCopy1[aa]->GraphPointer.push_back(std::pair<Graph*, int>(g, themin));
		if (GraphHaveLoop(GraCopy1))
		{
			deleteGraph(GraCopy1);
			GraCopy1 = CopyGraph(GraCopy3);
		}
		else
		{
			deleteGraph(GraCopy3);
			GraCopy3 = CopyGraph(GraCopy1);
			i++;
		}
	}
}

std::vector<Graph*> CopyGraph(std::vector<Graph*> Gra)
{
	std::vector<Graph*> G;
	for (int i=0;i<Gra.size();i++)
	{
		Graph* gr = new Graph(Gra[i]);
		G.push_back(gr);
	}
	return G;
}

void deleteGraph(std::vector<Graph*> Gra)
{
	for (int i = 0; i < Gra.size(); i++)
	{
		delete Gra[i];
	}
}

bool GraphHaveLoop(std::vector<Graph*> Gra)
{
	std::vector<Graph*> G = CopyGraph(Gra);
	bool Deleted = true;
	while (Deleted)
	{
		Deleted = false;
		for ( int i=0;i<G.size();i++)
		{
			if (G[i]->GraphPointer.size()==1)
			{
				auto gg= G[i]->GraphPointer.begin()->first;
				for (auto it =gg->GraphPointer.begin();it!= gg->GraphPointer.end();++it)
				{
					if (it->first== G[i])
					{
						gg->GraphPointer.erase(it);
						break;
					}
				}
				delete G[i];
				G.erase(G.begin() + i);
				Deleted = true;
			}
		}
	}
	return G.size() > 0;
}