#include "ObjectFile.h"
#include <fstream>


void ObjectFile::readFile(string inputFile){
	ifstream inFile(inputFile);
	string line = "";
	if (!inFile.is_open()){
		cerr << "Error:opening input file:"<<inputFile << endl;
		exit(1);
	}
	getline(inFile,line);
	line.erase(line.length()-1);
	cout<<"GEtLINE" <<line<<endl;
	while (line != "#Tabela Simbola"){
		getline(inFile, line);
		line.erase(line.length()-1);
	}
	
	//Ovde treba dodati sta se radi ako se ne naidje na ovo #Tabela Simbola.
	readSymTable(inFile);
	//Ovde je procitana tabelaSimbola sad idemo na citanje sekcija i realokacija pa cemo onda da sredjujemo sta ne valja.
	
	getline(inFile, line);
	line.erase(line.length()-1);
	while (line != ".END"){
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
				string name = "";
				for (unsigned i = 1; i < line.length(); i++){
					name += line[i];
				}
				readSection(inFile, name);
			}
		}
		getline(inFile, line);
		line.erase(line.length()-1);
	}
	//Na kraju uvek u sve sekcije treba da dodam i objekat za BSS sekciju u sections.
	auto symbBSS = symbols.find(".bss");
	if (symbBSS != symbols.end()){
		Section * s = new Section(symbBSS->second.name, symbBSS->second.size);
		sections.insert({ ".bss", *s });
	}
	symbols.erase("und");
}

void ObjectFile::readSection(ifstream& inFile, string name){
	string line = "";
	getline(inFile, line);
	line.erase(line.length()-1);
	queue<string> tokens;
	while (line != "#end"){
		split_line(line, " ", tokens);
		getline(inFile, line);
		line.erase(line.length()-1);
	}
	int size = tokens.size();
	Section * s = new Section(name, size);
	for (int i = 0; i < size; i++){
		string content = tokens.front();
		tokens.pop();
		s->write1(i, content);
		cout << "Procitan je sadrzaj sekcije jednog fajla : " << content << endl;
	}
	sections.insert({ name, *s });
	cout << "Uneta je sekcija sa imenom" << name << endl;
	
	//Ovde znaci kupimo informacije iz ulaznog fajla o samoj sekciji.
	
}

void ObjectFile::readRelocation(ifstream& inFile, string section){
	string line = "";
	getline(inFile, line);//Citamo sledeci red koji je nevazan;
	getline(inFile, line);//Ovde citamo prvi red od znacaja
	line.erase(line.length()-1);
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
		relocations.push_back(r);
		getline(inFile, line);
		line.erase(line.length()-1);
		cout << "Procitana je realokacija za sekciju :" << section << "=" << offset << " ," << type << endl;

	}
}
string ObjectFile::getSymbolName(int symbol){
	map<string, Symbol>::iterator it;

	for (it = symbols.begin(); it != symbols.end(); it++){
		Symbol sym = it->second;
		if (sym.ind == symbol) return sym.name;
	}
	cout << "Nije pronadjen u tabeli simbola simbol sa tim rednim brojem." << symbol << endl;
	return "";
}
void ObjectFile::readSymTable(ifstream &inFile){
	string line = "";
	getline(inFile, line);//Ovo je prva nevazna linija sa nazivima kolona
	getline(inFile, line);//Ovo je prva linija od znacaja
	line.erase(line.length()-1);
	while (line != "#end Symbol Table"){
		queue<string> tokens;
		split_line(line, "|", tokens);
		string name = tokens.front();
		tokens.pop();
		SectionType section =getSectionType(tokens.front());
		tokens.pop();
		int offset =stoi( tokens.front());
		tokens.pop();
		ScopeType scope = getScopeType(tokens.front());
		tokens.pop();
		TokenType type =getTokenType(tokens.front());
		tokens.pop();
		int size = stoi(tokens.front());
		tokens.pop();
		int number =stoi(tokens.front());
		tokens.pop();
		Symbol sym(name, offset, type, scope, section, size, number);
		symbols.insert({ name, sym });
		getline(inFile, line);
		line.erase(line.length()-1);
		cout << "Procitan simbol: " << sym.name << " ," << sym.offset << " ," << sym.ind << "."<<endl;

	}
	cout << "Uspesno citanje tabele simbola jednog objektong fajla."<<endl;
}
ScopeType ObjectFile::getScopeType(string scope){
	if (scope == "L")
		return ScopeType::LOCAL;
	if (scope == "G")
		return ScopeType::GLOBAL;
	else throw runtime_error("Wrong scope symbol in input file in TS");

}
TokenType ObjectFile::getTokenType(string type){
	if (type == "label")
		return TokenType::LABEL;
	if (type == "section")
		return TokenType::SECTION;
	if (type == "symbol")
		return TokenType::SYMBOL;
	else throw runtime_error("Wrong token type in TS in input file");


}
SectionType ObjectFile::getSectionType(string section){

	if (section == "bss")
		return SectionType::BSS;
	if (section == "UND")
		return SectionType::UND;
	if (section == "rodata")
		return SectionType::RODATA;
	if (section == "data")
		return SectionType::DATA;
	if (section == "text")
		return SectionType::TEXT;
	return SectionType::START;//Ako je neka greska u pitanju.




}

void ObjectFile::split_line(string line, const char* delim, queue<string>& tokens){
	size_t start = line.find_first_not_of(delim);

	size_t end = start;

	while (start != std::string::npos){
		end = line.find_first_of(delim, start);

		tokens.push(line.substr(start, end - start));

		start = line.find_first_not_of(delim, end);
	}


}
