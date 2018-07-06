#include "Search.h"
#include "Game.h"
#include "PriorGomoku.h"
#include <algorithm>
#include <cstdlib>
#include <functional>
#include <iomanip>
#include <random>

//sum of weight should be 1.0
int randomSelect(const vector<float> &weight)
{
	static std::default_random_engine e(cfg_seed);
	static std::uniform_real<float> u(0.0f, 1.0f);
	float s = u(e);
	for (int i = 0; i < weight.size(); i++)
	{
		s -= weight[i];
		if (s < 0) return i;
	}
	ASSERT(false);
	return 0;
}
int randomSelect(BoardWeight weight, int count)
{
	static std::default_random_engine e(cfg_seed);
	static std::uniform_real<float> u(0.0f, 1.0f);
	float s = u(e);
	for (int i = 0; i < count; i++)
	{
		s -= weight[i];
		if (s < 0) return i;
	}
	ASSERT(false);
	return 0;
}

void MCTS::addNoise(int cur, Val epsilon, Val alpha) 
{
	static std::default_random_engine e;
	std::vector<Val> dirichlet;
	std::gamma_distribution<float> gamma(alpha, 1.0f);
	Val sum = 0;
	for (int i = 0; i < BLSIZE; i++) 
	{
		dirichlet.push_back(gamma(e));
		sum += dirichlet[i];
	}

	for (int i = 0; i < BLSIZE; i++)
	{
		int ch = (*tr[cur].ch)[i];
		if (ch) tr[ch].policy = (1-epsilon) * tr[ch].policy + epsilon * dirichlet[i] / sum;
	}
}

MCTS::MCTS(Board &_board, int _col, NN *_network, int _playouts):boardhash(_board)
{
	board = _board;
	nowcol = _col;
	network = _network;
	playouts = _playouts;
	tr = new Node[(playouts+2)*BLSIZE];
	chlist = new Board[playouts + 2];
	ravelist = new BoardWeight[playouts + 2];
	raveclist = new Board[playouts + 2];
	Prior::setbyBoard(board);
	Prior::setPlayer(nowcol);
	starttime = clock();
}

void MCTS::make_move(int move)
{
	ASSERT(move >= 0 && move < BSIZE*BSIZE);
	ASSERT(board(move) == 0);
	boardhash.update(move, board[move], nowcol);
	board[move] = nowcol;
	Prior::MakeMove(Coord(move));
	nowcol = nowcol % 2 + 1;
}

void MCTS::unmake_move(int move)
{
	ASSERT(move >= 0 && move < BSIZE*BSIZE);
	nowcol = nowcol % 2 + 1;
	ASSERT(board(move) == nowcol);
	boardhash.update(move, nowcol, 0);
	Prior::DelMove(Coord(move));
	board[move] = 0;
}

void MCTS::createRoot()
{
	trcnt = 1; chlistcnt = 0;
	stopflag = false;
	tr[root].cnt = 1;
	tr[root].fa = -1;
	tr[root].move = -1;
	auto result = getEvaluation(board, nowcol, network, use_transform);
	tr[root].sumv = -result.first.v;
	expand(root, result.first ,result.second);
	if (result.second.count() == 1) 
		stopflag = true;
	debug_s << "net win: " << vresultToWinrate(result.first.v) << "   ";
	if (result.first.v > 0.99)
		debug_s << "dec:win\n";
	else if (result.first.v <-0.99)
		debug_s << "dec:loss\n";

	if (add_noise)
		addNoise(root, 0.25f, 0.1f);
}

//UCT-RAVE selection, use father hueristic
int MCTS::selection(int cur)
{
	if (!tr[cur].ch)
		return cur;
	int maxp = 0;
	Val maxv = -FLOAT_INF;
	for (int i=0;i<BLSIZE;i++)
	{
		int ch = (*tr[cur].ch)[i];
		if (!ch) continue;
		Val ucb;
		Val var_ele = UCBC*tr[ch].policy*sqrtf((Val)tr[cur].cnt) / (1 + tr[ch].cnt);

		//rescale range
		Val father_val = (-tr[cur].sumv / tr[cur].cnt + 1.0f) / 1.1f - 1.0f;
		static const Val father_decay = 0.5f;
		Val frac1 = powf(father_decay, tr[ch].cnt);
		Val rave_cnt = (Val)(*tr[cur].cnt_rave)[i];
		Val rave_win = (*tr[cur].sum_rave)[i] / rave_cnt;
		//Val rave_beta = rave_cnt /(rave_cnt + tr[ch].cnt + 4*rave_cnt*tr[ch].cnt);
		Val rave_beta = 0.0f;
		if (tr[ch].is_end) frac1 = 0, rave_beta = 0;

		if (tr[ch].cnt == 0)
		{ 
			if (rave_cnt > 0.1f)
				ucb = rave_beta * rave_win + (1 - rave_beta)*(frac1 * father_val) + var_ele;
			else
				ucb = father_val + var_ele;
		}
		else
		{
			rave_beta /= 2;
			ucb = rave_beta * rave_win+(1-rave_beta)*(frac1 * father_val + (1 - frac1)*tr[ch].sumv / tr[ch].cnt) + var_ele; 
		}
		if (ucb > maxv)
		{
			maxv = ucb;
			maxp = ch;
		}
	}
	return maxp;
}

