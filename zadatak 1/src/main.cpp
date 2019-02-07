#include <iostream>
#include <fstream>
#include "Compiler.h"

using namespace std;

int main(){

	ifstream inFile;
	inFile.open("ulaz.txt");

	if (!inFile.is_open()){
		cerr << "Error:opening input file." << endl;
		exit(1);
	}
	ofstream outFile;
	outFile.open("izlaz.txt");

	if (!outFile.is_open()){
		cerr << "Error:opening output file." << endl;
		exit(1);
	}



	Compiler* asembler = new Compiler();
	try{
		asembler->FirstRun(inFile);
		asembler->SecondRun();
		asembler->writeSymTab(outFile);
		asembler->writeSections(outFile);
	}
	catch (exception& e){
			cerr <<"Error:"<< e.what() << endl;
	}
	

	inFile.close();
	outFile.close();
	cout << "Success!" << endl;
	return 0;


}