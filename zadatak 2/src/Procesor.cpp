#include "Procesor.h"


Procesor::Procesor(int code_start){

	this->code_start = code_start;
	if (code_start < 16) throw runtime_error("Code starts in IVT table!");
	STACK_START = stoi("FF80", nullptr, 16);;
}
void Procesor::processing(ifstream &inFile){
	cout << "==========Proccesing=============" << endl;
	readFile(inFile);//Prvo idemo citanje svih podataka iz ulaznog objektnog fajla.
	setOffsetSection();
	setGlobalSymbolOffset();//Sad su svi simboli u tabeli simbola sa svojom stvarnom vrednoscu.
	//Globalni simbol sad u offsetu cuva svoju apsolutnu adresu.
	fixRealocations();//Sada su i realokacije prepravljene;
	loadSectionIntoMemory();//Ucitano je u memoriju;
	setIVT();
	init(); //Treba testirati init();
	execute();
	cout << "Finished executing program." << endl;
	
}
void Procesor::setIVT(){
	//Smestam adrese prekidnih rutina za reset,time,incorect i prekidna rutina tastera.
	cout << "============setingIVT==============" << endl;
	auto symbolReset = symbols.find("reset");
	if (symbolReset == symbols.end()) throw runtime_error("Reset routine not defined.");
	int offset = symbolReset->second.offset;//Ovo je adresa ove labele.
	string bytes = std::bitset<16>(offset).to_string();
	string lowerB = bytes.substr(8, 8);
	string higherB = bytes.substr(0, 8);
	memory[0] = stoi(lowerB,nullptr,2);
	memory[1] = stoi(higherB,nullptr,2);
	auto symbolTimer = symbols.find("timer");
	if (symbolTimer == symbols.end()) throw runtime_error("Timer routine not defined.");
    offset = symbolTimer->second.offset;//Ovo je adresa ove labele.
    bytes = std::bitset<16>(offset).to_string();
    lowerB = bytes.substr(8, 8);
    higherB = bytes.substr(0, 8);
	memory[2] = stoi(lowerB, nullptr, 2);
	memory[3] = stoi(higherB, nullptr, 2);
	auto symbolKeyboard = symbols.find("keyboard");
	if (symbolKeyboard == symbols.end()) throw runtime_error("Keyboard routine not defined.");
	offset = symbolKeyboard->second.offset;//Ovo je adresa ove labele.
	bytes = std::bitset<16>(offset).to_string();
	lowerB = bytes.substr(8, 8);
	higherB = bytes.substr(0, 8);
	memory[6] = stoi(lowerB, nullptr, 2);
	memory[7] = stoi(higherB, nullptr, 2);
	auto symbolIncorect = symbols.find("incorect");
	if (symbolIncorect == symbols.end()) throw runtime_error("Incorect routine not defined.");
	offset = symbolIncorect->second.offset;//Ovo je adresa ove labele.
	bytes = std::bitset<16>(offset).to_string();
	lowerB = bytes.substr(8, 8);
	higherB = bytes.substr(0, 8);
	memory[4] = stoi(lowerB, nullptr, 2);
	memory[5] = stoi(higherB, nullptr, 2);
	for (int i = 8; i < 16; i++){
		memory[i] = 0;
	}
	//Provera memorije 
	cout << "===================Sadrzaj memorije ================= " << endl;
	for (auto i : memory){

		cout << i.second << " ";
	}
	int e = stoi("FFFF", nullptr, 16);
	for (int i = 0; i < e; i++){
		memory.insert(std::pair<int, int>(i, 0));
	}
	
}
void Procesor::TimerInterruptRoutine(){
	/*if (psw.I == 1){
		cout << "I flag is set."<<endl;
		return;
	}
	*/
	
	cout << "Timer interrupt!" << endl;
	psw.setValue();

	push(PC);
	push(psw.value);
	psw.T = 0;
	psw.I = 0;
	string value = std::bitset<8>(memory[3]).to_string() + std::bitset<8>(memory[2]).to_string();
	PC = std::stoi(value, nullptr, 2);
	
}
void Procesor::handleIncorect(){
	cout << "Incorect instruction!" << endl;
	incorectInstruction = false;
	psw.setValue();
	push(PC);
	push(psw.value);
	psw.T = 0;
	psw.I = 0;
	string value = std::bitset<8>(memory[5]).to_string() + std::bitset<8>(memory[4]).to_string();
	
	PC = std::stoi(value, nullptr, 2);

}
void Procesor::HandleInterrupts(){
	chrono::time_point<chrono::system_clock> currentTime = chrono::system_clock::now();
	chrono::duration<double> elapsed = currentTime - lastTimerExecution;
	double val= elapsed.count();
	cout << "Timer value  is : " << val << endl;
	if (incorectInstruction){//Ovo moze da se desi samo ako nismo u okviru prekidne rutine i kod mene ne spada u blokirajuce prekide.
		handleIncorect();
	}
	else if (elapsed.count() >= 1.0 ){
		cout << "Timer is greater than 1." << endl;
		if (psw.T == 0){
			cout << "T flag(13th bit of psw) is not set." << endl;
			return;
		}
		lastTimerExecution = currentTime;
		TimerInterruptRoutine();
	}
/*	if (keyboardInt){
		KeyboardInterruptRoutine();
	}
	if (memory[KEYBOARD_OUT] != 0){//Ovaj keyboardOUT cemo postaviti u okviru prekidne rutine koju cemo definisati.
		char temp = memory[KEYBOARD_OUT];
		memory[KEYBOARD_OUT] = 0;
		cout << "Print char :" << temp << endl;//Ovde treba jos rutina koja cita karakter koji je kliknut

	}*/
}


