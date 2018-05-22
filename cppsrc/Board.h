#pragma once
#include "Common.h"

bool inBorder(Coord a);
bool inBorder(int x, int y);

template<typename T, int size = BSIZE>
struct BoardArray
{
	T m[size * size];
	T& operator()(Coord o)
	{
		return m[o.p()];
	}
	T& operator()(int x, int y)
	{
		return m[x * size + y];
	}
	T& operator[](int p)
	{
		return m[p];
	}
	void swap()
	{
		for (int i = 0; i < size * size; i++)
			if (m[i])
				m[i] = m[i] % 2 + 1;
	}
	void debug() const;
	void clear();
	int count() const;
	int countv(int col) const;
};


typedef BoardArray<int> Board;
typedef BoardArray<float> BoardWeight;

string board2showString(Board &board, bool withaxis);
extern Board transform_table[16];

void initTransformTable();

template<typename T>
void boardTransform(int mode, BoardArray<T> &board)
{
	ASSERT(mode >= 0 && mode < 16);
	auto copy = board;
	for (int i = 0; i < BLSIZE; i++)
		board[i] = copy[transform_table[mode][i]];
}

int posTransform(int mode, int p);

void initZobristTable();

struct BoardHasher
{
	unsigned long long num;
	BoardHasher(Board &board);

	void update(int pos, int old, int target);
	unsigned long long operator()() { return num; }
};

const int cx[8] = { -1,-1,-1,0,0,1,1,1 };
const int cy[8] = { -1,0,1,-1,1,-1,0,1 };
