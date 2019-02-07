#ifndef COMPILER_H
#define COMPILER_H

#include <iostream>
#include <fstream>
#include <bitset>
#include "Enums.h"
#include "Symbol.h"
#include "Section.h"
#include "Relocation.h"
#include <string>
#include <vector>
#include <queue>
#include <regex>
#include <map>
using namespace std;


class Compiler{
public:

	int locationCounter;
	vector<vector<string>> inputAssembly;
	vector<Relocation> relocations;
	map<string, Symbol> symTable;
	map<string, Section> sections;
	static map<AddressingType, string> codeAddresingType;
	bool flagJMP;
	static bool helper;
	static string helper1;
	AddressingType resolveAddresingTypeOperand(string firstT);
	void FirstRun(ifstream& inputFile);
	void SecondRun();
	TokenType parseToken(string token);
	void parseInput(ifstream& inputFile);
	void split_line(string line, const char* delim, vector<string>& tokens);
	void writeAssemblyInput(ofstream& output);
	void writeSymTab(ofstream& output);
	void addNewSymbol(string labelName, int locationCounter, TokenType token_type, ScopeType scope_type, SectionType curr_section, bool def);
	void directiveFirstRun(string directive, queue<string> &tokens);
	void updateDataSection(string _currTokenS, SectionType& _currSection);
	static map<string, int> instructionNumOperands;
	void  instructionFirstRun(string instr, queue<string> &tokens);
	string getSectionName(SectionType section);
	string getScopeName(ScopeType scope);
	string getTypeName(TokenType);
	int NumOperandsDirective(queue<string> &tokens);
	void secondPassGlobal(queue<string>& tokens,SectionType currSec);
	void secondPassDirective(string currTokenS,queue<string>& tokens, string currSec);
	void secondPassInstruction(queue<string>& tokens, string currToken,SectionType _currSec,string currSec);
	int getValueOperand(string operand, string _currSecS="", SectionType _currSec=START, bool& needAdditionalBytes=helper, string& reg=helper1);
	string getNameSection(SectionType sec);
	int setAbsRelocation(string symbol,string _currSecS);
	string decToBinString(string dec);
	int setPCRelRelocation(string symS, string _currS);
	void insertIntoSection(string _currSectionS, string content, bool needAdditionalBytes1, bool needAdditionalBytes2, string addBytesString);
	void handleInstructionTwoOperands(queue<string>& tokens, string content, string _currSecS, SectionType _currSec);
	void handleInstructionOneOperand(queue<string>& tokens, string content, string _currSecS, SectionType _currSec);
	void handleInstructionZeroOperand(queue<string>& tokens, string content, string _currSecS, SectionType _currSec);
	string BinaryToHex(string bin);
	void writeSections(ofstream& output);
};


#endif
