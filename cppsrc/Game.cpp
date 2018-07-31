#include "Game.h"
#include "ConsolePrt.h"
#include "Search.h"
#include "Evaluation.h"
#include <vector>
#include <algorithm>
#include <ctime>
#include "GameData.h"
#include "PriorRenju.h"
using std::vector;

/*
Board emptygameboard = {
{
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,
0,0,0,2,0,0,1,2,0,0,0,0,0,0,0 ,
0,0,0,0,1,0,2,0,0,0,0,0,0,0,0 ,
0,0,0,0,1,2,2,2,0,1,0,0,0,0,0 ,
0,0,0,0,0,0,2,1,1,0,0,0,0,0,0 ,
0,0,0,0,0,1,2,2,0,0,0,0,0,0,0 ,
0,0,0,0,2,1,1,0,1,0,0,0,0,0,0 ,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0  } };
//*/
//*
Board emptygameboard = {
	{
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 ,
0,0,0,0,0,0,0,0,0,0,0,0,0,0,0  } };
//*/

void swap3(Board &board)
{
	for (int i = 0; i < BLSIZE; i++)
		if (board[i])
			board[i] = board[i] % 2 + 1;
}

int judgeWin(Board &board)
{
	for (int i = 0; i < BSIZE; i++)
		for (int j = 0; j < BSIZE; j++)
			if (board(i,j))
			{
				int col = board(i,j);
				for (int d = 0; d < 4; d++)
				{
					int dx = i, dy = j, cnt = 0;
					do
					{
						dx += cx[d]; dy += cy[d];
						cnt++;
					} while (inBorder({ dx,dy }) && board(dx, dy) == col);
					if (cnt >= 5)
						return col;
				}
			}
	return 0;
}

bool judgeAvailable(int pos) {
#ifdef RULE_RENJU
	return Prior::AvailablePos((pos/BSIZE+4)*BDSIZE + pos%BSIZE +4);
#else
	return 1;
#endif
}

void Game::make_move(Coord pos)
{
	if (pos.p() == BLSIZE) 
		gameboard.swap();
	else
		gameboard(pos) = nowcol;

	if (gamestep >= history.size())
		history.push_back(pos.p());
	else
		history[gamestep] = pos.p();

	nowcol = nowcol % 2 + 1;
	Prior::setbyBoard(gameboard);
	Prior::setPlayer(nowcol);
	gamestep++;
}

void Game::unmake_move()
{
	gamestep--;
	if (history[gamestep] == BLSIZE)
		gameboard.swap();
	else
		gameboard[history[gamestep]] = 0;

	nowcol = nowcol % 2 + 1;
	Prior::setbyBoard(gameboard);
	Prior::setPlayer(nowcol);
}

void Game::reset()
{
	gameboard = emptygameboard;
	gamestep = 0;
	nowcol = 1;
	history.clear();
}

void Game::printWinner(int z)
{
	std::cout << std::endl;
	if (z == 1) 
		std::cout << "Black win!";
	else if (z==2) 
		std::cout << "White win!";
	else 
		std::cout << "Draw!";
	std::cout << std::endl;
}

void Game::runGame(Player &player1, Player &player2)
{
	reset();
	if (show_mode == 1) print(gameboard);
	while (gameboard.count() < BLSIZE)
	{
		Coord c;
		if (nowcol == 1)
			c = player1.run(gameboard, nowcol);
		else
			c = player2.run(gameboard, nowcol);
		make_move(c);
		if (show_mode == 1) print(gameboard);
		else if (show_mode == 0) std::cout << c.format() << ' ';
		if (judgeWin(gameboard))
			break;
	}
	int winner = nowcol % 2 + 1;
	if (gameboard.count() == BLSIZE) winner = 0;
	printWinner(winner);
}

void Game::runGame_selfplay(Player &player)
{
	reset();
	vector<BoardWeight> policy;
	vector<float> winrate;
	if (show_mode==1) print(gameboard);
	while (gameboard.count() < BLSIZE)
	{
		Coord c = player.run(gameboard, nowcol);
		make_move(c);
		policy.push_back(player.getlastPolicy());
		winrate.push_back(player.searchlogger.winrate);
		if (show_mode == 1) print(gameboard);
		else if (show_mode == 0) std::cout << c.format() <<' ';
		if (judgeWin(gameboard))
			break;
	}
	int winner = nowcol % 2 + 1;
	if (gameboard.count() == BLSIZE) winner = 0;
	printWinner(winner);
	EposideTrainingData data(history, policy, winrate , winner);
	ofstream out(output_file, std::ios::app|std::ios::binary);
	data.writeByte(out);
}

