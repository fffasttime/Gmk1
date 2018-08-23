#include "Common.h"

int cfg_seed;
bool cfg_swap3 = true;
bool cfg_timelim = false;
int timeout_turn;
int timeout_left;
int cfg_loglevel;
int cfg_special_rule; 
string exepath;

const Coord Coord::center = Coord(BSIZE / 2, BSIZE / 2);

stringstream debug_s;
ofstream filelog;

void logOpen(string filename)
{
	filelog.open(filename, std::ios::app);
}

void logRefrsh()
{
	string s;
	if (filelog.is_open())
	{
		while (!debug_s.eof())
		{
			getline(debug_s, s);
			filelog << s << std::endl;
		} 
	}
	debug_s.clear();
}
