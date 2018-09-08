#pragma once
#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <cassert>
#include <vector>
using std::ifstream;
using std::ofstream;
using std::stringstream;
using std::string;

#define BSIZE (15)
#define BLSIZE (BSIZE * BSIZE)
#define FLOAT_INF (1e10)

//#define RULE_RENJU
#define RULE_GOMOKU

#ifdef RULE_RENJU
#define Prior PriorRenju
#endif
#ifdef RULE_GOMOKU
#define Prior PriorGomoku
#endif

extern int cfg_seed;
extern bool cfg_swap3;
extern bool cfg_timelim;
extern int timeout_turn;
extern int timeout_left;
extern int cfg_loglevel;
extern int cfg_special_rule; 
extern string exepath;

extern stringstream debug_s;

void logOpen(string filename);
void logRefrsh();

struct Coord
{
	int x, y;
	Coord() = default;
	Coord(int _x, int _y) :x(_x), y(_y) {}
	Coord(int n) :x(n / BSIZE), y(n%BSIZE) {}
	int p() { return x*BSIZE + y; }

	//Don't use these operations in high frequency calculation
	Coord operator+(const Coord &v)const {
		return Coord(x + v.x, y + v.y);
	}
	Coord operator-(const Coord &v)const {
		return Coord(x - v.x, y - v.y);
	}
	int lenth() {
		return abs(x) + abs(y);
	}
	string toString(){
		stringstream ss;
		ss << '(';
		if (x == BSIZE)
			ss << "sw";
		else
			ss << x <<','<< y;
		ss << ')';
		string s; ss >> s;
		return s;
	}
	string format() 
	{ 
		stringstream ss; 
		if (x == BSIZE)
			ss << "sw";
		else
			ss << (char)('A' + x) << y + 1; 
		string s; ss >> s; 
		return s; 
	}
	static const Coord center;
};

const int C_E = 0, C_B = 1, C_W = 2;

//#define ASSERT(expr)
#define ASSERT(expr) assert(expr)

#define POLICY_MAX_EXPANDS BLSIZE
