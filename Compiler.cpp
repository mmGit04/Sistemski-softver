#include "Compiler.h"

map<string,int> Compiler::instructionNumOperands=
{
	{ "add",2 },
	{ "sub",2 },
	{ "mul",2},
	{ "div", 2 },
	{ "cmp",2},
	{ "and", 2 },
	{ "or", 2 },
	{ "not", 2 },
	{ "test", 2 },
	{ "push", 1 },
	{ "pop", 1},
	{ "iret",0},
	{ "call",1 },
	{ "mov", 2 },
	{ "shl", 2 },
	{ "shr", 2 },
	{ "ret", 0},
	{ "jmp",  1}
	
};
map<AddressingType,string> Compiler::codeAddresingType=
{
	{ AddressingType::IMM, "00" },
	{ AddressingType::MEM, "10" },
	{ AddressingType::REG_DIR, "01" },
	{AddressingType::REG_IND_POM,"11"},
	{AddressingType::PC_POM,"11"},
	

};
void Compiler::secondPassGlobal(queue<string>& tokens,SectionType currSec){
	while (!tokens.empty()){
		string currTokenS = tokens.front();
		TokenType currToken = (TokenType)parseToken(currTokenS);
		tokens.pop();
		if (currToken != SYMBOL){
			throw runtime_error("After .global expecting symbol.");
		}
		auto symbol = symTable.find(currTokenS);
		if (symbol != symTable.end()) {
			symbol->second.scope = ScopeType::GLOBAL;//Ako se taj simbol nalazi u tabeli Simbola samo treba da ga oznacimo kao GLOBAL.
		}
		else {
			addNewSymbol(currTokenS, locationCounter, currToken, ScopeType::GLOBAL, SectionType::UND, false);//Ako nije definisan onda ga samo ubacim u tabelu simbola kao nedefinisan.
		}//Ovde ce bez obzira vrednost locationCountera da bude 0 jer se sve ovo dogadja pre nego sto smo usli usekcije.

	}
}
void Compiler::secondPassDirective(string currTokenS, queue<string>& tokens, string currSec){
	if (currTokenS != ".align" && currTokenS != ".skip" && currSec == ".bss")
		throw runtime_error("Directive .char ,.word and .long cant be defined in .bss section.");

	if (currTokenS == ".align"){//Isto kao u prvom prolazu podesavamo vrednost LC.
		int oldCounter = locationCounter;//Msm da .align moze da se javi u bilo kojoj sekciji.
		string op = tokens.front();
		tokens.pop();
		TokenType opT = parseToken(op);
		int value = getValueOperand(op);
		if (!(locationCounter % value == 0)){
			locationCounter = locationCounter / value * value + value;
		}
		auto section = sections.find(currSec);
		section->second.writeZero(oldCounter, locationCounter - oldCounter);//Ali u drugom prolazu treba da popunimo i sekcije na koje nailazimo.
		return;//Vazno zapamtiti da se u jednoj lokaciji u sekcijama smesta HEKSADECIMALNA vrednost jednog bajta.
	}
	
	if (currTokenS == ".skip"){//Takodje msm da se moze javiti u bilo kojoj sekciji.
		string op = tokens.front();
		tokens.pop();
		TokenType opT = parseToken(op);
		if (opT != TokenType::OP_DEC) throw runtime_error("Wrong type of operand for .skip directive,has to be decimal number.");
		int opValue = getValueOperand(op);//Ovako necu ulaziti u getValueOperand ako nije dobar tip operanda,Jer tako moze da mi se napravi npr zapis o realnokoaciji a da ja to nisam zelela.Ovde prvi put pozivat getValueOperand.
		auto section = sections.find(currSec);
		if(section==sections.end()) cout<<"Nije pronasao sekciju u .skip direktivi "<<endl;
		section->second.writeZero(locationCounter, opValue);
		locationCounter += opValue;
		return;
	}

	if (currTokenS == ".char"){
		
		while (!tokens.empty()){
			string operand = tokens.front();
			tokens.pop();
			TokenType opT = parseToken(operand);
			if (opT != TokenType::OP_DEC)//Moja je zamisao da svaki simbol mora zauzimati 16 bita jer 
				throw runtime_error("Directive .char  can be written only with decimal operand.");
			int opValue = getValueOperand(operand);
			string op = std::bitset<8>(opValue).to_string();
			op = BinaryToHex(op);
			auto section = sections.find(currSec);
			section->second.write1(locationCounter, op);
			locationCounter++;

		}
		return;
	}

	if (currTokenS == ".word"){
	
		while (!tokens.empty()){
			string operand = tokens.front();
			tokens.pop();
			TokenType opT = parseToken(operand);
			if (opT != TokenType::OP_DEC && opT!=TokenType::OP_SYM_VALUE)//Moja je zamisao da svaki simbol mora zauzimati 16 bita jer 
				throw runtime_error("Directive .word and .long can be written only with decimal and symbol value operand.");
			//Moja pretpostavka je da u drugoj metodi koja se bavi vrednoscu ce na osnovu adresiranja da mi vrati koju vrendost treba da upisem.
			int opValue = getValueOperand(operand,currSec);
			string op = std::bitset<16>(opValue).to_string();
			string firstB = op.substr(8, 8);//Jer je little endian;
			firstB = BinaryToHex(firstB);
			auto section = sections.find(currSec);
			section->second.write1(locationCounter, firstB);
			locationCounter++;
			string secondB = op.substr(0, 8);
			secondB = BinaryToHex(secondB);
			section->second.write1(locationCounter, secondB);
			locationCounter++;
	
		}
		return;
	}
	if (currTokenS == ".long"){
	
		while (!tokens.empty()){
			string operand = tokens.front();
			tokens.pop();
			TokenType opT = parseToken(operand);
			if (opT != TokenType::OP_DEC && opT != TokenType::OP_SYM_VALUE)//Moja je zamisao da svaki simbol mora zauzimati 16 bita jer 
				throw runtime_error("Directive .word and .long can be written only with decimal and symbol value operand.");

			//Moja pretpostavka je da u drugoj metodi koja se bavi vrednoscu ce na osnovu adresiranja da mi vrati koju vrendost treba da upisem.
			int opValue = getValueOperand(operand,currSec);
			string op = std::bitset<32>(opValue).to_string();
			auto section = sections.find(currSec);
			string byte = op.substr(24, 8);
			byte = BinaryToHex(byte);
			section->second.write1(locationCounter, byte);
			locationCounter++;
		    byte = op.substr(16, 8);
			byte = BinaryToHex(byte);
			section->second.write1(locationCounter, byte);
			locationCounter++;
			 byte = op.substr(8, 8);
			byte = BinaryToHex(byte);
			section->second.write1(locationCounter, byte);
			locationCounter++;
			 byte = op.substr(0, 8);
			byte = BinaryToHex(byte);
			section->second.write1(locationCounter, byte);
			locationCounter++;
		}
		return;
	}



}

