#pragma once
#include "Board.h"
#include "Common.h"
#include "Search.h"

int judgeWin(Board &board); 
bool judgeAvailable(int pos);

class Game
{
private:
	int gamestep;
	int nowcol;
	Board gameboard;
	vector<int> history;
	void make_move(Coord pos);
	void unmake_move();
	void reset();
	void printWinner(int z);
	int getPlayerClick(Coord & posresult);
public:
	void runGame(Player &player1, Player &player2);
	void runGameUser(Player &player1, int col);
	void runGameUser_Yuko(Player &player1, int col);
	void runGameUser2();
	void runGame_selfplay(Player &player);
	void saveSGF(int col);
	void runRecord(const std::vector<int> &moves);
	void runFromFile(string filename);
	void runGomocup(Player & player);
	void selfplay(Player &player);
	void match(Player & player1, Player & player2);
	string output_file = "selfplaydata.txt";
	int show_mode = 0;
	int selfplay_count = 2048;
};
