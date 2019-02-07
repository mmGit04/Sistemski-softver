#ifndef SECTION_H
#define SECTION_H


#include <iostream>
#include <string>
#include <map>

using namespace std;

class Section{
public:
	Section(string _name,int _size);
	string name;
	int size;
	map<int, string> content;
	void writeZero(int pos, int number);
	void write1(int pos, string value);
	~Section();
};
#endif