#ifndef RELOCATION_H
#define RELOCATION_H

#include <iostream>
#include "Enums.h"

using namespace std;

class Relocation{

public:
	Relocation(string _section, int _offset, RelocationType _relT, string _sym);
	string section;
	int offset;
	RelocationType relT;
	string symbol;

};




#endif