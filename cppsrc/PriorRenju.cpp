#include "PriorRenju.h"
#include <algorithm>

namespace PriorRenju
{
	typedef unsigned long long U64;
	const int toolong = 8;
	const int win = 7;
	const int flex4 = 6;
	const int block4 = 5;
	const int flex3 = 4;
	const int block3 = 3;
	const int flex2 = 2;
	const int block2 = 1;
	const int b_start = 4, b_end = BSIZE + 4;

	int pval[8][8][8][8];
	int typeTable[10][6][6][3];
	int patternTable[65536][2];
	
	//coordinate with cx, cy
	constexpr int range[8] = { -BDSIZE-1,-BDSIZE,-BDSIZE+1,1,BDSIZE+1,BDSIZE,BDSIZE-1,-1 };

	int Range4[32];
	constexpr int makePoint(int x, int y) { return x*BDSIZE + y; }
	constexpr int upperLeft = makePoint(BSIZE + 3, BSIZE + 3);
	constexpr int lowerRight = makePoint(4, 4);

	BoardArray<Cell, BDSIZE> cell;
	bool inBorderD(int x, int y) { return x >= 4 && y >= 4 && x < BDSIZE - 4 && y < BDSIZE - 4; }

	struct Point
	{
		int p;
		int val;
	};

	int who = 0, opp = 1;

	int GetKey(int x, int y, int i)
	{
		const int stepX = cx[i];
		const int stepY = cy[i];
		//mode ?
		int key = (cell(x - stepX * 4, y - stepY * 4).piece)
			^ (cell(x - stepX * 3, y - stepY * 3).piece << 2)
			^ (cell(x - stepX * 2, y - stepY * 2).piece << 4)
			^ (cell(x - stepX * 1, y - stepY * 1).piece << 6)
			^ (cell(x + stepX * 1, y + stepY * 1).piece << 8)
			^ (cell(x + stepX * 2, y + stepY * 2).piece << 10)
			^ (cell(x + stepX * 3, y + stepY * 3).piece << 12)
			^ (cell(x + stepX * 4, y + stepY * 4).piece << 14);
		return key;
	}

	bool IsType(Cell *c, int role, int type)
	{
		return c->pattern[role][0] == type || c->pattern[role][1] == type || c->pattern[role][2] == type || c->pattern[role][3] == type;
	}
	void UpdateType(int x, int y);
	//only for black forbidden check
	void MakeMove_NoCheck(int next)
	{
		int who0 = who, opp0 = opp;
		who = 0, opp = 1;
		cell[next].piece = who;
		int x = next / BDSIZE, y = next%BDSIZE;
		UpdateType(x, y);
		who = who0, opp = opp0;
	}
	void DelMove_NoCheck(int next)
	{
		int x = next / BDSIZE, y = next%BDSIZE;
		cell[next].piece = Empty;
		UpdateType(x, y);
	}
	bool checkForbidden(int x, int y);
	bool recheckFlex4Line(int x, int y, int d)
	{
		int pos = x*BDSIZE + y;
		MakeMove_NoCheck(pos);
		int p1 = pos + range[d];
		while (cell[p1].piece == 0)
			p1 += range[d];
		int p2 = pos - range[d];
		while (cell[p2].piece == 0)
			p2 -= range[d];
		bool flag = checkForbidden(p1 / BDSIZE, p1%BDSIZE) || 
					checkForbidden(p2 / BDSIZE, p2%BDSIZE);
		DelMove_NoCheck(pos);
		return !flag;
	}
	bool checkFlex3Line(int x, int y, int d)
	{
		int pos = x*BDSIZE + y;
		for (int j=1;j<4;j++)
		{
			int pos1 = pos + range[d] * j;
			if (cell[pos1].piece == Empty && IsType(&cell[pos1], 0, flex4))
				if (!checkForbidden(pos1 / BDSIZE, pos1%BDSIZE) && !IsType(&cell[pos1], 0, win))
					if (recheckFlex4Line(pos1 / BDSIZE, pos1%BDSIZE, d))
						return true;
		}
		for (int j=1;j<4;j++)
		{
			int pos1 = pos - range[d] * j;
			if (cell[pos1].piece == Empty && IsType(&cell[pos1], 0, flex4))
				if (!checkForbidden(pos1 / BDSIZE, pos1%BDSIZE) && !IsType(&cell[pos1], 0, win))
					if (recheckFlex4Line(pos1 / BDSIZE, pos1%BDSIZE,d))
						return true;
		}
		return false;
	}
	bool check4SameLine(int x, int y, int d)
	{
		int pos = x*BDSIZE + y;
		int p1 = pos + range[d], i=0, j=0;
		while (cell[p1].piece == 0 && i<1)
			p1 += range[d], i++;
		int p2 = pos - range[d];
		while (cell[p2].piece == 0 && j<1)
			p2 -= range[d], j++;
		if (cell[p1].piece == 2 && cell[p2].piece == 2 &&
			(cell[p1].pattern[0][d] == flex4 || cell[p1].pattern[0][d] == block4) && 
			(cell[p2].pattern[0][d] == flex4 || cell[p2].pattern[0][d] == block4))
			return true;
		return false;
	}
	bool checkForbidden(int x, int y) 
	{
		int pos = x*BDSIZE + y;
		if (IsType(&cell[pos], 0, win)) return false;
		if (IsType(&cell[pos], 0, toolong)) return true;
		int c3=0, c4 = 0;
		for (int i = 0; i < 4; i++)
		{
			if (cell[pos].pattern[0][i] == block4 ||
				cell[pos].pattern[0][i] == flex4)
			{
				c4++;
				if (check4SameLine(x, y, i))
					return true; 
			}
		}
		if (c4 > 1) return true;
		for (int i = 0; i < 4; i++)
		{
			if (cell[pos].pattern[0][i] == flex3)
				c3++;
		}
		if (c3 > 1) //recheck
		{ 
			c3 = 0;
			MakeMove_NoCheck(pos);
			for (int i = 0; i < 4; i++)
				if (cell[pos].pattern[0][i] == flex3)
					if (checkFlex3Line(x,y,i))
						c3++;
			DelMove_NoCheck(pos);
			if (c3 > 1) return true;
		}
		return false;
	}

