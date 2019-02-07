#ifndef LINKER_H
#define LINKER_H
#include <iostream>
#include <fstream>
#include <vector>
#include "Section.h"
#include "Symbol.h"
#include "ObjectFile.h"
using namespace std;

class Linker{

public: 
	void Link(vector<string> &inputFiles);
	void linkFile(string inputFile);
	void writeSymTab(ofstream& output);
	void writeSections(ofstream& output);
private:
	map<string, Section> sections;
	map<string, Symbol> symbols;
	vector<Relocation> relocations;
	int addSection(Symbol& symbol, Section& section);
	void addSymbol(Symbol& symbol);
	void checkLink();
	void setNumber();
	
	string getScopeName(ScopeType scope);
	string getSectionName(SectionType section);
	string getTypeName(TokenType type);


};
#endif