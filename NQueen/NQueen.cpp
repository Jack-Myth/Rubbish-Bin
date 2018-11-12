#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <vector>
#include <stdlib.h>

void SolveNQueen(std::vector<std::vector<unsigned char>>& _Chessboard, int n);
void PrintCheeseboard(std::vector<std::vector<unsigned char>>& _Chessboard);
int numc=0;
int main()
{
	int N;
	printf("Input N:");
	scanf("%d", &N);
	std::vector<std::vector<unsigned char>> tmpChessboard;
	tmpChessboard.resize(N, std::vector<unsigned char>(N, 0));
	SolveNQueen(tmpChessboard,0);
	system("pause>nul");
	printf("\n%d\n", numc);
	system("pause>nul");
}

void SolveNQueen(std::vector<std::vector<unsigned char>>& _Chessboard,int n)
{
	for (int i=0;i<_Chessboard[n].size();i++)
	{
		//0->empty
		//1->placed
		//2->not placeable
		if (_Chessboard[n][i]==2)
			continue;
		else //can't be 1,must be 0
		{
			std::vector<std::vector<unsigned char>> tmpChessboard = _Chessboard;
			tmpChessboard[n][i] = 1;
			if (n+1==_Chessboard.size())
			{
				PrintCheeseboard(tmpChessboard);
				//system("pause>nul");
				system("cls");
				numc++;
				continue;
			}
			//vertical
			for (int y=0;y< tmpChessboard[n].size();y++)
			{
				if (y==i)
					continue;
				tmpChessboard[n][y] = 2;
			}
			//horizontal
			for (int x = 0; x < tmpChessboard.size(); x++)
			{
				if (x == n)
					continue;
				tmpChessboard[x][i] = 2;
			}
			for (int xy = 1; xy<tmpChessboard.size(); xy++)
			{
				if(n + xy<tmpChessboard.size()&& i + xy<tmpChessboard.size())
					tmpChessboard[n + xy][i + xy] = 2;
				if (n - xy >= 0 && i + xy < tmpChessboard.size())
					tmpChessboard[n - xy][i + xy] = 2;
				if (n + xy < tmpChessboard.size() && i - xy >= 0)
					tmpChessboard[n + xy][i - xy] = 2;
				if (n - xy >= 0 && i - xy >= 0)
					tmpChessboard[n - xy][i - xy] = 2;
			}
			SolveNQueen(tmpChessboard, n + 1);
		}
	}
}

void PrintCheeseboard(std::vector<std::vector<unsigned char>>& _Chessboard)
{
	for (int y = 0; y < _Chessboard.size(); y++)
	{
		for (int x = 0; x < _Chessboard.size(); x++)
		{
			printf(_Chessboard[x][y] == 1 ? "¡ö" : "¡õ");
		}
		printf("\n");
	}
}