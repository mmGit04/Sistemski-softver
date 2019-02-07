#include "Linker.h"

void Linker::Link(vector<string> &inputFiles){
	cout<<"In Link;" <<endl;
	for (auto &inputFile : inputFiles){
		linkFile(inputFile);
	}
	checkLink();//Jer ovo za start i UND moram da radim tek kad linkujem sve fajlove.
}

string Linker::getScopeName(ScopeType scope){
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
string Linker::getTypeName(TokenType type){
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
string Linker::getSectionName(SectionType section){
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
void Linker::setNumber(){
	int num = 0;
	map<string, Symbol>::iterator it;
	for (it = symbols.begin(); it != symbols.end(); it++){
		it->second.ind = ++num;
	}
	//Ovako da svaki simbol ima jedinstvenu indetifikaciju.
}

void Linker::writeSymTab(ofstream& output){
	setNumber();
	output << "#Tabela Simbola" << endl;
	output << "Ime Simbola" << " |" << "Sekcija" << " |" << "Offset" << " |" << "Scope" << " |" << "Tip simbola" << " | " << "Size" << "|" << "Number" << endl;
	for (auto it : symbols){
		string name = it.first;
		Symbol sym = it.second;
		string section = getSectionName(sym.section);
		string scope = getScopeName(sym.scope);
		string type = getTypeName(sym.type);
		output << name << "|" << section << "|" << sym.offset << "|" << scope << "|" << type << "|" << sym.size << "|" << sym.ind << endl;

	}
	output << "#end Symbol Table" << endl;
}

void Linker::writeSections(ofstream& output){
	for (auto section : sections){

		string name = section.first;
		if (name == ".bss") continue;
		Section sec = section.second;
		output << "#.ret" << name << endl;
		output << "#offset" << "  |  " << "tip" << "  |  " << "vr[" << name << "]:" << endl;

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

				auto s = symbols.find(symbol);
				int symNumber = s->second.ind;
				output << rel.offset << "|" << tip << "|" << symNumber << endl;
			}
		}
		output << "#end" << endl;
		output << "" << endl;
		output << "#" << name << endl;
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
void Linker::checkLink(){
	map<string, Symbol>::iterator it;

	for (it = symbols.begin(); it != symbols.end(); it++){
		Symbol sym = it->second;
		if (sym.section == SectionType::UND) throw runtime_error("Simbol" + sym.name+" nije definisan ni u jednom fajlu koji linkujemo.");
	}
	for (it = symbols.begin(); it != symbols.end(); it++){
		Symbol sym = it->second;
		if (sym.type == TokenType::LABEL && sym.name == "START") return;
	}
	throw runtime_error("Simbol START nije definisan ni u jednom od fajlova.");
}
int Linker::addSection(Symbol& symbol, Section& section){
	//Ovo je ako ta sekcija vec postoji on je nece dodati a ako ne postoji onda ce je dodati.
	//Ovo su reference na memoriju u okviru objekta koji predstavlja taj jedan fajl.
	sections.insert({ section.name, section });
	auto sym = symbols.find(symbol.name);
	int offset;
	if (sym == symbols.end()){
		symbols.insert({ symbol.name, symbol });
		offset = 0;
		cout << "Prvi put nailazimo na sekciju : " << section.name << endl;
	}
	else {
		offset = sym->second.size;
		sym->second.size += symbol.size;
		auto sec = sections.find(symbol.name);
		
		sections.find(symbol.name)->second.addContent(section);
		cout << "Linkovana je nova sekcija sa offsetom: " << offset << " i nazivom: " << symbol.name << endl;
	}
	return offset;
}

void Linker::addSymbol(Symbol& symbol){
	auto sym = symbols.find(symbol.name);
	if (sym == symbols.end()){
		symbols.insert({ symbol.name, symbol });
	}
	else if (symbol.section != SectionType::UND){//To znaci da je definisan u tom fajlu a ne da se uvozi.
		if (sym->second.section != SectionType::UND){
			throw runtime_error("Simbol" + symbol.name + " je vec definisan.");
		}
		else {//Ako ga je neki drugi fajl uvozio .A mi sad nailazimo na njegovu definiciju.
			sym->second.offset = symbol.offset;
			sym->second.section = symbol.section;
			sym->second.type = symbol.type;
			//Ja smatram da je taj simbol vec postavljen kao globalan.
			//Ovde treba proveriti da li je taj simbol postavljen kao globalni u ovom fajlu.
		}
	//Opcija ako postoji vec UND i mi ponovo ubacujemo UND on se samo nece ponovo ubaciti.
	}
}

void Linker::linkFile(string inputFile){
	ObjectFile* objectFile=new ObjectFile();
	objectFile->readFile(inputFile);
	//Sad linkujemo ovaj fajl na ostale fajlove.
	//Ovde je zavrseno citanje ulaznog fajla i njegovo umetanje u poseban objekat.
	
	map<SectionType, int> offset;
	//Prvo linkujemo sekcije i ako je kreiran neki offset za njih to pamtim.
	for (auto &symbol : objectFile->symbols){
		if (symbol.second.type == TokenType::SECTION){
			auto section = objectFile->sections.find(symbol.second.name);
			offset[symbol.second.section] = addSection(symbol.second, section->second);
		}
	}
	
	for (auto &symbol : objectFile->symbols){
		if (symbol.second.type != TokenType::SECTION){//Jer moze da bude i simbol i labela.
			//Ja hocu da ubacim  u krajnju tabelu simbola samo globalne simbole.
			//A za lokalne simbole kada je radjena realokacija mora da se sredi memorija.

			if (symbol.second.scope == ScopeType::GLOBAL){//Ovo radimo znaci samo za globalne simbole.
				if (symbol.second.section != SectionType::UND){//Ovo znaci  da je definisan u okviru tog fajla i u okviru neke sekcije.
					symbol.second.offset += offset[symbol.second.section];//Gore je tip za sekciju koja moze da ima samo odredjeni skup vrednosti.
				}
				addSymbol(symbol.second);
			}
		}
	}
	
	for (auto &relocation : objectFile->relocations){
		Symbol section = objectFile->symbols[relocation.section];
		Symbol symbol = objectFile->symbols[relocation.symbol];
		//1.Pomeramo offset odnosno gde ce da se menja vrednost ;
		//2.treba promenititi i vrdnost koja tamo stoji.
		relocation.offset += offset[section.section];
		if (symbol.type == TokenType::SECTION){//Ako stavim ovako section onda ima dobro semanticko znacenje.
			Section& content = sections[section.name];//Ja znam da je to sekcija.Taj simbol na koji se odnosi realokacija.
			int offsetAdd = offset[symbol.section];//Pomeraj sekcije za koju se vezao simbol koji smo uneli.
			//Ovu vrednost sad treba predstaviti kao string;Dodati je na staru vrednost.
			content.addOffset(offsetAdd, relocation.offset);
		}
		relocations.push_back(relocation);
	}
}