AddressingType Compiler::resolveAddresingTypeOperand(string firstT){
	std::regex op_dec{ ("([0-9]+)") };
	std::regex op_sym_value{ "(\\&)([a-zA-Z_][a-zA-Z0-9]*)" };
	std::regex op_sym_mem{ "([a-zA-Z_][a-zA-Z0-9]*)" };
	std::regex op_mem{ "(\\*)([0-9]+)" };
	std::regex op_reg{ "r[0-7]" };
	std::regex op_reg_ind_value{ "(r[0-7])(\\[)([0-9]+)(\\])" };
	std::regex op_reg_ind_sym{ "(r[0-7])(\\[)([a-zA-Z_][a-zA-Z0-9]*)(\\])" };
	std::regex op_$_label{ "(\\$)([a-zA-Z_][a-zA-Z0-9]*)" };

	if (regex_match(firstT, op_dec)){
		return AddressingType::IMM;
	}
	if (regex_match(firstT, op_reg)){
		return AddressingType::REG_DIR;
	}
	if (regex_match(firstT, op_sym_value)){
		return AddressingType::IMM;
	}
	if (regex_match(firstT, op_sym_mem)){
		return AddressingType::MEM;
	}
	if (regex_match(firstT, op_mem)){
		return AddressingType::MEM;
	}
	if (regex_match(firstT, op_reg_ind_value)){
		return AddressingType::REG_IND_POM;
	}
	if (regex_match(firstT, op_reg_ind_sym)){
		return AddressingType::REG_IND_POM;
	}
	if (regex_match(firstT, op_$_label)){
		return AddressingType::PC_POM;
	}
	throw runtime_error("Addressing type is not recognized");
	
}
void Compiler::handleInstructionZeroOperand(queue<string>& tokens, string content, string currSecS, SectionType _currSec){
	content += "0000000000";//Dodam one bite koji idu za operande.Tako da uvek bude 16 bita.
	insertIntoSection(currSecS, content, false, false,"");

}
void Compiler::handleInstructionOneOperand(queue<string>& tokens, string content, string currSecS, SectionType _currSec){
	string firstT = tokens.front();
	string firstContent = "";
	tokens.pop();
	AddressingType addT = resolveAddresingTypeOperand(firstT);
	string pom = content.substr(2, 4);
	if (pom == "1010" && addT == AddressingType::IMM) throw runtime_error("Destionation cant be imm operand");
	firstContent += codeAddresingType[addT];
	bool needAdditionalBytes = false;
	string reg = "000";
	int additionalBytes;
	locationCounter += 2;//Isti razlog zbog pravljenja realokacija.
	additionalBytes = getValueOperand(firstT, currSecS, _currSec, needAdditionalBytes, reg);
	locationCounter -= 2;
	firstContent += reg;
	string addBytesString;
	if (needAdditionalBytes == true) {
		addBytesString = std::bitset<16>(additionalBytes).to_string();
	}
	content += firstContent;
	content += "00000";//Dodaj one bite koji idu za drugi operand.
	insertIntoSection(currSecS, content, needAdditionalBytes,false, addBytesString);//Ova funkcija azurira vrednost LocationCountera.

}

