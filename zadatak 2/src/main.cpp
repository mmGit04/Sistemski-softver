#include <iostream>
#include <fstream>
#include "Linker.h"
#include <vector>
#include "Procesor.h"
using namespace std;

int main(int argc, char ** argv){
	
	cout<<"Program started"<<endl;
	ofstream outFile;
	outFile.open("exit.txt");
	if (!outFile.is_open()){
		cerr << "Error:opening output file." << endl;
		exit(1);
	}
	vector<string> inputFiles;
	//for (int i = 2; i < argc-2; i++){
		//inputFiles.push_back(argv[i]);
	//}
	
	inputFiles.push_back("ivtDefault.txt");
	inputFiles.push_back("izlaz1ASM.txt");
	inputFiles.push_back("izlaz2ASM.txt");
	cout<<"Size inputfiles: "<<inputFiles.size()<<endl;
	Linker* linker = new Linker();
	try{
		linker->Link(inputFiles);
		linker->writeSymTab(outFile);
		linker->writeSections(outFile);
	}
	catch (exception& e){
		cerr << "Error:" << e.what() << endl;
	}

	int code_start = 20;//Ako zadajem pocetnu adresu kao heksadecimalni niz.
	
	ifstream inFile;
	inFile.open("exit.txt");

	try{
		Procesor* procesor = new Procesor(code_start);
		procesor->processing(inFile);


	}
	catch (exception& e){
		cerr << "Error:" << e.what() << endl;
	}

	outFile.close();
	inFile.close();
	return 0;

}
