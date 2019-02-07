#include "Symbol.h"

 Symbol::Symbol(string _name, int _offset, TokenType _type, ScopeType _scope, SectionType _section, int _size, int _ind){
	 ind = _ind;
	 name = _name;
	 offset = _offset;
	 type = _type;
	 scope = _scope;
	 section = _section;
	 size = _size;
 }
 Symbol::Symbol(){}
Symbol::~Symbol(){}
	