bool MCTS::getTimeLimit(int played)
{
	if (cfg_timelim)
	{
		clock_t t1 = clock();
		if (timeout_turn && t1 - starttime >= timeout_turn - 500)
			return true;
		Val maxrate = 0;
		for (int i = 0; i < BLSIZE; i++)
		{
			int ch = (*tr[root].ch)[i];
			if (ch) maxrate = std::max(maxrate, (float)tr[ch].cnt / played);
		}

		if (played > 600)
			if (maxrate > 0.9)
					return true;
		
		if (played > 1800)
			if (maxrate > 0.8)
				return true;

		if (played > 4000)
			if (maxrate > 0.7)
				return true;

		if (board.count() > 102 && played > 800 && fabs(tr[root].sumv / tr[root].cnt) < 0.05)
			return true;

		if (timeout_left)
		{ 
			int leftpos = (BLSIZE - board.count()) / 2 + 1;
			int ctrl_time = (float)timeout_left / leftpos * std::max((1.5 * (BLSIZE - board.count()) / 100.0),1.0);
			if (t1 - starttime >= ctrl_time - 100)
				return true;
		}
	}
	return false;
}

void MCTS::solve(BoardWeight &result)
{
	debug_s << "step:" << board.count() + 1<<'\n';
	createRoot();
	int played;
	for (int i = 0; i < playouts; i++)
	{
		if (stopflag && i>0) break;
		int cur = root;
		while (1)
		{
			int maxp = selection(cur);
			
			//leaf node
			if (maxp == cur) break;
			//forward search
			cur = maxp;
			make_move(tr[cur].move);
			//if (tr[cur].is_end) break;
		}
		//simulation & backpropagation
		simulation_back(cur);
		if (getTimeLimit(i)) stopflag = true;
		played = i;
	}
	mcwin = -tr[root].sumv / tr[root].cnt;
	debug_s << "mc win: " << vresultToWinrate(mcwin) << '\n' << "counter:" << counter << '\n'
		<< "time: " << clock() - starttime << "  playout: " << played << '\n';
	if (cfg_loglevel==2)
	{
		debug_s << board2showString(board, true);
		vector<std::pair<int, int>> pvlist;
		for (int i = 0; i < BLSIZE; i++)
		{
			int ch = (*tr[root].ch)[i];
			if (ch && tr[ch].cnt)
				pvlist.push_back({ tr[ch].cnt, ch });
		}
		sort(pvlist.begin(), pvlist.end());
		for (int i = 0; i < std::min(10, (int)pvlist.size());i++)
		{
			int v = pvlist[pvlist.size()-i-1].second;
			debug_s << std::setw(3) << Coord(tr[v].move).format()
				 << "  po:" << std::setw(5)<< tr[v].cnt
				<< " " << vresultToWinrate(tr[v].sumv/tr[v].cnt) << '\n';
		}
	}
	logRefrsh();
	result.clear();
	for (int i = 0; i < BLSIZE; i++)
	{
		int ch = (*tr[root].ch)[i];
		if (ch) result[i] = (Val)tr[ch].cnt;
		//std::cout << tr[ch].move << ' ' << tr[ch].cnt << ' ' << tr[ch].sumv / tr[ch].cnt << '\n';
	}
}

Val MCTS::getValue()
{
	int result = judgeWin(board);
	if (result)
		return result == nowcol ? -1.0f : 1.0f;
	return 0.0f;
}

void MCTS::expand(int cur,RawOutput &output, Board &avail)
{
	//board.debug();
	//std::cout<<"netwin:"<<output.v<<'\n';
	tr[cur].ch = &chlist[chlistcnt];
	tr[cur].sum_rave = &ravelist[chlistcnt];
	tr[cur].cnt_rave = &raveclist[chlistcnt];
	(*tr[cur].ch).clear();
	(*tr[cur].sum_rave).clear();
	(*tr[cur].cnt_rave).clear();
	chlistcnt++;
	for (int i = 0; i < BLSIZE; i++)
		if (avail[i]) //for valid
		{
			(*tr[cur].ch)[i]=trcnt;
			tr[trcnt].sumv = 0.0f;
			tr[trcnt].ch = nullptr;
			tr[trcnt].sum_rave = nullptr;
			tr[trcnt].cnt_rave = nullptr;
			tr[trcnt].cnt = 0;
			tr[trcnt].policy = output.p[i];
			tr[trcnt].move = i;
			tr[trcnt].fa = cur;
			tr[trcnt].is_end = false;
			trcnt++;
		}
}