void Procesor::KeyboardInterruptRoutine(){
	/*if (psw.I == 1){
		cout << "I flag is set." << endl;
		return;
	}
	cout << "Keyboard interrupt!" << endl;
	char temp = keyboardBuffer;//Tu se smesta sta se ukuca.
	keyboardInt = false;
	//Ova naredna linija ne znam da l treba odnosno rad sa tastaturom ne zna da l je ovako organizovan ili treba drugacije.

	memory[KEYBOARD_IN] = temp;//Ovo je kao lokacija registra ulazno ilazne periferije.
	psw.setValue();
	push(PC);
	push(psw.value);
	string value = std::bitset<8>(memory[7]).to_string() + std::bitset<8>(memory[6]).to_string();
	PC = std::stoi(value, nullptr, 2);
*/
}

void Procesor::readNextInstruction(){

	string instruction = std::bitset<8>(memory[PC]).to_string();
	instruction += std::bitset<8>(memory[PC + 1]).to_string();
	instruction += std::bitset<8>(memory[PC + 2]).to_string();
	instruction += std::bitset<8>(memory[PC + 3]).to_string();
	//Ovo je da procerim da l smo stigli do kraja .
	
	//Procitali smo 4B za instruckiju i to saljemo ka instrukciji da ona izparsira taj podataka;
	currentInstruction = new Instruction(instruction);
	
	if (currentInstruction->longIns){
		PC += 4;
	}
	else 
		PC += 2;
}
bool Procesor::checkCondition(){
	cout << "Current instruction condition is : " << currentInstruction->cond << endl;
	if (currentInstruction->cond == InstructionCond::EQ)
		return psw.Z == 1;
	if (currentInstruction->cond == InstructionCond::NE)
		return psw.Z == 0;
	if (currentInstruction->cond == InstructionCond::GT)
		return (psw.Z == 0 && psw.N == 0);
	if (currentInstruction->cond == InstructionCond::AL)
		return true;
}
int Procesor::getValueOperand(int operand){
	AddressingType addr;
	int bits;//Ona 3 bita
	int additionalB=currentInstruction->addBytes;
	if (operand == 1){
		addr = currentInstruction->firstOpAdd;
		bits = currentInstruction->firstBit;
	}
	else {
		addr = currentInstruction->secondOpAdd;
		bits = currentInstruction->secondBit;
	}
	if (addr == AddressingType::IMM)
		return additionalB;
	if (addr == AddressingType::REG_DIR){ //Ovde treba dodati da ne sme da se cita ili upisuje u registre sp i pc;oNDA JE TO NEKOREKTNA INSTRUCKIJA.
		if (bits == 7) return PC;
		if (bits == 6) return SP;

		return reg[bits];
	}
	if (addr == AddressingType::MEM){//Ako je jedan operan citanje podatka iz memorije rezultat drugi mora biti 
		int lowerB = memory[additionalB];
		int higherB = memory[additionalB + 1];
		string valueS = std::bitset<8>(higherB).to_string() + std::bitset<8>(lowerB).to_string();
		
		cout << "Data fetched from memory : " << additionalB << " = " << stoi(valueS, nullptr, 2)<<"." << endl;

		return stoi(valueS, nullptr, 2);
	}
	if (addr == AddressingType::REG_IND_POM){
		int r;
		if (bits == 7) r = PC;
		else if (bits == 6) r = SP;
		else r = reg[bits];
		int address = r+additionalB;
		int lowerB = memory[address];
		int higherB = memory[address + 1];
		string valueS = std::bitset<8>(higherB).to_string() + std::bitset<8>(lowerB).to_string();
		cout << "Data fetched from memory : " << address << " = " << stoi(valueS, nullptr, 2) << "." << endl;
		return stoi(valueS, nullptr, 2);
	}

}
void Procesor::putResult(int result){
	AddressingType addrT = currentInstruction->firstOpAdd;
	int addBytes = currentInstruction->addBytes;
	int bits = currentInstruction->firstBit;
	if (addrT == AddressingType::REG_DIR){
		if (bits == 7) {//Ovde treba videti da li zelimo da dozvolio to .
			PC = result; return;
		}
		if (bits == 6){
			SP = result; return;
		}
		reg[bits] = result;
		return;
	}
	if (addrT == AddressingType::MEM){
		string value = std::bitset<16>(result).to_string();
		int higherB = std::stoi(value.substr(0, 8), nullptr, 2);
		int lowerB = std::stoi(value.substr(8, 8), nullptr, 2);
		memory[addBytes] = lowerB;
		memory[addBytes+1] = higherB;
		cout << "Data set in memory : " << addBytes << " = " << result << "." << endl;
		return;
	}
	if (addrT == AddressingType::REG_IND_POM){
		string value = std::bitset<16>(result).to_string();
		int higherB = std::stoi(value.substr(0, 8), nullptr, 2);
		int lowerB = std::stoi(value.substr(8, 8), nullptr, 2);
		int r = 0;
		if (bits == 7) r = PC;
		else if (bits == 6) r = SP;
		else r = reg[bits];
		int address = addBytes + r;
		memory[address] = lowerB;
		memory[address + 1] = higherB;
		cout << "Data set in memory: " << address << " = " << result << "." << endl;
		return;
	}
}
void Procesor::handleInstruction(){
	
	//cout << "Executing: " << currentInstruction->name << " , " << "first Operand: " << getValueOperand(1) << ",secondOperand: " << getValueOperand(2) << ",result: " << result << endl;
	if (currentInstruction->name == InstuctionName::ADD){
		short first = short(getValueOperand(1));
		short second = short(getValueOperand(2));
		
		short result =first +second;
		cout << "Executing ADD " << " , " << "first Operand: " << first << ",second Operand: " << second << ",result: " << result << endl;
		putResult(result);
		psw.Z = result == 0;
		psw.N = result < 0;
		psw.O = (first>=0 && second >= 0 && result<0) || (first<0 && second < 0 && result >= 0);
		psw.C = (result >= 0 && (first < 0 || second < 0)) ||(result<0 && first<0 && second<0);
		return;
	}
	if (currentInstruction->name == InstuctionName::SUB){
		short first = short(getValueOperand(1));
		short second = short(getValueOperand(2));
		
		short result = first -second;
		cout << "Executing SUB " <<" ,  " <<"first Operand: " <<first<< ",second Operand: "<<second<<",result: "<<result  << endl;
		putResult(result);
		psw.Z = result == 0;
		psw.N = result < 0;
		psw.O = (first>=0 && second< 0 && result<0) || (first<0 && second >= 0 && result >= 0);
		psw.C = result < 0 && first >=0;
		return;
	}
	if (currentInstruction->name == InstuctionName::MUL){
		short first = short(getValueOperand(1));
		short second = short(getValueOperand(2));
		
		short result = first * second;
		cout << "Executing MUL" << " , " << "first Operand: " << first << ",second Operand: " << second << ",result: " << result << endl;
		putResult(result);
		psw.Z = result == 0;
		psw.N = result < 0;
		return;
	}
	if (currentInstruction->name == InstuctionName::DIV){
		short first = short(getValueOperand(1));
		short second = short(getValueOperand(2));
		
		short result = first / second;
		cout << "Executing DIV " << " , " << "first Operand: " << first << ",second Operand: " << second << ",result: " << result << endl;
		putResult(result);
		psw.Z = result == 0;
		psw.N = result < 0;
		return;
	}
	
	if (currentInstruction->name == InstuctionName::CMP){
		short first = short(getValueOperand(1));
		short second = short(getValueOperand(2));
		
		short result = first - second;
		cout << "Executing CMP:" << " , " << "first Operand: " << first << ",second Operand: " << second <<" , result: " << result << endl;
		//putResult(result);
		psw.Z = result == 0;
		psw.N = result < 0;
		psw.O = (first >= 0 && second< 0 && result<0) || (first<0 && second >= 0 && result >= 0);
		psw.C = result < 0 && first >= 0; 
		return;
	}
	if (currentInstruction->name == InstuctionName::AND){
		short first = short(getValueOperand(1));
		short second = short(getValueOperand(2));
		
		short result = first & second;
		cout << "Executing AND:" << " , " << "first Operand: " << first << ",second Operand: " << second << " , result: " << result << endl;
		putResult(result);
		psw.Z = result == 0;
		psw.N = result < 0;
		return;
	}
	if (currentInstruction->name == InstuctionName::OR){
		short first = short(getValueOperand(1));
		short second = short(getValueOperand(2));
		
		short result = first | short(getValueOperand(2));
		cout << "Executing OR:" << " , " << "first Operand: " << first << ",second Operand: " << second << " , result: " << result << endl;
		putResult(result);
		psw.Z = result == 0;
		psw.N = result < 0;
		return;
	}
	if (currentInstruction->name == InstuctionName::NOT){
		short first = short(getValueOperand(1));
		short second = short(getValueOperand(2));
		
		short result = ~ second;
		cout << "Executing NOT:" << " , " << "first Operand: " << first << ",second Operand: " << second << " , result: " << result << endl;
		putResult(result);
		psw.Z = result == 0;
		psw.N = result < 0;
	}
	if (currentInstruction->name == InstuctionName::TEST){
		short first = short(getValueOperand(1));
		short second = short(getValueOperand(2));
		
		short result = first & second;
		cout << "Executing TEST:" << " , " << "first Operand: " << first << ",second Operand: " << second << " , result: " << result << endl;
		//putResult(result);
		psw.Z = result == 0;
		psw.N = result < 0;
		return;

	}
	if (currentInstruction->name == InstuctionName::PUSH){
		cout << "Executing PUSH;" << endl;
		short value = short(getValueOperand(1));
		
		push(value);//Imam metodu koja radi push za mene;
		return;
	}
	if (currentInstruction->name == InstuctionName::POP){

		cout << "Executing POP;" << endl;
		short value = pop();
		putResult(value);
		return;
	}
	if (currentInstruction->name == InstuctionName::CALL){
		
		//Kad pozivas CALL moja pretpostavka je da najbolje samo napisemo &xOdnosno adresu na koju skacem dobijam kao vrednost labele.b
		short address = short(getValueOperand(1));
		
		cout << "Executing CALL, "<< "address : " << address<< endl;
		push(PC);
		PC = address;
		return;
	}
	if (currentInstruction->name == InstuctionName::MOV){
		short first = short(getValueOperand(1));
		short second = short(getValueOperand(2));
		
		short result = second;
		cout << "Executing MOV: " <<" value: "<<result << endl;	
		putResult(result);
		psw.Z = result == 0;
		psw.N = result < 0;
		return;
	}
	if (currentInstruction->name == InstuctionName::SHL){
		short first = short(getValueOperand(1));
		short second = short(getValueOperand(2));
		if (second>16) incorectInstruction = true;
		short result = first;
		for (int i = 1; i <= second; i++){
			psw.C = result<0? 1:0;
			result <<= 1;
		}
		result<<= second;
		cout << "Executing SHL:" << " , " << "first Operand: " << first << ",second Operand: " << second << " , result: " << result << endl;

		putResult(result);
		psw.Z = result == 0;
		psw.N = result < 0;
		
		return;
	}
	if (currentInstruction->name == InstuctionName::SHR){
		short first = short(getValueOperand(1));
		short second = short(getValueOperand(2));
		short result = short(getValueOperand(1));
		if (second>16) incorectInstruction = true;
		for (int i = 1; i <= second; i++){
			psw.C = (result % 2 == 0) ? 0 : 1;
			result >>= 1;
		}
		//result>>= short(getValueOperand(2));
		cout << "Executing SHR:" << " , " << "first Operand: " << first << ",second Operand: " << second << " , result: " << result << endl;

		putResult(result);
		psw.Z = result == 0;
		psw.N = result < 0;
		
		return;
	}
	if (currentInstruction->name == InstuctionName::RET){
		//ret naredba ne postoji onda se kodira sa pop;
		/*cout << "Executing ret" << endl;
		short value = pop();//Skinemo vrednost sa steka.
		PC = value;
		if (PC == 0) endProgram = true; //Ovde smo testirali rad programa.
		*/return;
	}
	if (currentInstruction->name == InstuctionName::IRET){
		cout << "Executing IRET;" << endl;
		short p = pop();
		string ss =std::bitset<6>(p).to_string();
		psw.set(ss);
		PC = pop();
	}
	
}
void Procesor::executeInstruction(){
	
	
	if (checkCondition()){
		handleInstruction();//Konkretno sta radi instrukcija;
	}
	else {	
		cout << "Skipping instruction,condition is not met." << endl;

	}
	writeStatusProcesor();
	if(PC!=0) HandleInterrupts();

}


