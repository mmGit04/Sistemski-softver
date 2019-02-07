#include "Relocation.h"


Relocation::Relocation(string _section, int _offset, RelocationType _relT, string _sym){
	section = _section;
	offset = _offset;
	relT = _relT;
	symbol = _sym;
}