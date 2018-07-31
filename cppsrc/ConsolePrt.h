#pragma once
#include "Common.h"
#include "Board.h"

#define WIN_CON

#ifdef WIN_CON

void minit();

void mexit();

Coord getCurClick(); 
Coord getPlayerPos(Board &gameboard);

void cls();
void clline(int line);
void gotoXY(short x, short y);

#endif

Coord MlocToPloc(const Coord &p);
void print(Board &gameboard, int col = 0, int lastmove = -1);