void MCTS::simulation_back(int cur)
{
	Val val;
	if (tr[cur].cnt == 0 && judgeWin(board) == 0 && board.count() < BLSIZE) //isn't end node
	{
		auto result = getEvaluation(board, nowcol, network, use_transform, tr[cur].move);
		val = -result.first.v;
		expand(cur, result.first, result.second);
		if (val<-0.99 || val>0.99) tr[cur].is_end = true;
		//auto &it = hash_table.find(boardhash());
		//if (it != hash_table.end()) counter++;
		//else hash_table[boardhash()] = cur;
	}
	else
	{
		val = 1; 
		if (board.count() == BLSIZE)
			val = 0;
		tr[cur].is_end = true;
	}
	if (cfg_swap3 && board.count() == 3 && board.countv(2) == 1 && tr[cur].fa>0) //if swap, player choice max rate point
		val = -fabs(val);
	
backprop:
	int tcur = cur;
	int move = tr[cur].move;
	while (tcur > 0)
	{
		tcur = tr[tcur].fa;
		(*tr[tcur].sum_rave)[move] += val;
		(*tr[tcur].cnt_rave)[move] ++;
		tcur = tr[tcur].fa;
	}

	tr[cur].sumv += val;
	tr[cur].cnt++;

	while (cur > 0)
	{
		unmake_move(tr[cur].move);
		if (cfg_swap3 && board.count() == 3 && board.countv(2) == 1 && tr[cur].fa>0) //if swap, player choice max rate point
		{
			tr[cur].sumv -= val;
			val = fabs(val);
			tr[cur].sumv += val;
		}
		cur = tr[cur].fa;
		val = - val;
		tr[cur].sumv += val;
		tr[cur].cnt++;
	}
}

int MCTS::solvePolicy(Val te, BoardWeight &policy, float &winrate)
{
	solve(policy);
	winrate = -tr[0].sumv / tr[0].cnt;
	auto tpolicy = policy;

	{
		Val sum = 0;
		for (int i = 0; i < BLSIZE; i++)
		{
			policy[i] = powf(policy[i], 1.0f);
			sum += policy[i];
		}
		for (int i = 0; i < BLSIZE; i++)
			policy[i] /= sum;
	}

	//Tempearture
	if (te < 0.2) //tau-->0
	{
		int maxc; Val maxv=-FLOAT_INF;
		for (int i = 0; i < BLSIZE; i++)
			if (maxv < tpolicy[i])
			{
				maxc = i;
				maxv = tpolicy[i];
			}
		for (int i = 0; i < BLSIZE; i++) tpolicy[i] = 0;
		tpolicy[maxc] = 1;
		return maxc;
	}
	else
	{
		Val sum = 0;
		for (int i = 0; i < BLSIZE; i++)
		{
			tpolicy[i] = powf(tpolicy[i], 1.0f / te);
			sum += tpolicy[i];
		}
		for (int i = 0; i < BLSIZE; i++)
			tpolicy[i] /= sum;
		return randomSelect(tpolicy, BLSIZE);
	}
}

Coord Player::randomOpening(Board gameboard)
{
	if (gameboard.count() == 0)
	{
		return { 2+rand()%11, 2 + rand() % 11 };
	}/*
	else if (gameboard.count() == 1)
	{
		int first;
		for (int i = 0; i < BLSIZE; i++)
			if (gameboard[i] == 1)
				first = i;
		int dir = rand() % 8;
		return { cx[dir] + first / BSIZE, cy[dir] + first % BSIZE };
	}
	else
	{
		int first;
		for (int i = 0; i < BLSIZE; i++)
			if (gameboard[i] == 1)
				first = i;
		int x = first / BSIZE, y = first%BSIZE;
		BoardWeight po;
		po.clear();
		float sum = 0;
		for (int i = x-3; i <= x+3; i++)
			for (int j = y-3; j <= y+3; j++)
			{
				if (gameboard(i, j) == 0)
					if (i >= x-1 && i <= x+1 && j >= y-1 && j <= y+1)
						po(i, j) = 1, sum+=1;
					else
						po(i, j) = 1, sum+=1;
			}
		for (int i = 0; i < BLSIZE; i++)
			po[i] /= sum;
		return randomSelect(po, BLSIZE);
	}*/
}

Coord Player::run(Board &gameboard, int nowcol)
{
	MCTS mcts(gameboard, nowcol, &network, cfg_playouts);
	mcts.add_noise = cfg_add_noise;
	mcts.UCBC = cfg_puct;
	mcts.use_transform = cfg_use_transform;
	Coord ret;
	if (gameboard.count()>=cfg_temprature_moves)
		ret = Coord(mcts.solvePolicy(cfg_temprature2, policy, winrate));
	else
		ret = Coord(mcts.solvePolicy(cfg_temprature1, policy, winrate));
	//if (gameboard.count() == 0) return randomOpening(gameboard);
	if (cfg_swap3 && gameboard.count() == 3 && gameboard.countv(2)==1 && mcts.mcwin<0)
		return Coord(BLSIZE);
	return ret;
}
