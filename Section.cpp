#include "Section.h"


Section::Section(string _name, int _size){
	size = _size;
	name = _name;
}
void Section::writeZero(int pos, int number){
	for (int i = 0; i < number; i++){
		std::pair<int, string> value(pos+i,"00");
		content.insert(value);
	}
}

void Section::write1(int pos, string value){
	std::pair<int, string> v(pos,value);
	content.insert(v);
}
Section::~Section(){
}