#include "Section.h"

Section::Section(){

}
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
void Section::readAll(){
	map<int, string>::iterator it;

	for (it = content.begin(); it != content.end(); it++)
	{
		std::cout << it->first  // string (key)
			<< ':'
			<< it->second   // string's value 
			<< std::endl;
	}
}

void Section::addContent(Section& section){
	int pos = size;
	map<int, string>::iterator it;
	for (it = section.content.begin(); it != section.content.end(); it++){
		write1(pos, it->second);
		pos++;
		size++;
	}
}
void Section::addOffset(int valueAdd, int offset){
	string lower = content[offset];
	string higher = content[offset + 1];
	string number = higher + lower;
	int value = stoi(number, nullptr, 16);
	value += valueAdd;
	string newValue = std::bitset<16>(value).to_string();
	lower = newValue.substr(8, 8);
	string lowerB = BinaryToHex(lower);
	higher = newValue.substr(0, 8);
	string higherB = BinaryToHex(higher);
	auto pom = content.find(offset);
	pom->second = lowerB;
	pom = content.find(offset + 1);
	pom->second = higherB;

}


string Section::BinaryToHex(string bin){
	string first = bin.substr(0, 4);
	string ret = "";
	for (int i = 0; i < 2; i++){
		if (first == "0000") ret += "0";
		else if (first == "0001") ret += "1";
		else if (first == "0010") ret += "2";
		else if (first == "0011") ret += "3";
		else if (first == "0100") ret += "4";
		else if (first == "0101") ret += "5";
		else if (first == "0110") ret += "6";
		else if (first == "0111") ret += "7";
		else if (first == "1000") ret += "8";
		else if (first == "1001") ret += "9";
		else if (first == "1010") ret += "A";
		else if (first == "1011") ret += "B";
		else if (first == "1100") ret += "C";
		else if (first == "1101") ret += "D";
		else if (first == "1110") ret += "E";
		else if (first == "1111") ret += "F";
		first = bin.substr(4, 4);
	}
	return ret;

}
