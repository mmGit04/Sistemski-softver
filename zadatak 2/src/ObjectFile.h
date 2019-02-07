#ifndef OBJECTFILE_H
#define OBJECTFILE_H
#include "Section.h"
#include "Symbol.h"
#include "Relocation.h"
#include "Enums.h"
#include <queue>
#include <map>


class ObjectFile{
public:
	void readFile(string inputFile);
	map<string, Symbol> symbols;
	map<string, Section> sections;
	vector<Relocation> relocations;
private:
	void readSymTable(ifstream& inFile);
	void split_line(string line, const char* delim, queue<string>& tokens);
	SectionType getSectionType(string section);
	TokenType getTokenType(string type);
	ScopeType getScopeType(string scope);
	void readRelocation(ifstream& inFile,string section);
	string getSymbolName(int symbol);
	void readSection(ifstream& inFile, string name);
	
};
#endif