void Compiler::handleInstructionTwoOperands(queue<string>& tokens,string content,string currSecS,SectionType _currSec){
	string firstT = tokens.front();//Ovde dohvatam prvi operand.
	string firstContent = "";
	tokens.pop();
	AddressingType addT = resolveAddresingTypeOperand(firstT);
	if (addT==AddressingType::IMM)
		throw runtime_error("First operand in two address instrucitons cant be immediate");
	firstContent += codeAddresingType[addT];//Ovde smo dodali  kod za nacin adresiranja.
	bool needAdditionalBytes1 = false;
	string reg = "000";
	int additionalBytes1;
	locationCounter +=2;//Jer ako u getValueOperand treba da napravi realokaciju onda treba ova vrednost.
	additionalBytes1 = getValueOperand(firstT, currSecS, _currSec, needAdditionalBytes1, reg);
	locationCounter -= 2;
	firstContent += reg;//Dodajemo obavezno sadrzaj registra.
	//Na kraju cemo celu instruckiju da dodamo.
	string secondT = tokens.front();
	string secondContent = "";
	tokens.pop();
	addT = resolveAddresingTypeOperand(secondT);
	if(flagJMP && addT==AddressingType::PC_POM){
		secondContent+="00";
	}
	else {
		secondContent+=codeAddresingType[addT];
	}
	
	bool needAdditionalBytes2 = false;
	reg = "000";
	int additionalBytes2;
	locationCounter += 2;
	additionalBytes2 = getValueOperand(secondT, currSecS, _currSec, needAdditionalBytes2, reg);
	locationCounter -= 2;
	secondContent += reg;
	//Ovde na kraju ja imam podatke i vrednost dodatnog pomeraja i first i second operanda treba ih samo ucitati na odgovarajucu lokaciju.
	content += firstContent;
	content += secondContent;
	string addBytesString;
	if (needAdditionalBytes1 && needAdditionalBytes2)
		throw runtime_error("Both operand cannot demand 4B insturction.");
	if (needAdditionalBytes1 == true) {
		addBytesString = std::bitset<16>(additionalBytes1).to_string();
	}
	else if (needAdditionalBytes2 == true){
		addBytesString = std::bitset<16>(additionalBytes2).to_string();
	}

	insertIntoSection(currSecS, content, needAdditionalBytes1, needAdditionalBytes2, addBytesString);//Ova funkcija azurira vrednost LocationCountera.

}
void Compiler::secondPassInstruction(queue<string>& tokens, string currToken, SectionType _currSec, string currSecS){
	smatch match;
	flagJMP=false;
	string content = "";
	std::regex instruction{ "(int|add|sub|mul|div|cmp|and|or|not|test|push|pop|call|iret|mov|shl|shr|ret|jmp)(eq|ne|gt|al)?" };
	regex_match(currToken, match, instruction);
	string instructionName = match[1];
	string condition;
	string condContent = "11";
	bool condBool = false;
	if (match[2].matched){
		condBool = true;
		condition = match[2];
	}
	if (condBool){
		if (condition == "eq")
			condContent = "00";
		else if (condition == "ne")
			condContent = "01";
		else if (condition == "gt")
			condContent = "10";
	}
	content += condContent;//Instrukcija se tako koduje tako sto joj se prvo stavi Kod da li se uslovno ili bezuslovno izvrsava.
	int numOperand = instructionNumOperands[instructionName];
	if (instructionName == "ret"){
		instructionName = "pop";
		numOperand = 1;
		tokens.push("r7");
	}
	if (instructionName == "jmp") {
		numOperand = 2;
		flagJMP=true;
		string first = tokens.front();
		queue<string> helTokens;
		helTokens.push("r7");
		while (!tokens.empty()){
			string pom = tokens.front();
			tokens.pop();
			helTokens.push(pom);
		}
		tokens = helTokens;//Ovo treba proveriti da l je moze ovako da se napise.
		AddressingType adT = resolveAddresingTypeOperand(first);
		if (adT == AddressingType::PC_POM){
			instructionName = "add";
		}
		else instructionName = "mov";
	}
	switch (numOperand){
	case 2:
		if (instructionName == "add")
			content += "0000";
		else if (instructionName == "sub")
			content += "0001";
		else if (instructionName == "mul")
			content += "0010";
		else if (instructionName == "div")
			content += "0011";
		else if (instructionName == "cmp")
			content += "0100";
		else if (instructionName == "and")
			content += "0101";
		else if (instructionName == "or")
			content += "0110";
		else if (instructionName == "not")
			content += "0111";
		else if (instructionName == "test")
			content += "1000";
		else if (instructionName == "mov")
			content += "1101";
		else if (instructionName == "shl")
			content += "1110";
		else if (instructionName == "shr")
			content += "1111";//Dodajem kod same instrukcije.
		handleInstructionTwoOperands(tokens, content,currSecS, _currSec);
		break;
	case 1:
		if (instructionName == "call")
			content += "1011";
		else if (instructionName == "push")
			content += "1001";
		else if (instructionName == "pop")
			content += "1010";
		handleInstructionOneOperand(tokens, content, currSecS, _currSec);
		break;
	case 0:
	
		 if (instructionName == "iret")
			content += "1100";
		handleInstructionZeroOperand(tokens, content, currSecS, _currSec);
		break;
	}
	
}

