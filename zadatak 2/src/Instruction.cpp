#include "Instruction.h"
map<InstuctionName, int> Instruction::instructionNumOperands =
{
	{ InstuctionName::ADD, 2 },
	{ InstuctionName::SUB, 2 },
	{ InstuctionName::MUL, 2 },
	{ InstuctionName::DIV, 2 },
	{ InstuctionName::CMP, 2 },
	{ InstuctionName::AND, 2 },
	{ InstuctionName::OR , 2 },
	{ InstuctionName::NOT, 2 },
	{ InstuctionName::TEST, 2 },
	{ InstuctionName::PUSH, 1 },
	{ InstuctionName::POP, 1 },
	{ InstuctionName::IRET, 0 },
	{ InstuctionName::CALL, 1 },
	{ InstuctionName::MOV, 2 },
	{ InstuctionName::SHL, 2 },
	{ InstuctionName::SHR, 2 },
	{ InstuctionName::RET, 0 },
	{ InstuctionName::JMP, 1 }


};
Instruction::Instruction(string in){
	cond = getInstrutionCond(in.substr(0, 2));
	name = getInstructionName(in.substr(2, 4));
	firstOpAdd = getAdressingType(in.substr(6, 2));
	firstBit = std::stoi(in.substr(8,3), nullptr, 2);
	secondOpAdd = getAdressingType(in.substr(11, 2));
	secondBit = std::stoi(in.substr(13, 3), nullptr, 2);
	int numOperands = instructionNumOperands[name];
	if (numOperands == 0){
		longIns = false;
		addBytes = 0;
		return;
	}
	if (numOperands == 1){
		if (firstOpAdd == AddressingType::REG_DIR){
			longIns = false;
			addBytes = 0;
		}
		else{
			longIns = true;
			string value = in.substr(24, 8) + in.substr(16, 8);
			addBytes = std::stoi(value, nullptr, 2);
		}
		return;
	}
	if (numOperands == 2){
		if (firstOpAdd == AddressingType::REG_DIR && secondOpAdd == AddressingType::REG_DIR){
			longIns = false;
			addBytes = 0;
		}
		else{
			longIns = true;
			string value = in.substr(24, 8) + in.substr(16, 8);
			addBytes = std::stoi(value, nullptr, 2);
		}
		return;
	}

	return;
		

}

AddressingType Instruction::getAdressingType(string val){
	if (val == "00")
		return AddressingType::IMM;
	if (val == "01")
		return AddressingType::REG_DIR;
	if (val == "10")
		return AddressingType::MEM;
	if (val == "11")
		return AddressingType::REG_IND_POM;
	cout << "Error: " << "wrong addresing type." << endl;
	return AddressingType::IMM;
}
InstructionCond Instruction::getInstrutionCond(string val){
	if (val == "00")
		return InstructionCond::EQ;
	if (val == "01")
		return InstructionCond::NE;
	if (val == "10")
		return InstructionCond::GT;
	if (val == "11")
		return InstructionCond::AL;
	cout << "Error: " << "wrong instruction condition." << endl;
	return InstructionCond::EQ;

}
InstuctionName Instruction::getInstructionName(string val){
	if (val == "0000")
		return InstuctionName::ADD;
	if (val == "0001")
		return InstuctionName::SUB;
	if (val == "0010")
		return InstuctionName::MUL;
	if (val == "0011")
		return InstuctionName::DIV;
	if (val == "0100")
		return InstuctionName::CMP;
	if (val == "0101")
		return InstuctionName::AND;
	if (val == "0110")
		return InstuctionName::OR;
	if (val == "0111")
		return InstuctionName::NOT;
	if (val == "1000")
		return InstuctionName::TEST;
	if (val == "1001")
		return InstuctionName::PUSH;
	if (val == "1010")
		return InstuctionName::POP;
	if (val == "1011")
		return InstuctionName::CALL;
	if (val == "1100")
		return InstuctionName::IRET;
	if (val == "1101")
		return InstuctionName::MOV;
	if (val == "1110")
		return InstuctionName::SHL;
	if (val == "1111")
		return InstuctionName::SHR;
	cout << "Error: " << "wrong instruction code" << endl;
	return InstuctionName::ADD;
}