#ifndef SYMBOL_H
#define SYMBOL_H


#include <iostream>
#include "Enums.h"

using namespace std;

class Symbol{
public:
	static int id;
	int ind;
	string name;
	int offset;
	TokenType type;
	ScopeType scope;
	SectionType section;
	bool defined;
	int size;
	Symbol(string _name, int _offset, TokenType _type, ScopeType _scope, SectionType _section, bool _defined,int _size);
	~Symbol();
};

#endif