int Game::getPlayerClick(Coord &posresult) {
	while (1) {
		auto mloc = getCurClick();
		auto sp = MlocToPloc(mloc);
		if (inBorder(sp) && !gameboard(sp))// && judgeAvailable(gameboard, sp(), nowcol)) 
		{
			posresult = sp;
			return 0;
		}
		//undo
		if (mloc.x == BSIZE + 1 && mloc.y <= 5) {
			if (gamestep > 1) {
				unmake_move();
				unmake_move();
				print(gameboard, nowcol, gamestep ? history[gamestep - 1] : -1);
			}
			return 1;
		}
		//redo
		if (mloc.x == BSIZE + 1 && mloc.y >= 8 && mloc.y <= 13) {
			if (gamestep <= history.size() - 2) {
				make_move(history[gamestep]);
				make_move(history[gamestep]);
				print(gameboard, nowcol, history[gamestep - 1]);
			}
			return 1;
		}
	}
}

void Game::saveSGF(int col) {
	ofstream file("result.txt", std::ios::app);
	file << "\n";
	file << "{[C5]";
	if (col == C_W)
		file << "[对手 B][阿尔法五子棋 W]";
	else
		file << "[阿尔法五子棋 B][对手 W]";

	if (nowcol == C_W)
		file << "[先手胜]";
	else
		file << "[后手胜]";
	time_t t = time(nullptr);
	tm *ft = localtime(&t);
	file << "[" << ft->tm_year + 1900 << '.' << ft->tm_mon << '.' << ft->tm_mday << ' ';
	file << " " << ft->tm_hour << ':' << ft->tm_min << "  合肥" << "]";
	file << "[2018 CCGC]";
	for (int i = 0; i < gamestep; i++) {
		Coord c(history[i]);
		file << ';';
		if (i & 1) file << 'W';
		else file << 'B';
		file << '(' << (char)(c.x + 'A')<<',' << c.y + 1 << ')';
	}
	file << '}' << '\n';
}

int getUserSwap() {
	gotoXY(0, BSIZE+2);
	printf("[交换]    [不交换]");
	while (1){

		auto mloc = getCurClick();
		if (mloc.x == BSIZE+2 && mloc.y <= 6)  
			return 1;
		else if (mloc.x==BSIZE+2 && mloc.y>=11 && mloc.y<=18)
			return 0;
	}
}

vector<int> getBlackPoints(Board &gameboard, int points) 
{
	clline(BSIZE + 2);
	gotoXY(0, BSIZE+2);
	printf("[确定]   要求打点数: %d", points);
	Board setted;
	setted.clear();
	int count=0;
	while (1) {
		auto mloc = getCurClick();
		auto sp = MlocToPloc(mloc);
		if (inBorder(sp) && !gameboard(sp)) 
		{
			gotoXY(sp.y * 2, sp.x);
			if (setted[sp.p()]) {
				count--;
				printf("┼");
			}
			else {
				count++;
				printf("■");
			}
			setted[sp.p()] = !setted[sp.p()];
		}
		if (mloc.x == BSIZE + 2 && mloc.y <= 6 && count==points)
			break;
	}
	std::vector<int> result;
	for (int i = 0; i < BLSIZE; i++)
		if (setted[i])
			result.push_back(i);
	return result;
}