void Procesor::writeStatusProcesor(){
	//Ovde ispisujemo stanje procesora.
	for (int i = 0; i < 6; i++){
		cout << "R[" << i << "]" << "=" << reg[i] << endl;
	}
	cout << "PC:" << PC;
	cout << "SP:" << SP;
	cout << "PSW" << "=" << "I:" << psw.I << " " << "T:" << psw.T << " " << "Z:" << psw.Z << " " << "C: " << psw.C << " " << "O:" << psw.O << " " << "N:" << psw.N << " " << endl;
	//Ovde sam ispisala i status psw reci.
}


void Procesor::execute(){
	writeStatusProcesor();
	readNextInstruction();//Sada je postavljena currentInstruction i 
	while (1){
		executeInstruction();
		if (PC == 0) break;
		readNextInstruction();
	}
	cout << "Finished program" << endl;
}
int Procesor::getSectionOffset(SectionType sectionT){
	map<string, Symbol>::iterator it;
	for (it = symbols.begin(); it != symbols.end(); it++){
		if (it->second.type == TokenType::SECTION && it->second.section == sectionT){
			return it->second.offset;
		}
	}
	//Ovde ne znam zasto ne izbacuje gresku;Ako ne vraca van for.
}

void Procesor::setGlobalSymbolOffset(){
	map<string, Symbol>::iterator it;
	for (it = symbols.begin(); it != symbols.end(); it++){
		if (it->second.type == TokenType::LABEL){
			int offsetAdd = getSectionOffset(it->second.section);
			it->second.offset += offsetAdd;
		}
	}
	cout << "=========GlobalSimbolOffset Setting========" << endl;
	for (auto it : symbols){
		string name = it.first;
		Symbol sym = it.second;

		cout << name << "|" << name << "|" << sym.type << "|" << sym.offset << endl;

	}

}