	void checkForbiddens() 
	{
		for (int i = 4; i < 4 + BSIZE; i++)
			for (int j = 4; j < 4 + BSIZE; j++)
				if (cell(i, j).piece == Empty)
					if (checkForbidden(i, j))
						cell(i, j).forbidden = 1;
					else
						cell(i, j).forbidden = 0;
	}

	void UpdateType(int x, int y)
	{
		int a, b;
		int key;

		for (int i = 0; i < 4; ++i)
		{
			int pos;
			a = x + cx[i];
			b = y + cy[i];
			for (int j = 0; j < 4 && inBorderD(a, b); a += cx[i], b += cy[i], ++j)
			{
				key = GetKey(a, b, i);
				pos = a*BDSIZE + b;
				cell[pos].pattern[0][i] = patternTable[key][0];
				cell[pos].pattern[1][i] = patternTable[key][1];
			}
			a = x - cx[i];
			b = y - cy[i];
			for (int k = 0; k < 4 && inBorderD(a, b); a -= cx[i], b -= cy[i], ++k)
			{
				key = GetKey(a, b, i);
				pos = a*BDSIZE + b;
				cell[pos].pattern[0][i] = patternTable[key][0];
				cell[pos].pattern[1][i] = patternTable[key][1];
			}
		}
	}

	int CheckFlex4(int line[4])
	{
		int i, j, count;

		int five = 0;
		int role = line[4];
		for (i = 0; i < 9; i++)
			if (line[i] == Empty)
			{
				count = 0;
				for (j = i - 1; j >= 0 && line[j] == role; j--)
					count++;
				for (j = i + 1; j <= 8 && line[j] == role; j++)
					count++;
				if (count >= 4)
					five++;
			}

		return five >= 2 ? flex4 : block4;
	}

	int CheckFlex3(int line[9])
	{
		int role = line[4];
		int type;
		for (int i = 0; i < 9; i++)
		{
			if (line[i] == Empty)
			{
				line[i] = role;
				type = CheckFlex4(line);
				line[i] = Empty;
				if (type == flex4)
					return flex3;
			}
		}
		return block3;
	}