void Compiler::insertIntoSection(string _currSectionS,string content,bool needAdditionalBytes1,bool needAdditionalBytes2,string addBytesString){
	auto sec = sections.find(_currSectionS);
	string firstB = content.substr(0, 8);//Kod samog koda instrukcije prvo se smesta visi B pa onda tek nizi ,samo kod adresa i kod pomeraja se koristi to drugo pravilo,ako neko bude rekao da je drugacije to onda treba videti.
	firstB = BinaryToHex(firstB);
	sec->second.write1(locationCounter, firstB);
	locationCounter++;
	string secondB = content.substr(8, 8);//Nizi B ide na visu adresu .
	secondB = BinaryToHex(secondB);
	sec->second.write1(locationCounter, secondB);
	locationCounter++;
	if (needAdditionalBytes1 || needAdditionalBytes2){
			firstB = addBytesString.substr(8 ,8);//Nizi B ide na nizu adresu .
			firstB = BinaryToHex(firstB);
			sec->second.write1(locationCounter, firstB);
			locationCounter++;
			secondB = addBytesString.substr(0, 8);//Visi B ide na visu adresu.
			secondB = BinaryToHex(secondB);
			sec->second.write1(locationCounter, secondB);
			locationCounter++;
	}
}

string Compiler::getNameSection(SectionType sec){
	switch (sec){
	case TEXT:
		return ".text";
	case DATA:
		return ".data";
	case RODATA:
		return ".rodata";
	case BSS:
		return ".bss";
	case START:
		return "start";
	default:	return  "";
	}
}
string Compiler::decToBinString(string dec){
	if (dec == "0") return "000";
	if (dec == "1") return "001";
	if (dec == "2") return "010";
	if (dec == "3") return "011";
	if (dec == "4") return "100";
	if (dec == "5") return "101";
	if (dec == "6") return "110";
	if (dec == "7") return "111";
	else throw runtime_error("Decimal value is wrong");
};