void Procesor::setOffsetSection(){
	code_finish = code_start;
	map<string, Symbol>::iterator it;
	for (it = symbols.begin(); it != symbols.end(); it++){
		if (it->second.type == TokenType::SECTION){
			it->second.offset = code_finish;
			code_finish += it->second.size;
		}
	}
	cout << "============================Podesavanje offseta sekcija zbog smestanja u memoriju===========" << endl;
	for (auto it : symbols){
		string name = it.first;
		Symbol sym = it.second;
	
		cout << name << "|" << name << "|"<<sym.type<<"|" << sym.offset  << endl;

	}

}


void Procesor::fixRealocations(){
	cout << "=======Fixing Realocations======" << endl;
	for (unsigned i = 0; i < realocations.size(); i++){
		Relocation& r = realocations[i];//Isto vraca referencu;
		auto section = sections.find(r.section);
		auto sectionSymbol = symbols.find(section->second.name);
		auto symbol = symbols.find(r.symbol);
		if (r.relT == RelocationType::ABS){
			section->second.addOffset(symbol->second.offset, r.offset);
		}
		else {
			int addValue = sectionSymbol->second.offset + r.offset;
			addValue -= symbol->second.offset;
			section->second.addOffset(addValue, r.offset);
		}
	}
	//Ovde dodajem ispis contenta svih sekcija da bih proverila da li mi je ispravka realokacija bila ispravna.
	for (auto section : sections){
		Section sec = section.second;
		cout << "#" << section.first << endl;
		int size = sec.size;
		for (int i = 0; i < size; i++){
			auto con = sec.content.find(i);
			cout << con->second << " ";
		}
		cout << "" << endl;
		cout << "#end" << endl;
		cout << "" << endl;
	}
}
void Procesor::push(int value){
	if (STACK_END == SP) {
		cout << "Stack overflow" << endl;
		throw runtime_error("Stack overflow!");
	}
	SP -= 2;
	string valueS = std::bitset<16>(value).to_string();
	string lowerB = valueS.substr(8, 8);
	string higherB = valueS.substr(0, 8);
	int lowerN = std::stoi(lowerB, nullptr, 2);
	int higherN = std::stoi(higherB, nullptr, 2);
	cout << "Stack push : " << value << endl;
	memory[SP] = higherN;
	memory[SP - 1] = lowerN;
	
}
int Procesor::pop(){
	if (SP == STACK_START) {
		cout << "Stack underflow!" << endl;
		throw runtime_error("Stack underflow!");
	}
	int higherN = memory[SP];
	int lowerN = memory[SP - 1];
	SP += 2;
	string higherB = std::bitset<8>(higherN).to_string();
	string lowerB = std::bitset<8>(lowerN).to_string();
	string valueS = higherB + lowerB;
	int value = std::stoi(valueS, nullptr, 2);
	cout << "Stack pop : " << value << endl;
	return value;

}
void Procesor::loadSectionIntoMemory(){
	map<string, Section>::iterator it;
	

	int pom = code_start;
	for (it = sections.begin(); it != sections.end(); it++)
	{
		Section sec = it->second;
		map<int, string>::iterator itC;
		for (itC = sec.content.begin(); itC != sec.content.end(); itC++){
			int value = stoi(itC->second, nullptr, 16);
			memory[pom]= value; 
			pom++;
		}
	}
	
	STACK_END = pom;
	if (STACK_END >= STACK_START) throw runtime_error("Program is to big,stack _end >=stack_start; ");
	cout << "Stack_end: "<<STACK_END << endl;

}
void Procesor::KeyboardThreadMethod(){
	while (true){
		while (keyboardInt);//Cekam dok god je postavljen znak za izvrsavanje interrupta.
		cin >> keyboardBuffer;
		keyboardInt = true;
		cout << "Read char : " << keyboardBuffer << endl;
	}
}
void Procesor::init(){
	incorectInstruction = false;
	SP = stoi("FF80", nullptr, 16);//Ovo je pocetak steka.Ukazuje na poslednju zauzetu lokaciju na steku.
	PC = 0;
	push(PC);//Ideja je kad se vratimo iz START podprograma da procita PC kao nulu i da zna da je doslo do kraja;

	auto s= symbols.find("START");
	PC = s->second.offset;//Posto je globalni simbol ima svoju apsolutnu adresu.
	
	endProgram = false;
	psw.reset(); //Ovo se radi kod int;
	psw.T = 1;//Dozovljavamo trap instrukciju.
	psw.I = 1;//Dozvoljeni su spolji maskirajuci prekidi;
	executionStart = chrono::system_clock::now();
	lastTimerExecution = executionStart;
	
	//std::thread keyboardThread=std::thread(&Procesor::KeyboardThreadMethod,this);//Ovo valjda ovako moze.
	psw.setValue();
	push(PC);//Mi ovde stavimo da je povratna vrednost ustavi labela start;
	push(psw.value);
	psw.T = 0;//Postavljam ih na 0 kada je je ulazak u prekidnu rutinu.
	psw.I = 0;//Postavljam na 0 kada je ulazak u prekidnu rutinu.
	string value = (std::bitset<8>(memory[1])).to_string() + (std::bitset<8>(memory[0])).to_string();
	PC = std::stoi(value, nullptr, 2);
	cout << "PC value: " << PC<<endl;
	cout << "SP value:" << SP<<endl;
	cout << "Finished init" << endl;//Ovde je izbacivaio break point;

}
void Procesor::readFile(ifstream &inFile){
	string line = "";
	getline(inFile, line);
	//line.erase(line.length()-1);
	
	while (line != "#Tabela Simbola"){
		
		getline(inFile, line);
		
	}
	cout<<"Da li sam stigao ovde"<<endl;
	//Ovde treba dodati sta se radi ako se ne naidje na ovo #Tabela Simbola.
	readSymTable(inFile);
	getline(inFile, line);
	//line.erase(line.length()-1);
	
	while (line != ".END"){
		cout<<"WHile"<<endl;
		if (line[0] == '#'){
			string pom = "";
			for (int i = 0; i < 5; i++){
				pom += line[i];
			}
			if (pom == "#.ret"){//Onda znamo da je relokaciona tabela.
				string name = "";
				for (unsigned i = 5; i < line.length(); i++){
					name += line[i];
				}
				//Dohvatili smo ime same sekcije;

				readRelocation(inFile, name);
			}
			else {
				cout<<"Sekcija"<<endl;
				string name = "";
				for (unsigned i = 1; i < line.length(); i++){//Ovde je bila poenta sto i izadje iz scopa za greske i onda je line[i] za i koji je 8 i tad svati da nema vise a posto je rodata najduze sto moze da se desi mi cemo to da stavimo;
					//if (line[i] == ' ') break;Msm da ova linija nije nephodna.
					name += line[i];
				}
				readSection(inFile, name);
			}
		}
		getline(inFile, line);
		//line.erase(line.length()-1);
	}
	auto symbBSS = symbols.find(".bss");
	if (symbBSS != symbols.end()){
		Section * s = new Section(symbBSS->second.name, symbBSS->second.size);
		
		for (int i = 0; i < s->size; i++){
			s->write1(i, "00");
		}
		sections.insert({ ".bss", *s });
	}

}

