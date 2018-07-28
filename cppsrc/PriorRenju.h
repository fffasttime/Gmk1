#pragma once

#include "Common.h"
#include "Board.h"
#include <algorithm>

namespace PriorRenju
{
#define BDSIZE (BSIZE + 8)
	void MakeMove(Coord c);
	void DelMove(Coord c);
	void setPiece(int move, int col);
	void setbyBoard(Board &board);
	void setPlayer(int col);

	int GenerateMove(std::vector<int> &result);

	void initPrior(); 
	
	enum Pieces { Black = 0, White = 1, Empty = 2, Outside = 3 };
	struct Cell
	{
		int piece;
		int pattern[2][4];
		bool forbidden;
		void debugprint2()
		{
			std::cout << std::max(std::max(pattern[0][0], pattern[0][1]), std::max(pattern[0][2], pattern[0][3]));
			std::cout << std::max(std::max(pattern[1][0], pattern[1][1]), std::max(pattern[1][2], pattern[1][3]));
		}
		void debugprint1()
		{
			if (piece == Empty) std::cout << "  ";
			else std::cout << ' ' << piece;
		}
		int maxv(int col)
		{
			return std::max(std::max(pattern[col][0], pattern[col][1]), std::max(pattern[col][2], pattern[col][3]));
		}
	};
	extern BoardArray<Cell, BDSIZE> cell;

	bool AvailablePos(int pos);
	void checkForbiddens();
}