void Game::runGameUser_Yuko(Player &player1, int col)
{
	int point = 0;
	//step 1,2,3
	if (col == 1) {
		reset();
		cls();
		print(gameboard);
		make_move(Coord::center);
		int straight = rand() % 2;
		if (straight) {
			make_move(Coord::center + Coord(1,0));
			int rd = rand() % 3;
			static const Coord u[3] = { {0,1} ,{1,1},{-1,1} };
			static const int points[3] = { 3,4,3 };
			point = points[rd];
			make_move(Coord::center + u[rd]);
		}
		else {
			make_move(Coord::center + Coord(1, 1));
			int rd = rand() % 4;
			static const Coord u[4] = { { 0,1 } ,{ 0,-1 },{ -1,1 },{0,2} };
			static const int points[4] = { 3,3,4,3 };
			point = points[rd];
			make_move(Coord::center + u[rd]);

		}
		print(gameboard, nowcol);
		clline(BSIZE+2);
		gotoXY(22, BSIZE+2);
		printf("打点数: %d", point);
		int sw = getUserSwap();
		if (sw) col = col % 2 + 1;
	}
	else {
		std::cout << "输入打点数:";
		std::cin >> point;
		reset();
		cls();
		print(gameboard);

		gotoXY(0, BSIZE + 2);
		printf("点击前三手");
		while (gamestep < 3) {
			int cmd;
			Coord c;
			do {
				cmd = getPlayerClick(c);
			} while (cmd);
			make_move(c);
			print(gameboard, nowcol, c.p());
		}
		int sw;
		if (point < 3 && (Coord(history[2]) - Coord(history[1])).lenth() < 5)
			sw = 1;
		else
			sw = 0;
		if (sw) col = col % 2 + 1;
	}
	ASSERT(gamestep == 3);

	//step 4,5
	if (col == nowcol) {
		clline(BSIZE + 2);
		gotoXY(0, BSIZE + 2);
		printf("○:Computer thinking...Do not click");
		Coord c = player1.run(gameboard, nowcol);
		make_move(c);
		print(gameboard, nowcol, c.p());

		vector<int> p = getBlackPoints(gameboard, point);

		clline(BSIZE + 2);
		gotoXY(0, BSIZE + 2);
		printf("■:Computer selecting...Do not click");
		int cp0 = player1.cfg_playouts;
		player1.cfg_playouts /= point;
		float mint=100; int minmove;
		for (auto move : p) {
			make_move(Coord(move));
			player1.run(gameboard, nowcol);
			if (player1.searchlogger.winrate < mint) {
				mint = player1.searchlogger.winrate;
				minmove = move;
			}
			unmake_move();
		}
		player1.cfg_playouts = cp0;
		make_move(minmove);
		print(gameboard, nowcol, minmove);
	}
	else 
	{
		clline(BSIZE + 2);
		gotoXY(0, BSIZE + 2);
		printf("○:Your turn");
		Coord c;
		int cmd;
		do {
			cmd = getPlayerClick(c);
		} while (cmd);
		make_move(c);
		print(gameboard, nowcol, c.p());

		clline(BSIZE + 2);
		gotoXY(0, BSIZE + 2);
		printf("■:Computer selecting...Do not click");
		c = player1.run(gameboard, nowcol);

		Board selected;
		selected.clear();
		for (int i = 0; i < point; i++) {
			gotoXY(0, BSIZE + i + 2);
			int po; float wr; int move;
			std::tie(po, wr, move) = player1.searchlogger.playout_rate_move[i];
			c = Coord(move);
			std::cout << c.toString() << ' ';
			printf("%4d ", po);
			printf("%.3f", wr);
			gotoXY(c.y * 2, c.x);
			printf("■");
			selected[move] = 1;
		}
		while (1) {
			auto mloc = getCurClick();
			c = MlocToPloc(mloc);
			if (inBorder(c) && selected(c))
				break;
		}
		make_move(c);
		print(gameboard, nowcol, c.p());
	}

	//original game
	while (gamestep < BLSIZE)
	{
		Coord c;
		clline(BSIZE + 2);
		gotoXY(0, BSIZE + 2);
		if (nowcol == C_W) printf("○:");
		else printf("●:");
		if (nowcol == col)
		{
			printf("Compter thinking...Do not click");
			c = player1.run(gameboard, nowcol);
			clline(BSIZE + 3);
			gotoXY(0, BSIZE + 3);
			printf("(%d,%d):%f", c.x, c.y, player1.searchlogger.winrate);
		}
		else
		{
			printf("It's your turn                 ");
			int cmd;
			do {
				cmd = getPlayerClick(c);
			} while (cmd);
		}
		make_move(c);
		print(gameboard, nowcol, c.p());
		if (judgeWin(gameboard))
			break;
	}
	int winner = nowcol % 2 + 1;
	if (gamestep == BLSIZE) winner = 0;
	printWinner(winner);
	saveSGF(col);
	system("pause");
}

void Game::runGameUser(Player &player1, int col)
{
#ifdef RULE_RENJU
	if (cfg_special_rule == 1)
	{ 
		runGameUser_Yuko(player1, col);
		return;
	}
#endif
	reset();
	cls();
	print(gameboard);
	while (gamestep < BLSIZE)
	{
		Coord c;
		gotoXY(0, BSIZE + 2);
		if (nowcol == C_W) printf("○:");
		else printf("●:");
		if (nowcol == col)
		{
			printf("Compter thinking...Do not click");
			c = player1.run(gameboard, nowcol);
			gotoXY(0, BSIZE + 3);
			printf("(%d,%d):%f", c.x, c.y, player1.searchlogger.winrate);
		}
		else
		{
			printf("It's your turn                 ");
			int cmd;
			do {
				cmd = getPlayerClick(c);
			} while (cmd);
		}
		make_move(c);
		print(gameboard, nowcol, c.p());
		if (judgeWin(gameboard))
			break;
	}
	int winner = nowcol % 2 + 1;
	if (gamestep == BLSIZE) winner = 0;
	printWinner(winner);
	saveSGF(col);
	system("pause");
}