	int ShortLine(int line[9])
	{
		int kong = 0, block = 0;
		int len = 1, len2 = 1, count = 1;
		int k;

		int who = line[4];
		for (k = 5; k <= 8; k++)
		{
			if (line[k] == who)
			{
				if (kong + count > 4)
					break;
				++count;
				++len;
				len2 = kong + count;
			}
			else if (line[k] == Empty)
			{
				++len;
				++kong;
			}
			else
			{
				if (line[k - 1] == who)
					block++;
				break;
			}
		}
		kong = len2 - count;
		for (k = 3; k >= 0; k--)
		{
			if (line[k] == who)
			{
				if (kong + count > 4) break;
				++count;
				++len;
				len2 = kong + count;
			}
			else if (line[k] == Empty)
			{
				++len;
				++kong;
			}
			else
			{
				if (line[k + 1] == who)
					block++;
				break;
			}
		}
		if (count == 5)
		{
			count = 1;
			for (k = 5; k <= 8 && line[k] == who; k++)
				count++;
			for (k = 3; k >= 0 && line[k] == who; k--)
				count++;
			if (count > 5) return toolong;
		}
		return typeTable[len][len2][count][block];
	}

	int LineType(int role, int key)
	{
		int line_left[9];
		int line_right[9];

		for (int i = 0; i < 9; i++)
			if (i == 4)
			{
				line_left[i] = role;
				line_right[i] = role;
			}
			else
			{
				line_left[i] = key & 3;
				line_right[8 - i] = key & 3;
				key >>= 2;
			}

		int p1 = ShortLine(line_left);
		int p2 = ShortLine(line_right);

		if (p1 == block3 && p2 == block3)
			return CheckFlex3(line_left);
		else if (p1 == block4 && p2 == block4)
			return CheckFlex4(line_left);
		else
			return p1 > p2 ? p1 : p2;
	}

	int GenerateAssist(int len, int len2, int count, int block)
	{
		if (len >= 5 && count > 1)
		{
			if (count == 5)
				return win;
			if (len > 5 && len2 < 5 && block == 0)
			{
				switch (count)
				{
				case 2:
					return flex2;
				case 3:
					return flex3;
				case 4:
					return flex4;
				}
			}
			else
			{
				switch (count)
				{
				case 2:
					return block2;
				case 3:
					return block3;
				case 4:
					return block4;
				}
			}
		}
		return 0;
	}

	int GetPval(int a, int b, int c, int d)
	{
		int type[8] = { 0 };
		type[a]++; type[b]++; type[c]++; type[d]++;

		if (type[win] > 0)
			return 5000;
		if (type[flex4] > 0)
			return 1300;
		if(type[block4] > 1)
			return 1200;
		if (type[block4] > 0 && type[flex3] > 0)
			return 1000;
		if (type[flex3] > 1)
			return 200;

		int val[6] = { 0, 2, 5, 5, 12, 12 };
		int score = 0;
		for (int i = 1; i <= block4; i++)
			score += val[i] * type[i];

		return score;
	}

	void InitChessType()
	{
		for (int i = 0; i < 10; ++i)
			for (int j = 0; j < 6; ++j)
				for (int k = 0; k < 6; ++k)
					for (int l = 0; l < 3; ++l)
						typeTable[i][j][k][l] = GenerateAssist(i, j, k, l);
		for (int key = 0; key < 65536; key++)
		{
			patternTable[key][0] = LineType(0, key);
			patternTable[key][1] = LineType(1, key);
			//regard white long link as win
			if (patternTable[key][1] == toolong)
				patternTable[key][1] = win;
		}
		for (int i = 0; i < 8; ++i)
			for (int j = 0; j < 8; ++j)
				for (int k = 0; k < 8; ++k)
					for (int l = 0; l < 8; ++l)
						pval[i][j][k][l] = GetPval(i, j, k, l);
	}