string Compiler:: BinaryToHex(string bin){
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


bool Compiler::helper = false;
string Compiler::helper1 = "000";
//Ovde bi trebalo videti i sta se radi da vracam informaciju i kojoj vrsti adresiranja se radi kod obrade instruckija.
int Compiler::getValueOperand(string operand, string _currSecS, SectionType _currSec , bool& needAdditionalBytes, string& reg){//Ovde sam stavila da kao imam podrazumevanu vrednost za trenutnu sekciju.
	std::regex op_dec{ ("([0-9]+)") };
	std::regex op_sym_value{ "(\\&)([a-zA-Z_][a-zA-Z0-9]*)" };
	std::regex op_sym_mem{ "([a-zA-Z_][a-zA-Z0-9]*)" };
	std::regex op_mem {"(\\*)([0-9]+)"};
	std::regex op_reg{ "(r)([0-7])" };
	std::regex op_reg_ind_value{ "(r)([0-7])(\\[)([0-9]+)(\\])" };
	std::regex op_reg_ind_sym {"(r)([0-7])(\\[)([a-zA-Z_][a-zA-Z0-9]*)(\\])"};
	std::regex op_$_label{ "(\\$)([a-zA-Z_][a-zA-Z0-9]*)" };
	
	smatch match;
	if (regex_match(operand, match, op_dec)){
		needAdditionalBytes = true;//Ovo je decimalna vrednost samo zadata.
		return stoi(match[1]);
	}
	if (regex_match(operand, match, op_reg)){
		needAdditionalBytes = false;
		string rNumber = match[2];
		int pom = stoi(rNumber);
		//if (pom == 6 || pom == 7)
			//throw runtime_error("Cant use pc and sp as registers.");
		reg = decToBinString(rNumber);
		return 0;
	}
	if (regex_match(operand, match, op_sym_value)){
		needAdditionalBytes = true;
		string symS = match[2];//Ovo je simbol;//I ovo moze da se javi u direktivama.
		return setAbsRelocation(symS, _currSecS);
	}
	if (regex_match(operand, match, op_sym_mem)){
		needAdditionalBytes = true;
		string symS = operand;
		return setAbsRelocation(symS,_currSecS);
	}
	if (regex_match(operand, match, op_mem)){
		needAdditionalBytes = true;
		return stoi(match[2]);
	}
	
	if (regex_match(operand, match, op_reg_ind_value)){
		needAdditionalBytes = true;
		string rNumber = match[2];//Zbog ovih zagrada je na 3. poziciji.
		reg = decToBinString(rNumber);
		return stoi(match[4]);
	}
	if (regex_match(operand, match, op_reg_ind_sym)){
		needAdditionalBytes = true;
		string rNumber = match[2];//Zbog ovih zagrada je na 3 poziciji.
		reg = decToBinString(rNumber);
		string symS = match[4];
		return setAbsRelocation(symS, _currSecS);
	}
	if (regex_match(operand, match, op_$_label)){
		if(flagJMP=false){
			string rNumber="7";
			reg=decToBinString(rNumber);
		}
		
		needAdditionalBytes = true;
		//string rNumber = "7";
		//reg = decToBinString(rNumber);
		string symS = match[2];
		return setPCRelRelocation(symS, _currSecS);

	}
	throw runtime_error("AddresingType is not recognized");
	return 0;
}
//LocationCounter sam dodavala sad 2 jer ja vrednost locationCountera menjam tek kad upisujem.Ali se to dogadja tek u drugom prolazu sto znaci da moze imati razlike u prvom i drugom prolazu.
//Samo smo u odnosu na apsolutnu relokaciju dodali da se upisuje -2 na offset.
int Compiler::setPCRelRelocation(string symS, string _currS){
	auto symbol = symTable.find(symS);
	if (symbol->second.scope == ScopeType::GLOBAL){//U pitanju je globalni 
		Relocation relocation(_currS, locationCounter, RelocationType::PCREL, symS);
		relocations.push_back( relocation);
		return -2;//Vrednost koju je potrebno upisati je 0.I tada je tip adresiranja neposredna vrednost.
	}
	else{
		//Ovo je ako je simbol lokalni.
		SectionType sec = symbol->second.section;
		string secS = getNameSection(sec);
		if (secS == _currS){
			return symbol->second.offset - (locationCounter + 2);//Ako je lokalni samo na tu vrednost treba smestiti razliku adrese naredne instruckije i offseta tog simbola.
		}
		Relocation relocation(_currS, locationCounter, RelocationType::PCREL, secS);
		relocations.push_back(relocation);
		return symbol->second.offset-2;//Upisujemo offset tog lokalnog simbola na tu lokaciju.

	}
}

int Compiler::setAbsRelocation(string symS,string _currSecS){
	auto symbol = symTable.find(symS);
	if(symbol==symTable.end()) throw runtime_error("Symbol koji se koristi za inicijalizaciju nekog podatka nije definisan");
	if (symbol->second.scope == ScopeType::GLOBAL){//U pitanju je globalni 
		Relocation relocation(_currSecS, locationCounter, RelocationType::ABS, symS);
		
		relocations.push_back(relocation);
		//Smestam ove podatke.
		return 0;//Vrednost koju je potrebno upisati je 0.I tada je tip adresiranja neposredna vrednost.
	}
	else{
		SectionType sec = symbol->second.section;
		string secS = getNameSection(sec);
		Relocation relocation(_currSecS, locationCounter, RelocationType::ABS, secS);
		relocations.push_back(relocation);
		return symbol->second.offset;//Upisujemo offset tog lokalnog simbola na tu lokaciju.

	}
}

void Compiler::SecondRun(){
	locationCounter = 0;
	SectionType currSection = START;
	string currSectionS = "start";
	TokenType currToken;
	string currTokenS;
	for (const auto &tokens : inputAssembly){

		queue<string> tokensQueue;
		for (const auto &token : tokens){
			tokensQueue.push(token);
		}
		currTokenS = tokensQueue.front();
		currToken = (TokenType)parseToken(currTokenS);
		if (currToken == LABEL){
			tokensQueue.pop();
			if (tokensQueue.empty())
				continue;//Odnosno ako je taj red prazan onda samo treba preci dalje da ne bi dole proveravao.
		}
		currTokenS = tokensQueue.front();
		currToken = (TokenType)parseToken(currTokenS);
		tokensQueue.pop();//Skinemo iz reda koji obradjujemo taj token koji smo dohvatili.
		switch (currToken)
		{
		case EXT_GLB:
			secondPassGlobal(tokensQueue,currSection);
			break;
		case DIRECTIVE:
			secondPassDirective(currTokenS,tokensQueue, currSectionS);
			break;
		case SECTION:
			currSectionS = currTokenS;
			updateDataSection(currTokenS, currSection);
			break;
		case INSTRUCTION:

			secondPassInstruction(tokensQueue,currTokenS,currSection,currSectionS);
		default:
			break;
		}
	

	}

}

void Compiler::FirstRun(ifstream& inputFile){
	
	parseInput(inputFile);
	locationCounter = 0;
	SectionType currSection = START;
	string currSectionS = "start";
	TokenType currToken;
	string currTokenS;
	addNewSymbol("und", locationCounter, TokenType::SECTION, ScopeType::LOCAL, SectionType::UND, true);
	for (const auto &tokens : inputAssembly){//const auto & definise da ja zelim da citam samo ,znaci nikako ne modifikujem ulaz tokom mog rada;

		queue<string> tokensQueue;
		for (const auto &token : tokens){
			tokensQueue.push(token);
		}
		//Obrada jednog reda.Svaki red je predstavljen sa strukturom red radi dohvatanja svakog elementa deo po deo.
		currTokenS = tokensQueue.front();
		currToken = (TokenType)parseToken(currTokenS);
		string labelName;
		
		if (currToken == LABEL){
			if (currSection == START)
				throw runtime_error("Label defined in start section.");
			labelName = currTokenS.substr(0, currTokenS.size() - 1);
			tokensQueue.pop();
			addNewSymbol(labelName, locationCounter,currToken,ScopeType::LOCAL,currSection,true);
			if (tokensQueue.empty())
				continue;
		}
		currTokenS = tokensQueue.front();
		currToken = (TokenType)parseToken(currTokenS);
		tokensQueue.pop();
		switch (currToken)
		{
		case LABEL:
			throw runtime_error("Double label definition in the same row.");
			break;
		case DIRECTIVE:
			if (currSection == START || currSection == TEXT)
				throw runtime_error("Directives cant be defined in START or TEXT section.");
			directiveFirstRun(currTokenS,tokensQueue);
			break;
		case SECTION:
			//Kada naidjemo na novu sekciju azuriramo vrednost stare sto znaci da ovu poslednju takodje treba da azurirammoKada naidjemo na end ili kraj fajla.
			if (currSection != SectionType::START){
				sections.insert({ currSectionS, Section(currSectionS, locationCounter) });
				symTable.find(currSectionS)->second.size = locationCounter;//Jer kad smo ubacivali u TS mi smo svima stavljali da je size=0;
			}
			currSectionS = currTokenS;//Sa tackom pamtim ime sekcije.
			updateDataSection(currTokenS,currSection);
			addNewSymbol(currTokenS, locationCounter, currToken, ScopeType::LOCAL, currSection, true);
			//Ovde se ocekuje kraj linije pa i to treba proveriti.Ne mora jer nece obradjivati ostatak linije samo cce preci na sledecu liniju.
			break;
		case EXT_GLB:
			if (currSection != START)
				throw runtime_error("Global directive cant be defined outside of START section.");
			while (!tokensQueue.empty()) tokensQueue.pop();//Ova linija msm da mi ne treba jer svejedno ja prelazim na naredni red.Msm ne smeta.
			break;
		case INSTRUCTION:
			if (currSection != TEXT)
				throw runtime_error("Instrucitons cant be defined outside of text section.");
			instructionFirstRun(currTokenS,tokensQueue);
			//Ovde je samo trebalo proveriti koja je velicina ove instrukcije zavisno od nacina adresiranja.
			break;
		case END:
			if (currSection != SectionType::START){
				sections.insert({ currSectionS, Section(currSectionS, locationCounter) });
				symTable.find(currSectionS)->second.size = locationCounter;//Jer kad smo ubacivali u TS mi smo svima stavljali da je size=0;
			}
			break;
		default:
			throw runtime_error("Wrong token at the start of line during first Run.");
			break;
		}


	}

}

void Compiler::instructionFirstRun(string instr, queue<string> &tokens){
	smatch match;
	std::regex instruction{ "(int|add|sub|mul|div|cmp|and|or|not|test|push|pop|call|iret|mov|shl|shr|ret|jmp)(eq|ne|gt|al)?" };
	std::regex op_dec{ "[0-9]+" };
	std::regex op_reg{ "r[0-7]" };
	std::regex_match(instr, match, instruction);
	string icode = match[1];
	int size = instructionNumOperands[icode];
	int largeinstr = false;
	if (size == 0){
		locationCounter += 2;
		return;
	}
	//Moja pretpostavka je da samo registarsko adresiranje ne zauzima 4 bajta.Odnosno samo ako je operand tako adresiran on nece da zauzima dodatna 2B za pomeraj odnosno neposrednu vrednost.
	string op1 = tokens.front();
	tokens.pop();
	if (!regex_match(op1, op_reg)) {
		locationCounter += 4;
		return;
	}
	if (size == 1){
		locationCounter += 2;
		return;
	}
	string op2 = tokens.front();
	tokens.pop();
	if (!regex_match(op2, op_reg)) {
		locationCounter += 4;
		return;
	}
	locationCounter += 2;
}



void Compiler::updateDataSection(string _currTokenS, SectionType& _currSection){

	locationCounter = 0;
	if (_currTokenS == ".text"){
		_currSection = SectionType::TEXT;
		return;
	}
	if (_currTokenS == ".data"){
		_currSection = SectionType::DATA;
		return;
	}
	if (_currTokenS == ".rodata"){
		_currSection = SectionType::RODATA;
		return;
	}
	if (_currTokenS == ".bss"){
		_currSection = SectionType::BSS;
		return;
	}

}
void Compiler::directiveFirstRun(string directive, queue<string> &tokens){
	
	
	if (directive == ".align"){
		string op = tokens.front();
		tokens.pop();
		TokenType opT = parseToken(op);
		if (opT != TokenType::OP_DEC) throw runtime_error("Directive .align nedd decimal operand.");
		int value = getValueOperand(op);
		if (!(locationCounter % value == 0)){
			locationCounter = locationCounter / value * value + value;
		}
		return;
	}
	if (directive == ".skip"){
		string op = tokens.front();
		tokens.pop();
		TokenType opT = parseToken(op);
		if (opT != TokenType::OP_DEC) throw runtime_error("Directive skip need decimal operand .");
		int value = getValueOperand(op);//Get value operand znaci pozivam samo ako znam da je dobar zapis ovog operanda.
		locationCounter += value;
		return;
	}
	//Ovde skip smo preskocili jer skip mora da uzme u obzir i ove operande.
	int pom = 0;
	if (directive == ".char"){
		pom = NumOperandsDirective(tokens);
		locationCounter += pom;
		return;
	}
	if (directive == ".word"){
		pom = NumOperandsDirective(tokens);
		locationCounter += 2*pom;
		return;
	}
	if (directive == ".long"){
		pom = NumOperandsDirective(tokens);
		locationCounter += 4*pom;
		return;
	}

}
int Compiler::NumOperandsDirective(queue<string> &tokens){
	int ret = 0;
	while (!tokens.empty()){
		tokens.pop();
		ret++;
	}
	return ret;
}

void Compiler ::addNewSymbol (string labelName, int locationCounter, TokenType token_type, ScopeType scope_type, SectionType curr_section, bool def){
	
	if (symTable.count(labelName) > 0){
		throw runtime_error("Symbol already defined.");
	}
	Symbol* sym=new Symbol(labelName, locationCounter, token_type, scope_type, curr_section, def,0);
	string pom = "LABEL";
	if (token_type == 1) pom = "SECTION";
	cout << "Added new symbol in symTab named: " << labelName <<" ,type : "<<pom<< endl;
	symTable.insert({ labelName, *sym });
}



TokenType Compiler::parseToken(string token){
	TokenType typeT = INCORECT;
	std::string ulaz = token;
	std::regex label{ "([a-zA-Z][a-zA-Z0-9_]*):" };
	std::regex section{ "(\\.)(text|data|bss|rodata)" };
	std::regex directive{ "\\.(align|char|word|long|skip)" };
	std::regex instruction{ "(int|add|sub|mul|div|cmp|and|or|not|test|push|pop|call|iret|mov|shl|shr|ret|jmp)(eq|ne|gt|al)?"};
	std::regex symbol{("^([a-zA-Z_][a-zA-Z0-9]*)$")};
	std::regex op_dec{ ("([0-9]+)") };
	std::regex op_sym_value{ "(\\&)([a-zA-Z_][a-zA-Z0-9]*)" };
	std::regex end{ "(\\.)(end)" };
	//compare ako su jednaki vraca 0.Ako su razliciti vraca !=0
	if (!ulaz.compare(".global"))
		return  TokenType::EXT_GLB;
    if (std::regex_match(ulaz, label))
		 return TokenType::LABEL;
	 if (std::regex_match(ulaz, directive))
		return TokenType::DIRECTIVE;
	 if (std::regex_match(ulaz, section))
		 return TokenType::SECTION;
	 if (std::regex_match(ulaz, instruction))
		 return TokenType::INSTRUCTION;
	 if (std::regex_match(ulaz, symbol))
		 return TokenType::SYMBOL;
	 if (std::regex_match(ulaz, op_dec))
		 return TokenType::OP_DEC;
	 if (std::regex_match(ulaz, op_sym_value))
		 return TokenType::OP_SYM_VALUE;
	 if (std::regex_match(ulaz, end))
		 return TokenType::END;
	 return typeT;
}

void Compiler::parseInput(ifstream& inputFile){
	string line;
	while (getline(inputFile, line)){
		line.erase(line.length()-1);

		vector<string> tokens;
		split_line(line, " ,\t", tokens);
		if (tokens.size() == 0)
			continue;
		if (tokens[0] == ".end"){
			inputAssembly.push_back(tokens);
			break;
		}

		inputAssembly.push_back(tokens);
	}
}

void Compiler::split_line(string line, const char* delim, vector<string>& tokens){
	size_t start = line.find_first_not_of(delim);
	
	size_t end = start;

	while (start != std::string::npos){
		end = line.find_first_of(delim, start);
		
		tokens.push_back(line.substr(start, end - start));
		
		start = line.find_first_not_of(delim, end);
	}


}



void Compiler::writeAssemblyInput(ofstream& output){

	for (auto t : inputAssembly){
		for (string token : t){
			output << token << endl;
		}

	}
}

void Compiler::writeSymTab(ofstream& output){
	output << "#Tabela Simbola" << endl;
	output << "Ime Simbola" << " |" << "Sekcija" << " |" << "Offset" << " |" << "Scope" <<" |" << "Tip simbola" << " | " << "Size"<<"|"<<"Number"<<endl;
	for (auto it : symTable){
		string name = it.first;
		Symbol sym = it.second;
		string section=getSectionName(sym.section);
		string scope = getScopeName(sym.scope);
		string type = getTypeName(sym.type);
		output << name << "|" << section << "|" << sym.offset << "|" << scope << "|" << type << "|" <<sym.size<<"|"<<sym.ind <<endl;

	}
	output << "#end Symbol Table" << endl;
}

string Compiler::getTypeName(TokenType type){
	switch (type)
	{
	case TokenType::LABEL:
		return "label";
	case TokenType::SECTION:
		return "section";
	case TokenType::SYMBOL:
		return "symbol";
	default:
		return "error";
	}

}
string Compiler::getScopeName(ScopeType scope){
	switch (scope)
	{
	case GLOBAL:
		return "G";
	case LOCAL:
		return "L";
	default:
		throw runtime_error("Not recognized scope type");
	}

}
string Compiler::getSectionName(SectionType section){
	switch (section)
	{
	case SectionType::START:
		return "UND";//Samo se global direktiva moze naci Van start.
	case SectionType::BSS:
		return "bss";
	case SectionType::DATA:
		return "data";
	case SectionType::RODATA:
		return "rodata";
	case SectionType::TEXT:
		return "text";
	case SectionType::UND:
		return "UND";
	default:
		throw runtime_error("Not recognized section name");
	}
}

void Compiler::writeSections(ofstream& output){
	for (auto section : sections){
		
		string name = section.first;
		if (name == ".bss") continue;
		Section sec = section.second;
		output << "#.ret" << name << endl;
		output << "#offset" << "  |  " << "tip" <<"  |  "<< "vr[" << name << "]:" << endl;
	
		for (Relocation rel : relocations){
			if (rel.section == name){
				string tip;
				switch (rel.relT){
				case ABS:
					tip = "R_386_32";
					break;
				case PCREL:
					tip = "R_386_PC32";
					break;
				}
				string symbol = rel.symbol;

				auto s = symTable.find(symbol);
				int symNumber = s->second.ind;
				output << rel.offset << "|" << tip << "|" << symNumber << endl;
			}
		}
		output << "#end" << endl;
		output << "" << endl;
		output << "#" <<name<< endl;
		int size = sec.size;
		for (int i = 0; i < size; i++){
			auto con = sec.content.find(i);
			output << con->second << " ";
		}
		output << "" << endl;
		output << "#end" << endl;
		output << "" << endl;
	}
	output << ".END" << endl;

}