void Game::runGameUser2()
{
	reset();
	print(gameboard);
	while (gameboard.count() < BLSIZE)
	{
		Coord c;
		c = getPlayerPos(gameboard);
		make_move(c);
		print(gameboard);
		if (judgeWin(gameboard))
			break;
	}
	int winner = nowcol % 2 + 1;
	if (gameboard.count() == BLSIZE) winner = 0;
	printWinner(winner);
}

void Game::selfplay(Player &player)
{
	for (int i = 0; i < selfplay_count; i++)
	{
		std::cout << "game " << i << '\n';
		runGame_selfplay(player);
	}
}

void Game::match(Player &player1, Player &player2)
{
	int play_counts = 100;
	for (int i = 0; i < play_counts; i++)
	{
		std::cout << "game " << i << '\n';
		runGame(player1, player2);
	}
}

void Game::runRecord(const vector<int> &moves)
{
	reset();
	if (show_mode == 1) print(gameboard);
	for (auto move : moves)
	{
		Coord c(move);
		make_move(c);
		if (show_mode == 1) print(gameboard);
		else if (show_mode == 0) std::cout << c.format() << ' ';
	}
	int winner = nowcol % 2 + 1;
	if (gamestep == BLSIZE) winner = 0;
	printWinner(winner);
}

void Game::runFromFile(string filename)
{
	DataSeries<EposideData> datas;
	datas.readString(filename);
	for (auto &data : datas.datas)
	{
		runRecord(data.moves);
	}
}


void Game::runGomocup(Player &player)
{
	using std::cin;
	using std::cout;
	using std::endl;
	string command;
	Coord input, best;
	char dot;
	while (1) 
	{
		cin >> command;
		std::transform(command.begin(), command.end(), command.begin(), ::toupper);

		if (command == "START") {
			int size; cin >> size;
			if (size != BSIZE )
				cout << "ERROR only support "<<BSIZE<<"*"<<BSIZE<<" board" << endl;
			else
			{
				reset();
				auto time = std::time(NULL);
				debug_s<<"Engine loaded, local time:"<<std::ctime(&time)<<'\n';
				cout << "OK" << endl;
			}
		}
		else if (command == "RESTART") {
			reset();
			cout << "OK" << endl;
		}
		else if (command == "TAKEBACK") {
			unmake_move();
			cout << "OK" << endl;
		}
		else if (command == "BEGIN") {
			best = player.run(gameboard, nowcol);
			make_move(best);
			cout << best.x << "," << best.y << endl;
		}
		else if (command == "TURN") {
			cin >> input.x >> dot >> input.y;
			if (!inBorder(input) && input.p()!=BLSIZE || gameboard(input))
				cout << "ERROR invalid move" << endl;
			else {
				make_move(input);
				best = player.run(gameboard, nowcol);
				make_move(best);
				cout << best.x << "," << best.y << endl;
			}
		}
		else if (command == "BOARD") {
			int c;
			Coord m;
			stringstream ss;
			reset();

			cin >> command;
			std::transform(command.begin(), command.end(), command.begin(), ::toupper);
			while (command != "DONE") {
				ss.clear();
				ss << command;
				ss >> m.x >> dot >> m.y >> dot >> c;
				if (!inBorder(m) && m.p() != BLSIZE || gameboard(m))
					cout << "ERROR invalid move" << endl;
				else
					make_move(m);
				cin >> command;
				std::transform(command.begin(), command.end(), command.begin(), ::toupper);
			}
			best = player.run(gameboard, nowcol);
			make_move(best);
			cout << best.x<< "," << best.y<< endl;
		}
		else if (command == "INFO") {
			int value;
			string key;
			cin >> key;
			std::transform(key.begin(), key.end(), key.begin(), ::toupper);

			if (key == "TIMEOUT_TURN") {
				cin >> value;
				timeout_turn = value;
				cfg_timelim = true;
			}
			else if (key == "TIMEOUT_MATCH") {
				cin >> value;
				//TODO
			}
			else if (key == "TIME_LEFT") {
				cin >> value;
				timeout_left = value;
				cfg_timelim = true;
				//TODO
			}
			else if (key == "MAX_MEMORY") {
				cin >> value;
				// TODO
			}
			else if (key == "GAME_TYPE") {
				cin >> value;
				// TODO
			}
			else if (key == "RULE") {
				cin >> value;
				// TODO
			}
			else if (key == "FOLDER") {
				string t;
				cin >> t;
			}
		}
		else if (command == "END")
			exit(0);
	}
	cout << "If you see this information, you should download a gomoku manager to run this program. Or you can modify Gmk0.json to change the mode. ";
}
