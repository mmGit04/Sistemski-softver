#ifndef SYMBOL_H
#define SYMBOL_H


#include <iostream>
#include "Enums.h"

using namespace std;

class Symbol{
public:
	int ind;
	string name;
	int offset;
	TokenType type;
	ScopeType scope;
	SectionType section;

	int size;
	Symbol();
	Symbol(string _name, int _offset, TokenType _type, ScopeType _scope, SectionType _section, int _size, int _ind);
	~Symbol();
};

#endif