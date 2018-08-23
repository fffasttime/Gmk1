#include "Board.h"
#include <random>
#include <iomanip>

bool inBorder(Coord a)
{
	return a.x >= 0 && a.y >= 0 && a.x < BSIZE && a.y < BSIZE;
}

bool inBorder(int x, int y)
{
	return x >= 0 && y >= 0 && x < BSIZE && y < BSIZE;
}

template <>
void BoardArray<int>::clear()
{
	memset(m, 0, sizeof(m));
}
template <>
void BoardArray<int, BSIZE + 8>::clear()
{
	memset(m, 0, sizeof(m));
}

template <>
int BoardArray<int>::count() const
{
	int cnt = 0;
	for (int i = 0; i < BLSIZE; i++)
		if (m[i])
			cnt++;
	return cnt;
}

template <>
int BoardArray<int>::countv(int col) const
{
	int cnt = 0;
	for (int i = 0; i < BLSIZE; i++)
		if (m[i] == col)
			cnt++;
	return cnt;
}

template <>
void BoardArray<float>::clear()
{
	memset(m, 0, sizeof(m));
}

template <>
void BoardArray<int>::debug() const
{
	for (int i = 0; i<BSIZE; i++)
	{
		for (int j = 0; j < BSIZE; j++)
			if (m[i*BSIZE + j])
				std::cout << " " << m[i*BSIZE + j];
			else
				std::cout << "  ";
		std::cout << '\n';
	}
	std::cout << '\n';
}

string board2showString(Board &board, bool withaxis) 
{
	stringstream ss;
	if (withaxis)
	{
		ss << "  ";
		for (int i = 0; i < BSIZE; i++)
			ss << " " << (char)('A' + i);
		ss << std::endl;
	}
	for (int i = 0; i < BSIZE; i++) 
	{
		if (withaxis)
			ss << std::setw(2) <<i + 1 << ' ';
		for (int j = 0; j < BSIZE; j++)
			if (board(j, i) == 0)
				ss << "¡¤";
			else if (board(j, i) == 1)
				ss << "¡ñ";
			else
				ss << "¡ð";
		ss << std::endl;
	}
	return ss.str();
}

Board transform_table[16];

void initTransformTable()
{
	for (int i = 0; i < BLSIZE; i++)
		transform_table[0][i] = i;
	for (int i = 0; i < BSIZE; i++)
		for (int j = 0; j < BSIZE; j++)
			transform_table[1][i*BSIZE + j] = j*BSIZE + BSIZE - i - 1;
	for (int i = 0; i < BSIZE; i++)
		for (int j = 0; j < BSIZE; j++)
			transform_table[4][i*BSIZE + j] = i*BSIZE + BSIZE - j - 1;
	transform_table[2] = transform_table[1];
	boardTransform(1, transform_table[2]);
	transform_table[3] = transform_table[2];
	boardTransform(1, transform_table[3]);
	transform_table[5] = transform_table[4];
	boardTransform(1, transform_table[5]);
	transform_table[6] = transform_table[5];
	boardTransform(1, transform_table[6]);
	transform_table[7] = transform_table[6];
	boardTransform(1, transform_table[7]);

	for (int i = 0; i < 8; i++)
		for (int j = 0; j < BLSIZE; j++)
			transform_table[i + 8][transform_table[i][j]] = j;
}

int posTransform(int mode, int p)
{
	return transform_table[mode][p];
}

BoardArray<unsigned long long> zobirst_table[3];

void initZobristTable()
{
	std::mt19937 e(cfg_seed);
	std::uniform_int<unsigned long long> u(0, 1ull<<63);
	for (int i = 0; i < 3; i++)
		for (int j = 0; j < BLSIZE; j++)
		{
			zobirst_table[i][j] = u(e);
		}
}

BoardHasher::BoardHasher(Board &board)
{
	num = 0;
	for (int i = 0; i < BLSIZE; i++)
		num ^= zobirst_table[board[i]][i];
}

void BoardHasher::update(int pos, int old, int target)
{
	num ^= zobirst_table[old][pos];
	num ^= zobirst_table[target][pos];
}


std::vector<BoardAction> openingsBook;
bool cfg_use_openings = false;

int find_in_openingsBook(const Board &board) {
	int i = 0;
	for (auto &b : openingsBook) {
		if (memcmp(&board, &b.board, sizeof(board))==0)
			return i;
		i++;
	}
	return -1;
}

int load_openingsBook() {
	std::ifstream fin(exepath+"/openings.txt");
	int count = 0;
	if (fin.is_open()) {
		while (!fin.eof()){
			Board board; board.clear(); int nowcol = C_B;
			string line;
			std::getline(fin, line);
			std::istringstream ss(line);
			std::vector<int> numbers;
			while (!ss.eof()){
				string s;
				std::getline(ss, s, ',');
				std::istringstream ns(s);
				int num; ns >> num;
				numbers.push_back(num);
			}
			if (numbers.size() % 2) {
				std::cout << "error: Bad openings book\n";
				exit(1);
			}
			if (numbers.empty()) continue;

			for (size_t i = 0; i < numbers.size() / 2; i++) {
				auto p = Coord(numbers[i * 2], numbers[i * 2 + 1]) + Coord::center;
				if (!inBorder(p)) {
					std::cout << "error: Bad openings book\n";
					exit(1);
				}
				int x = find_in_openingsBook(board);
				if (x == -1)
					openingsBook.push_back({ board, std::vector<int>{p.p()} });
				else
					openingsBook[x].moves.push_back(p.p());
				board[p.p()] = nowcol;
				nowcol = nowcol % 2 + 1;
			}
			count++;
		}
	}
	if (!openingsBook.empty())
		cfg_use_openings = 1;
	if (cfg_loglevel) {
		debug_s << count << " openings loaded\n";
		logRefrsh();
	}
	return cfg_use_openings;
}

