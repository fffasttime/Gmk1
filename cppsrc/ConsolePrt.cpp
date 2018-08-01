#include "ConsolePrt.h"
#include "Game.h"

#ifdef WIN_CON
#include <Windows.h>

HANDLE hOut, hIn;

void minit()
{
	HANDLE consolehwnd;
	consolehwnd = GetStdHandle(STD_OUTPUT_HANDLE);
	//SetConsoleTextAttribute(consolehwnd, 127 + 128 - 15);
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	hIn = GetStdHandle(STD_INPUT_HANDLE);
	//SMALL_RECT rc={0,0,winsize.X,winsize.Y};
	//SetConsoleScreenBufferSize(hOut, winsize);
	//SetConsoleWindowInfo(hOut, TRUE, &rc);
}

void mexit()
{
	CloseHandle(hOut);
	CloseHandle(hIn);
}

Coord getCurClick()
{
	//CONSOLE_SCREEN_BUFFER_INFO bInfo;
	INPUT_RECORD    mouseRec;
	DWORD           res;
	COORD           crPos;
	while (1)
	{
		ReadConsoleInput(hIn, &mouseRec, 1, &res);
		if (mouseRec.EventType == MOUSE_EVENT)
		{
			crPos = mouseRec.Event.MouseEvent.dwMousePosition;
			switch (mouseRec.Event.MouseEvent.dwButtonState)
			{
			case FROM_LEFT_1ST_BUTTON_PRESSED:
				return{ crPos.Y, crPos.X };
			}
		}
	}
}

void gotoXY(short x, short y)
{
	SetConsoleCursorPosition(hOut, { x, y });
}

#endif

Coord MlocToPloc(const Coord &p)
{
	if (p.y >= 2 * BSIZE || p.x >= BSIZE) return{ -1,-1 };
	return{ p.x,p.y / 2 };
}

Coord getPlayerPos(Board &gameboard)
{
	auto sp = MlocToPloc(getCurClick());
	while (!inBorder(sp) || gameboard(sp))
	{
		sp = MlocToPloc(getCurClick());
	}
	return sp;
}

void cls() {
	gotoXY(0, 0);
	for (int i = 0; i < 5000; i++)
		putchar(' ');
}
void clline(int line) {
	gotoXY(0, line);
	for (int i = 0; i < 60; i++)
		putchar(' ');
}

void print(Board &gameboard, int col, int lastmove)
{
	gotoXY(0, 0);
	for (int i = 0; i<BSIZE; i++)
	{
		for (int j = 0; j<BSIZE; j++)
		{
			if (gameboard(i, j) == C_W)
				printf("¡ð");
			else if (gameboard(i, j) == C_B)
				printf("¡ñ");
			else if (gameboard(i, j) == C_E && (col == 0 ) || judgeAvailable(Coord(i, j).p()))
				printf("©à");
			else
				printf("¡Á");
		}/*
		 for (int j = 0; j<BSIZE; j++)
		 {
		 if (gameboard(i, j) == C_E)
		 fout << "©à";
		 else if (gameboard(i, j) == C_W)
		 fout << "¡ð";
		 else
		 fout << "¡ñ";
		 }*/
		printf("\n");
		//fout << "\n";
	}
	if (col && lastmove != -1) {
		gotoXY(lastmove % BSIZE * 2, lastmove / BSIZE);
		if (col == C_W)
			printf("¡ø");
		else
			printf("¡÷");
	}
	printf("\n");
}
