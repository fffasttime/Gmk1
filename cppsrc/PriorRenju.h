#pragma once

#include "Common.h"
#include "Board.h"

namespace PriorRenju
{
	void MakeMove(Coord c);
	void DelMove(Coord c);
	void setPiece(int move, int col);
	void setbyBoard(Board &board);
	void setPlayer(int col);

	int GenerateMove(std::vector<int> &result);

	void initPrior();
}