	void MakeMove(int next)
	{
		cell[next].piece = who;
		who ^= 1;
		opp ^= 1;
		int x = next / BDSIZE, y = next%BDSIZE;
		UpdateType(x, y);
	}
	void MakeMove(Coord c) { MakeMove((c.x + 4)*BDSIZE + c.y + 4); }
	void DelMove(int next)
	{
		int x = next / BDSIZE, y = next%BDSIZE;
		who ^= 1;
		opp ^= 1;
		cell[next].piece = Empty;
		UpdateType(x, y);
	}
	void DelMove(Coord c) { DelMove((c.x + 4)*BDSIZE + c.y + 4); }
	bool AvailablePos(int pos) 
	{
		return cell[pos].piece == Empty && (who == White || !cell[pos].forbidden);
	}
	bool AvailablePosOpp(int pos) 
	{
		return cell[pos].piece == Empty && (who == Black || !cell[pos].forbidden);
	}
	void reset()
	{
		for (int i = 0; i < BDSIZE; i++)
			for (int j = 0; j < BDSIZE; j++)
				if (i >= 4 && i < BDSIZE - 4 && j >= 4 && j < BDSIZE - 4)
					cell(i, j) = Cell({ Empty,{ { 0,0,0,0 },{ 0,0,0,0 } }, false });
				else
					cell(i, j) = Cell({ Outside,{ { 0,0,0,0 },{ 0,0,0,0 } }, false });
	}
	void debugPrint()
	{
		for (int i = 0; i < BDSIZE; i++, std::cout << '\n')
			for (int j = 0; j < BDSIZE; j++)
				if (i >= 4 && i < BDSIZE - 4 && j >= 4 && j < BDSIZE - 4)
					cell(i, j).debugprint1();

		for (int i = 0; i < BDSIZE; i++, std::cout << '\n')
			for (int j = 0; j < BDSIZE; j++)
				if (i >= 4 && i < BDSIZE - 4 && j >= 4 && j < BDSIZE - 4)
					cell(i, j).debugprint2();
	}
	void setPiece(int move, int col)
	{
		int x = move / BSIZE + 4, y = move%BSIZE + 4;
		if (col)
			cell(x, y).piece = col - 1;
		else
			cell(x, y).piece = Empty;
		UpdateType(x, y);
	}
	void setPlayer(int col)
	{
		who = col - 1;
		opp = who ^ 1;
	}
	void setbyBoard(Board &board)
	{
		reset();
		for (int i = 0; i < BSIZE; i++)
			for (int j = 0; j < BSIZE; j++)
				setPiece(i * BSIZE + j, board(i, j));
		checkForbiddens();
	}

