#include "Symbol.h"

 int Symbol::id = 0;
Symbol::Symbol(string _name, int _offset, TokenType _type, ScopeType _scope, SectionType _section, bool _defined,int _size){
	ind=id++;
	name = _name;
	offset = _offset;
	type = _type;
	scope = _scope;
	section = _section;
	defined = _defined;
	size = _size;
}

Symbol::~Symbol(){}
	