#ifndef SECTION_H
#define SECTION_H


#include <iostream>
#include <string>
#include <map>
#include <bitset>

using namespace std;

class Section{
public:
	Section();
	Section(string _name,int _size);
	string name;
	int size;
	map<int, string> content;
	void writeZero(int pos, int number);
	void write1(int pos, string value);
	void readAll();
	void addContent(Section& section);
	void addOffset(int valueAdd,int offset);
	~Section();
	string BinaryToHex(string bin);
};
#endif