void Procesor::readSection(ifstream& inFile, string name){
	string line = "";
	getline(inFile, line);
	//line.erase(line.length()-1);
	queue<string> tokens;
	while (line != "#end"){
		split_line(line, " ", tokens);
		getline(inFile, line);
		//line.erase(line.length()-1);
	}
	int size = tokens.size();
	Section * s = new Section(name, size);
	for (int i = 0; i < size; i++){
		string content = tokens.front();
		tokens.pop();
		s->write1(i, content);
	}
	sections.insert({ name, *s });
	//Ovde znaci kupimo informacije iz ulaznog fajla o samoj sekciji.
	cout << "Content of section  : " << name << endl;
	s->readAll();//Ovde je radi debagovanja bila faza da se ispise procitan sadzanj sekcije.
}
string Procesor::getSymbolName(int symbol){
	map<string, Symbol>::iterator it;

	for (it = symbols.begin(); it != symbols.end(); it++){
		Symbol sym = it->second;
		if (sym.ind == symbol) return sym.name;
	}
	cout << "Nije pronadjen u tabeli simbola simbol sa tim rednim brojem." << symbol << endl;
	return "";
}
void Procesor::readRelocation(ifstream& inFile, string section){
	string line = "";
	getline(inFile, line);//Citamo sledeci red koji je nevazan;
	getline(inFile, line);//Ovde citamo prvi red od znacaja
	//line.erase(line.length()-1);
	while (line != "#end"){//Dok ne dodjem do kraja realokacione tabele;
		queue<string> tokens;
		split_line(line, "|", tokens);
		string offset = tokens.front();
		tokens.pop();
		string type = tokens.front();
		tokens.pop();
		int symbol = stoi(tokens.front());
		tokens.pop();
		RelocationType rel;
		if (type == "R_386_32") rel = RelocationType::ABS;
		else rel = RelocationType::PCREL;
		string sym = getSymbolName(symbol);
		Relocation r(section, stoi(offset), rel, sym);
		realocations.push_back(r);
		getline(inFile, line);
		//line.erase(line.length()-1);
		cout << "Procitana je realokacija za sekciju :" << section << "=" << offset << " ," << type << endl;

	}
}
void Procesor::readSymTable(ifstream &inFile){
	string line = "";
	getline(inFile, line);//Ovo je prva nevazna linija sa nazivima kolona
	getline(inFile, line);//Ovo je prva linija od znacaja
	//line.erase(line.length()-1);
	while (line != "#end Symbol Table"){
		queue<string> tokens;
		split_line(line, "|", tokens);
		string name = tokens.front();
		tokens.pop();
		SectionType section = getSectionType(tokens.front());
		tokens.pop();
		int offset = stoi(tokens.front());
		tokens.pop();
		ScopeType scope = getScopeType(tokens.front());
		tokens.pop();
		TokenType type = getTokenType(tokens.front());
		tokens.pop();
		int size = stoi(tokens.front());
		tokens.pop();
		int number = stoi(tokens.front());
		tokens.pop();
		Symbol sym(name, offset, type, scope, section, size, number);
		symbols.insert({ name, sym });
		getline(inFile, line);
		//line.erase(line.length()-1);
		cout << "Procitan simbol: " << sym.name << " ," << sym.offset << " ," << sym.ind << "." << endl;
	}
	cout << "Upsesno citanje tabele simbola.";
}
ScopeType Procesor::getScopeType(string scope){
	if (scope == "L")
		return ScopeType::LOCAL;
	if (scope == "G")
		return ScopeType::GLOBAL;
	else throw runtime_error("Wrong scope symbol in input file in TS");

}
TokenType Procesor::getTokenType(string type){
	if (type == "label")
		return TokenType::LABEL;
	if (type == "section")
		return TokenType::SECTION;
	if (type == "symbol")
		return TokenType::SYMBOL;
	else throw runtime_error("Wrong token type in TS in input file");

}
SectionType Procesor::getSectionType(string section){

	if (section == "bss")
		return SectionType::BSS;
	if (section == "start")
		return SectionType::START;//Ovo ne moze da se javi ovde.
	if (section == "rodata")
		return SectionType::RODATA;
	if (section == "data")
		return SectionType::DATA;
	if (section == "text")
		return SectionType::TEXT;
	return SectionType::START;//Ako je neka greska u pitanju.




}
void Procesor::split_line(string line, const char* delim, queue<string>& tokens){
	size_t start = line.find_first_not_of(delim);

	size_t end = start;

	while (start != std::string::npos){
		end = line.find_first_of(delim, start);

		tokens.push(line.substr(start, end - start));

		start = line.find_first_not_of(delim, end);
	}


}
