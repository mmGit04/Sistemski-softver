#ifndef PROCESOR_H
#define PROCESOR_H

#include <thread>
#include <iostream>
#include <fstream>
#include <map>
#include <vector>
#include <bitset>
#include "Symbol.h"
#include "Section.h"
#include "Relocation.h"
#include "Instruction.h"
#include <queue>
#include <chrono>
#include <bitset>
#include <sstream>


using namespace std;
typedef std::bitset<8> byte;
typedef std::bitset<16> word;
struct PSW{
	int Z;
	int O;
	int C;
	int N;
	int T;
	int I;
	void reset(){
		Z = 0; O = 0; C = O; N = 0; T = 0; I = 0;
	}
	short value;
	void setValue(){
		std::stringstream ss;
		ss << I;
		ss << T;
		ss << N;
		ss << C;
		ss << O;
		ss << Z;
		string valueS = ss.str();
		value = std::stoi(valueS, nullptr, 2);//Ovo je int vrednost koja predstavlja ovu vrednost koju cine ovi biti psw.

	}
	void set(string value){

		I = value[0]-'0';
		T = value[1]-'0';
		N = value[2]-'0';
		C = value[3]-'0';
		O = value[4]-'0';
		Z = value[5]-'0';
	}

};
class Procesor{
public:
	Procesor(int code_start);
	void processing(ifstream &inputFile);
	map<int,int> memory;//Ovo je memorija.
	map<string, Symbol> symbols;//Ovde bi trebalo da se nalaze samo globalni simboli i nazivi sekcija.
	map<string, Section> sections; //Ovde je sadrzaj sekcija ,ime se pamti bez tacke.
	int PC;
	int SP;
	int code_start;//Ova se vrednost postavlja prilikom pokretanja programa.Odakle se ucitava citav program.
	int code_finish;
	bool incorectInstruction;
	vector<Relocation> realocations;//Takodje se ime sekcije na koju se odnosi pamti bez tacke.
	PSW psw;
	//int ivt[8];Ovo nigde nisam koristila.
	int reg[6];//Ovo su registri;
private:
	void init();
	int STACK_START;
	int STACK_END;
	void execute();
	void handleIncorect();
	void executeInstruction();
	void setIVT();
	static const int KEYBOARD_OUT = 0xFFFE;//ovo je registar izlazne periferije.
	static const int KEYBOARD_IN = 0xFFFC;//Ovo je registar ulazne periferije.
	void KeyboardInterruptRoutine();
	void TimerInterruptRoutine();
	void HandleInterrupts();
	bool endProgram;
	bool keyboardInt;
	void writeStatusProcesor();
	bool checkCondition();
	void handleInstruction();
	int getValueOperand(int operand);
	void putResult(int result);
	Instruction* currentInstruction;
	void readNextInstruction();
	void push(int value);
	int pop();
	void readFile(ifstream &inFile);
	void readSymTable(ifstream& inFile);
	void readSection(ifstream& inFile, string name);
	void readRelocation(ifstream& inFile, string section);
	void setOffsetSection();
	void setGlobalSymbolOffset();
	int getSectionOffset(SectionType sectionT);
	char keyboardBuffer;
	//thread* keyboardThread;
	void KeyboardThreadMethod();
	ScopeType getScopeType(string scope);
	TokenType getTokenType(string type);
	SectionType getSectionType(string section);
	void split_line(string line, const char* delim, queue<string>& tokens);
	chrono::time_point<chrono::system_clock> executionStart;
	chrono::time_point<chrono::system_clock> lastTimerExecution;

	string getSymbolName(int symbol);
	void loadSectionIntoMemory();
	void fixRealocations();

};

#endif