	int EvaluateMove(Cell *c)
	{
		int score[2];
		score[who] = pval[c->pattern[who][0]][c->pattern[who][1]][c->pattern[who][2]][c->pattern[who][3]];
		score[opp] = pval[c->pattern[opp][0]][c->pattern[opp][1]][c->pattern[opp][2]][c->pattern[opp][3]];

		if (score[who] >= 200 || score[opp] >= 200)
			return score[who] >= score[opp] ? score[who] * 2 : score[opp];
		else
			return score[who] * 2 + score[opp];
	}
	bool defendPoint[BDSIZE*BDSIZE];
	int CutMoveList(std::vector<int> &moves, std::vector<Point> cand)
	{
		memset(defendPoint, 0, sizeof(defendPoint));
		int candCount = cand.size();
		/*
		iwin: 10000  owin: 5000 if4:2600 i2b4:2400 ibf&if3:2000 of4:1300 o2b4:1200 obf&of3:1000 ilive2:400
		*/
		if (who == White) {
			if (cand[0].val == 10000)
			{
				moves.push_back(cand[0].p);
				for (int i = 1; i < candCount && cand[i].val == cand[i - 1].val; i++)
					moves.push_back(cand[i].p);
				return 1;
			}
			if (cand[0].val == 5000 && !cell[cand[0].p].forbidden)
			{
				moves.push_back(cand[0].p);
				int i = 1;
				for (; i < candCount && cand[i].val == cand[i - 1].val; i++)
					if (!cell[cand[i].p].forbidden)
						moves.push_back(cand[i].p);
				if (moves.size() > 1) return -1; 
				return 0;
			}
			if (cand[0].val >= 2400)
			{
				moves.push_back(cand[0].p);
				int i = 1;
				for (; i < candCount && cand[i].val == cand[i - 1].val; i++)
					moves.push_back(cand[i].p);
				return 1;
			}
			if (cand[0].val == 2000)
			{
				int v = cand[0].p;
				for (auto j : Range4)
					if (cell[j + v].piece == Empty && (IsType(&cell[j + v], opp, flex4) || IsType(&cell[j + v], opp, block4))) //no more link-5
					{ 
						cand.erase(cand.begin());
						goto next;
					}
				moves.push_back(cand[0].p);
				return 1;
			}
		next:
			return 0;
			/*
			if (cand[0].val == 1300 && !cell[cand[0].p].forbidden)
			{
				moves.push_back(cand[0].p);
				for (int i = 1; i < candCount && cand[i].val == cand[i - 1].val; i++)
					if (!cell[cand[i].p].forbidden)
					{ 
						moves.push_back(cand[i].p);
						int cur = cand[i].p;
						for (int d = 0; d < 4; d++)
							if (cell[cur].pattern[opp][d] == flex4)
								for (int j = -3; j <= 3; j++)
									if (cell[cur + range[d] * j].piece == Empty && IsType(&cell[cur + range[d] * j], opp, block4))
										defendPoint[cur + range[d] * j] = true;
					}
				int s0 = moves.size();
				for (int i = moves.size(); i < candCount; ++i)
				{
					Cell *p = &cell[cand[i].p];
					if (IsType(p, who, block4))
						moves.push_back(cand[i].p);
				}
				return 0;
			}
			if (cand[0].val == 1000 && !cell[cand[0].p].forbidden)
			{
				moves.push_back(cand[0].p);
				for (int i = 1; i < candCount && cand[i].val == cand[i - 1].val; i++)
					moves.push_back(cand[i].p);
				int s0 = moves.size();
				for (int i = moves.size(); i < candCount; ++i)
				{
					Cell *p = &cell[cand[i].p];
					if (IsType(p, who, block4))
						moves.push_back(cand[i].p);
				}
				int cur = cand[0].p;
				for (int d = 0; d < 4; d++)
					if (cell[cur].pattern[opp][d] == flex3)
						for (int i = -4; i <= 4; i++)
							if (cell[cur + range[d] * i].piece == Empty && IsType(&cell[cur + range[d] * i], opp, flex3))
								defendPoint[cur + range[d] * i] = true;
				return 0;
			}*//*
			if (cand[0].val == 400)
			{
				int i;
				for (i = moves.size(); i < candCount; ++i)
					if (IsType(&cell[cand[i].p], opp, block4) || IsType(&cell[cand[i].p], opp, flex4))
						break;
				if (i == candCount)
				{
					moves.push_back(cand[0].p);
					return 1;
				}
			}*/
		}
		else {
			if (cand[0].val >= 5000)
			{
				moves.push_back(cand[0].p);
				int i = 1;
				for (; i < candCount && cand[i].val == cand[i - 1].val; i++)
					moves.push_back(cand[i].p);
				if (cand[0].val == 10000)
					return 1;
				if (cell[cand[0].p].forbidden) { moves.clear(); return -1; };
				if (i > 1) { moves.clear(); return -1; }
				return 0;
			}
			std::vector<Point> temp(cand);
			cand.clear();
			for (auto &point : temp)
				if (!cell[point.p].forbidden)
					cand.push_back(point);
			if (cand.empty()) return -1;
			if (cand[0].val == 2600)
			{
				moves.push_back(cand[0].p);
				int i = 1;
				for (; i < candCount && cand[i].val == cand[i - 1].val; i++)
					moves.push_back(cand[i].p);
				return 1; 
			}
			if (cand[0].val == 2000)
			{
				int v = cand[0].p;
				for (int d = 0; d < 4; d++)
					if (cell[v].pattern[who][d] == flex3)
						if (!checkFlex3Line(v / BDSIZE, v%BDSIZE, d))
						{
							cand.erase(cand.begin());
							goto next2; 
						}
				for (auto j : Range4)
					if (cell[j + v].piece == Empty && (IsType(&cell[j + v], opp, flex4) || IsType(&cell[j + v], opp, block4))) //no more link-5
					{
						cand.erase(cand.begin());
						goto next2;
					}
				moves.push_back(cand[0].p);
				return 1;
			}
		next2:
			return 0;/*
			if (cand[0].val == 1300 || cand[0].val == 1200 || cand[0].val == 1000)
			{
				moves.push_back(cand[0].p);
				for (int i = 1; i < candCount && cand[i].val == cand[i - 1].val; i++)
					moves.push_back(cand[i].p);
				int s0 = moves.size();
				for (int i = moves.size(); i < candCount; ++i)
				{
					Cell *p = &cell[cand[i].p];
					if (IsType(p, who, block4))
						moves.push_back(cand[i].p);
				}
				if (cand[0].val==1000)
				{ 
					int cur = cand[0].p;
					for (int d = 0; d < 4; d++)
						if (cell[cur].pattern[opp][d] == flex3)
							for (int i = -3; i <= 3; i++)
								if (cell[cur + range[d] * i].piece == Empty && IsType(&cell[cur + range[d] * i], opp, flex3))
									defendPoint[cur + range[d] * i] = true;
				}
				return 0;
			}
			if (cand[0].val == 200)
			{
				moves.push_back(cand[0].p);
				for (int i = 1; i < candCount && cand[i].val == cand[i - 1].val; i++)
					moves.push_back(cand[i].p);
				int s0 = moves.size();
				for (int i = moves.size(); i < candCount; ++i)
				{
					Cell *p = &cell[cand[i].p];
					if (IsType(p, who, block4) || IsType(p, who, flex3) || IsType(p, who, block3))
						moves.push_back(cand[i].p);
				}
				int cur = cand[0].p;
				for (int d = 0; d < 4; d++)
					if (cell[cur].pattern[opp][d] == flex3)
						for (int i = -3; i <= 3; i++)
							if (cell[cur + range[d] * i].piece == Empty && IsType(&cell[cur + range[d] * i], opp, flex3))
								defendPoint[cur + range[d] * i] = true;
				return 0;
			}
			*/
		}
		
		return 0;
	}

