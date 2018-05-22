#pragma once
#include "Common.h"
#include "Board.h"
#include "NN/nn_cpp.h"
#include <array>

namespace Prior
{
	void initPrior();
	void setPiece(int move, int col);
	void setPlayer(int col);
	void setbyBoard(Board &board);
	void reset();
	void MakeMove(Coord c);
	void DelMove(Coord c);
	void debugPrint();
	int GenerateMove(std::vector<int> &result);
}

struct RawInput
{
	//color w, color b
	float feature[2][BLSIZE];
	RawInput(Board &board);
};

struct RawOutput
{
	BoardWeight p;
	float v;
};

std::pair<RawOutput, Board> getEvaluation(Board board, int col, NN *network, bool use_transform = true, int lastmove=-1);

double vresultToWinrate(double v);