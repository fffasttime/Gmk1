#include "Evaluation.h"
#include "PriorGomoku.h"
#include <algorithm>

RawInput::RawInput(Board &board)
{
	for (int i = 0; i < BLSIZE; i++)
		if (board[i] == C_B)
		{
			feature[0][i] = 1; 
			feature[1][i] = 0;
		}
		else if (board[i]==C_W)
		{
			feature[0][i] = 0; 
			feature[1][i] = 1;
		}
		else
		{
			feature[0][i] = 0; 
			feature[1][i] = 0;
		}
}

std::pair<RawOutput, Board> getEvaluation(Board board, int col, NN *network, bool use_transform, int lastmove)
{
#if 0
	RawOutput output;
	std::vector<int> avail;
	output.v = Prior::GenerateMove(avail);
	output.p.clear();
	for (int i = 0; i < avail.size(); i++)
		output.p[avail[i]] = 1.0f / avail.size();
	Board avail_list;
	avail_list.clear();
	for (auto move : avail)
		avail_list[move] = 1;
	return {output, avail_list};
#else
	int trans;
	if (use_transform)
		trans = rand() % 8;
	else
		trans = 0;
	boardTransform(trans, board);
	Network::NNPlanes input;
#ifdef RULE_GOMOKU
	input.resize(2);
#endif
#ifdef RULE_RENJU
	input.resize(3);
#endif
	for (int i = 0; i < BLSIZE; i++)
	{
	#ifdef RULE_GOMOKU
		if (board[i] == 0)
		{
			input[0][i] = 0;
			input[1][i] = 0;
		}
		else if (board[i] == col)
		{
			input[0][i] = 1;
			input[1][i] = 0;
		}
		else
		{
			input[0][i] = 0;
			input[1][i] = 1;
		}
	#endif
	#ifdef RULE_RENJU
		input[0][i] = (board[i] == 1);
		input[1][i] = (board[i] == 2);
		input[2][i] = col - 1;
	#endif
	}
#if 0
	assert(Prior::who==col-1);
	Prior::getFeature();
	for (int k = 0; k<8; k++)
	{ 
		boardTransform(trans, Prior::featurelayer[k / 4][k % 4]);
		for (int i = 0; i<BSIZE; i++)
			for (int j = 0; j<BSIZE; j++)
				input[k+2][i*BSIZE+j] = Prior::featurelayer[k / 4][k % 4](i,j);
	}
#endif
	std::vector<int> avail;
	int simplewin = Prior::GenerateMove(avail);
	Board avail_list;
	avail_list.clear();
	for (auto move : avail)
		avail_list[move] = 1;
	boardTransform(trans, avail_list);
	auto ret = network->forward(input);

	double sum_policy = 0.0;
	//scale sum_policy to 1
	for (int i = 0; i < BLSIZE; i++)
		if (avail_list[i])
			sum_policy += ret.first[i];
		else
			ret.first[i] = 0;

	RawOutput output;
	if (sum_policy > 1e-10)
		for (int i = 0; i < BLSIZE; i++)
			output.p[i] = (float)(ret.first[i] / sum_policy);

	boardTransform(trans + 8, output.p);
	boardTransform(trans + 8, avail_list);
	if (simplewin)
		output.v = simplewin;
	else
		output.v = ret.second  * 2.0f - 1.0f;
	return {output,avail_list};
#endif
}

double vresultToWinrate(double v) 
{
	return (int)((v + 1.0) * 5000.0+0.5)/100.0;
}