	bool c1(std::vector<Point> &cand)
	{
		int cb = 0, cw = 0, bx, by;
		for (int i = b_start; i < b_end; i++)
			for (int j = b_start; j < b_end; j++)
				if (cell[i*BDSIZE + j].piece == Black)
					cb++, bx = i, by = j;
				else if (cell[i*BDSIZE + j].piece == White)
					cw++;
		if (cb == 0 && cw == 0)
		{
			int i = BDSIZE / 2, j=BDSIZE/2;
			cand.push_back({ i*BDSIZE + j, 0 });
			return 1;
		}
		if (cb == 1 && cw == 0)
		{
			for (int i = b_start; i < b_end; i++)
				for (int j = b_start; j < b_end; j++)
					if (std::max(abs(i - bx), abs(j - by)) <= 1 && AvailablePos(i*BDSIZE+j))
						cand.push_back({ i*BDSIZE + j, 0 });
			return 1;
		}
		if (cb == 1 && cw == 1)
		{
			for (int i = b_start; i < b_end; i++)
				for (int j = b_start; j < b_end; j++)
					if (std::max(abs(i - bx), abs(j - by)) <= 2 && AvailablePos(i*BDSIZE+j))
						cand.push_back({ i*BDSIZE + j, 0 });
			return 1;
		}
		return 0;
	}

	std::vector<int> vcf_root()
	{
		std::vector<int> result;
		if (who == Black) return result;
		for (int i = lowerRight; i < upperLeft; i++)
			if (cell[i].piece == Empty)
			{
				auto p = &cell[i];
				if (IsType(p, who, block4))
				{
					MakeMove(i);
					for (int j = lowerRight; j < upperLeft; j++)
						if (cell[j].piece == Empty && IsType(&cell[j], opp, win) && checkForbidden(j / BDSIZE, j % BDSIZE))
							result.push_back(i);
					DelMove(i);
				}
			}
		return result;
	}

	int GenerateMove(std::vector<int> &result)
	{
		checkForbiddens();
		std::vector<Point> cand;
		int val=0;
		if (c1(cand)) goto start;
		for (int i = b_start; i < b_end; i++)
			for (int j = b_start; j < b_end; j++)
				if (cell(i,j).piece==Empty)
				{
					val = EvaluateMove(&cell(i, j));
					cand.push_back({ i*BDSIZE + j, val });
				}
	start:
		std::sort(cand.begin(), cand.end(), [](const auto &a, const auto &b) {return a.val > b.val; });
		std::vector<int> moves;
		int ret = CutMoveList(moves, cand);
		auto vcf_move = vcf_root();
		if (vcf_move.size() && cand[0].val<2000) //my vcf
		{
			for (auto &i : vcf_move)
				result.push_back(Coord(i / BDSIZE - 4, i%BDSIZE - 4).p());
			return 1;
		}
		if (moves.empty())
		{
			for (auto &i : cand)
				if (AvailablePos(i.p))
					result.push_back(Coord(i.p / BDSIZE - 4, i.p%BDSIZE - 4).p());
		}
		else
		{
			for (auto &i : moves)
				if (AvailablePos(i))
					result.push_back(Coord(i / BDSIZE - 4, i%BDSIZE - 4).p());
			if (ret == 0) 
			{
				for (int i = lowerRight; i < upperLeft; i++)
					if (AvailablePos(i) && defendPoint[i])
						result.push_back(Coord(i / BDSIZE - 4, i%BDSIZE - 4).p());
			}
		}
		return ret;
	}

	void initPrior()
	{
		InitChessType();
		for (int i = 0; i < 4; i++)
			for (int j = 0; j < 8; j++)
				Range4[j * 4 + i] = range[j] * (i + 1);
	}
}
