#ifndef INSTRUCTION_H
#define INSTRUCTION_H

#include "Enums.h"
#include <iostream>
#include <bitset>
#include <map>
using namespace std;


class Instruction{

public:
	static map<InstuctionName, int> instructionNumOperands;
	Instruction(string ins);
	InstructionCond cond;
	InstuctionName name;
	AddressingType firstOpAdd;
	AddressingType secondOpAdd;
	int addBytes;
	int firstBit;//Ovo su ona 3 dodatna bita.
	int secondBit;
	bool longIns;//Odnosno da li je duzina 4 BAJTA;
	AddressingType getAdressingType(string val);
	InstructionCond getInstrutionCond(string val);
	InstuctionName getInstructionName(string val);
};


#endif
