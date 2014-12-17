#include <sstream>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <stdlib.h>
#include <math.h>

; using namespace std;

// Set up Initial values
int clockCount = 128;
int cycle = 1;
char instruct[32];
char opcode[4];
int rs = 0;
int rt = 0;
int rd = 0;
int dataCount;
int dataClockStart = 128;
int regs[32];
int data[100];
int immediate;
bool negative = false;
bool sim = false;
string instruction;
string regAndImm = "";
string instructType = "";
string number = "";
ofstream disassembly("disassembly.txt");
ofstream simulation("simulation.txt");
ostringstream convert;

// Print the Current clock cycle in the text document for the simulation
void simul(){
	simulation << "--------------------" << endl;
	simulation << "Cycle:" << cycle << "\t" << clockCount << "\t" << instructType << regAndImm << endl;
	simulation << endl << "Registers" << endl;
	for (int i = 0; i < 25; i += 8){
		if (i < 10)
			simulation << "R0";
		else
			simulation << "R";
		simulation << i << ":";
		for (int j = i; j < i + 8; j++){
			simulation << "\t" << regs[j];
		}
		simulation << endl;
	}
	simulation << endl << "Data" << endl;
	for (int i = 0; i < dataCount; i += 8){
		simulation << dataClockStart + (i * 4) << ":";
		for (int j = i; j < i + 8; j++){
			simulation << "\t" << data[j];
		}
		simulation << endl;
	}
	simulation << endl;
}

// Method to convert a binary character array into the two's complement
void twoComp(char* binary, int z){
	if (binary[z] == '0'){
		binary[z] = '1';
	}
	else{
		binary[z] = '0';
		twoComp(binary, z - 1);
	}
}

// Method to convert a binary character array into a decimal value
void binaryConvert32(char* instruct){
	if (instruct[0] == '1'){
		for (int i = 0; i < 32; i++){
			if (instruct[i] == '1')
				instruct[i] = '0';
			else if (instruct[i] == '0')
				instruct[i] = '1';
		}
		twoComp(instruct, 31);
		negative = true;
	}
	for (int i = 31; i >= 0; i--){
		if (instruct[i] == '1')
			immediate = immediate + pow(2, 31 - i);
	}
	if (negative){
		immediate = immediate * -1;
	}
	negative = false;
}

// Method to convert an int into a ostringstream
void numConvert(int num){
	convert.str("");
	convert.clear();
	number.clear();
	convert << num;
	number = convert.str();
}

// Method to convert a decimal value into a binary character array
void decConvert32(int regValue, char* binary){
	if (regValue < 0){
		negative = true;
	}
	for (int i = 31; i >= 0; i--){
		if (regValue % 2 == 0)
			binary[i] = '0';
		else
			binary[i] = '1';
		regValue = (int)(regValue / 2);
	}
	if (negative){
		for (int i = 0; i < 32; i++){
			if (binary[i] == '0')
				binary[i] = '1';
			else
				binary[i] = '0';
		}
		twoComp(binary, 31);
	}
	negative = false;
}

// Method for the pseudo-MIPS Jump instruction
void j(){
	instructType = "J";
	immediate = 0;
	for (int i = 31; i >= 7; i--){ //offset 
		if (instruct[i] == '1')
			immediate = immediate + pow(2, 31 - i);
	}
	immediate *= 4;
	numConvert(immediate);
	regAndImm += " #"; regAndImm += number;
	
	if (sim){
		simul();
		clockCount = immediate - 4;
	}
}

// Method for the pseudo-MIPS Branch if Equal instruction
void beq(){
	instructType = "BEQ";
	immediate *= 4;
	numConvert(rs);
	regAndImm += " R"; regAndImm += number;
	numConvert(rt);
	regAndImm += ", R"; regAndImm += number;
	numConvert(immediate);
	regAndImm += ", #"; regAndImm += number;
	
	if (sim){
		simul();
		if (regs[rs] == regs[rt]){
			clockCount = clockCount + immediate;
		}
	}
}

// Method for the pseudo-MIPS Branch Greater than Zero instruction
void bgtz(){
	instructType = "BGTZ";
	immediate *= 4;
	numConvert(rs);
	regAndImm += " R"; regAndImm += number;
	numConvert(immediate);
	regAndImm += ", #"; regAndImm += number;
	
	if (sim){
		simul();
		if (regs[rs] > 0){
			clockCount = clockCount + immediate;
		}
	}
}

// Method for the pseudo-MIPS Break instruction
void progBreak(){
	instructType = "BREAK";
	
	if (sim){
		simul();
	}
}

// Method for the pseudo-MIPS Store Word instruction
void sw(){
	instructType = "SW";
	numConvert(rt);
	regAndImm += " R"; regAndImm += number; regAndImm += ", ";
	numConvert(immediate);
	regAndImm += number; regAndImm += "(R"; 
	numConvert(rs);
	regAndImm += number; regAndImm += ")";
	
	if (sim){
		int dataAddress = regs[rs] + immediate;
		int value = regs[rt];
		data[(dataAddress - dataClockStart) / 4] = value;
		simul();
	}
}

// Method for the pseudo-MIPS Load Word instruction
void lw(){
	instructType = "LW";
	numConvert(rt);
	regAndImm += " R"; regAndImm += number; regAndImm += ", ";
	numConvert(immediate);
	regAndImm += number; regAndImm += "(R";
	numConvert(rs);
	regAndImm += number; regAndImm += ")";
	
	if (sim){
		int dataAddress = regs[rs] + immediate;
		int value = data[(dataAddress - dataClockStart) / 4];
		regs[rt] = value;
		simul();
	}
}

// Method for the pseudo-MIPS Addition instruction
void add(){
	instructType = "ADD";
	numConvert(rd);
	regAndImm += " R"; regAndImm += number;
	numConvert(rs);
	regAndImm += ", R"; regAndImm += number;
	numConvert(rt);
	regAndImm += ", R"; regAndImm += number;
	
	if (sim){
		regs[rd] = regs[rs] + regs[rt];
		simul();
	}
}

// Method for the pseudo-MIPS Subtraction instruction
void sub(){
	instructType = "SUB";
	numConvert(rd);
	regAndImm += " R"; regAndImm += number;
	numConvert(rs);
	regAndImm += ", R"; regAndImm += number;
	numConvert(rt);
	regAndImm += ", R"; regAndImm += number;
	
	if (sim){
		regs[rd] = regs[rs] - regs[rt];
		simul();
	}
}

// Method for the pseudo-MIPS Multiplication instruction
void mul(){
	instructType = "MUL";
	numConvert(rd);
	regAndImm += " R"; regAndImm += number;
	numConvert(rs);
	regAndImm += ", R"; regAndImm += number;
	numConvert(rt);
	regAndImm += ", R"; regAndImm += number;
	
	if (sim){
		regs[rd] = regs[rs] * regs[rt];
		simul();
	}
}

// Method for the pseudo-MIPS And instruction
void and1(){
	instructType = "AND";
	numConvert(rd);
	regAndImm += " R"; regAndImm += number;
	numConvert(rs);
	regAndImm += ", R"; regAndImm += number;
	numConvert(rt);
	regAndImm += ", R"; regAndImm += number;
	
	if (sim){
		char binaryRS[32];
		decConvert32(regs[rs], binaryRS);
		char binaryRT[32];
		decConvert32(regs[rt], binaryRT);
		char binaryRD[32];
		for (int i = 0; i < 32; i++){
			if (binaryRS[i] == '1' && binaryRT[i] == '1')
				binaryRD[i] = '1';
			else
				binaryRD[i] = '0';
		}
		binaryConvert32(binaryRD);
		regs[rd] = immediate;
		simul();
	}
}

// Method for the pseudo-MIPS Or instruction
void or1(){
	instructType = "OR";
	numConvert(rd);
	regAndImm += " R"; regAndImm += number;
	numConvert(rs);
	regAndImm += ", R"; regAndImm += number;
	numConvert(rt);
	regAndImm += ", R"; regAndImm += number;
	
	if (sim){
		char binaryRS[32];
		decConvert32(regs[rs], binaryRS);
		char binaryRT[32];
		decConvert32(regs[rt], binaryRT);
		char binaryRD[32];
		for (int i = 0; i < 32; i++){
			if (binaryRS[i] == '1' || binaryRT[i] == '1')
				binaryRD[i] = '1';
			else
				binaryRD[i] = '0';
		}	
		binaryConvert32(binaryRD);
		regs[rd] = immediate;
		simul();
	}
}

// Method for the pseudo-MIPS Xor instruction
void xor1(){
	instructType = "XOR";
	numConvert(rd);
	regAndImm += " R"; regAndImm += number;
	numConvert(rs);
	regAndImm += ", R"; regAndImm += number;
	numConvert(rt);
	regAndImm += ", R"; regAndImm += number;

	if (sim){
		char binaryRS[32];
		decConvert32(regs[rs], binaryRS);
		char binaryRT[32];
		decConvert32(regs[rt], binaryRT);
		char binaryRD[32];
		for (int i = 0; i < 32; i++){
			if ((binaryRS[i] == '0' && binaryRT[i] == '1') || 
				(binaryRS[i] == '1' && binaryRT[i] == '0'))
				binaryRD[i] = '1';
			else
				binaryRD[i] = '0';
		}
		binaryConvert32(binaryRD);
		regs[rd] = immediate;
		simul();
	}
}

// Method for the pseudo-MIPS Nor instruction
void nor(){
	instructType = "NOR";
	numConvert(rd);
	regAndImm += " R"; regAndImm += number;
	numConvert(rs);
	regAndImm += ", R"; regAndImm += number;
	numConvert(rt);
	regAndImm += ", R"; regAndImm += number;

	if (sim){
		char binaryRS[32];
		decConvert32(regs[rs], binaryRS);
		char binaryRT[32];
		decConvert32(regs[rt], binaryRT);
		char binaryRD[32];
		for (int i = 0; i < 32; i++){
			if (binaryRS[i] == '0' && binaryRT[i] == '0')
				binaryRD[i] = '1';
			else
				binaryRD[i] = '0';
		}
		binaryConvert32(binaryRD);
		regs[rd] = immediate;
		simul();
	}
}

// Method for the pseudo-MIPS Add Immediate instruction
void addi(){
	instructType = "ADDI";
	numConvert(rt);
	regAndImm += " R"; regAndImm += number;
	numConvert(rs);
	regAndImm += ", R"; regAndImm += number;
	numConvert(immediate);
	regAndImm += ", #"; regAndImm += number;

	if (sim){
		regs[rt] = regs[rs] + immediate;
		simul();
	}
}

// Method for the pseudo-MIPS And Immediate instruction
void andi(){
	instructType = "ANDI";
	numConvert(rt);
	regAndImm += " R"; regAndImm += number;
	numConvert(rs);
	regAndImm += ", R"; regAndImm += number;
	numConvert(immediate);
	regAndImm += ", #"; regAndImm += number;

	if (sim){
		char binaryRS[32];
		decConvert32(regs[rs], binaryRS);
		char binaryImm[32];
		decConvert32(immediate, binaryImm);
		char binaryRT[32];
		for (int i = 0; i < 32; i++){
			if (binaryRS[i] == '1' && binaryImm[i] == '1')
				binaryRT[i] = '1';
			else
				binaryRT[i] = '0';
		}
		immediate = 0;
		binaryConvert32(binaryRT);
		regs[rt] = immediate;
		simul();
	}
}

// Method for the pseudo-MIPS Or Immediate instruction
void ori(){
	instructType = "ORI";
	numConvert(rt);
	regAndImm += " R"; regAndImm += number;
	numConvert(rs);
	regAndImm += ", R"; regAndImm += number;
	numConvert(immediate);
	regAndImm += ", #"; regAndImm += number;

	if (sim){
		char binaryRS[32];
		decConvert32(regs[rs], binaryRS);
		char binaryImm[32];
		decConvert32(immediate, binaryImm);
		char binaryRT[32];
		for (int i = 0; i < 32; i++){
			if (binaryRS[i] == '1' || binaryImm[i] == '1')
				binaryRT[i] = '1';
			else
				binaryRT[i] = '0';
		}
		immediate = 0;
		binaryConvert32(binaryRT);
		regs[rt] = immediate;
		simul();
	}
}

// Method for the pseudo-MIPS Xor Immediate instruction
void xori(){
	instructType = "XORI";
	numConvert(rt);
	regAndImm += " R"; regAndImm += number;
	numConvert(rs);
	regAndImm += ", R"; regAndImm += number;
	numConvert(immediate);
	regAndImm += ", #"; regAndImm += number;
	
	if (sim){
		char binaryRS[32];
		decConvert32(regs[rs], binaryRS);
		char binaryImm[32];
		decConvert32(immediate, binaryImm);
		char binaryRT[32];
		for (int i = 0; i < 32; i++){
			if ((binaryRS[i] == '0' && binaryImm[i] == '1') ||
				(binaryRS[i] == '1' && binaryImm[i] == '0'))
				binaryRT[i] = '1';
			else
				binaryRT[i] = '0';
		}
		immediate = 0;
		binaryConvert32(binaryRT);
		regs[rt] = immediate;
		simul();
	}
}

// Method to take Category 1 instructions, calculate proper registers and immediate values, and call the proper method
void category1(char* instruct){
	for (int i = 0; i < 4; i++){
		opcode[i] = instruct[i + 2];
	}
	for (int i = 4; i >= 0; i--){ // Base value for LW and SW
		if (instruct[i + 6] == '1')
			rs = rs + pow(2, 4 - i);
	}
	for (int i = 4; i >= 0; i--){
		if (instruct[i + 11] == '1')
			rt = rt + pow(2, 4 - i);
	}
	for (int i = 15; i >= 0; i--){ //offset 
		if (instruct[i + 16] == '1')
			immediate = immediate + pow(2, 15 - i);
	}
	if (opcode[0] == '0' && opcode[1] == '0' && opcode[2] == '0' && opcode[3] == '0'){
		j();
	}
	else if (opcode[0] == '0' && opcode[1] == '0' && opcode[2] == '1' && opcode[3] == '0'){
		beq();
	}
	else if (opcode[0] == '0' && opcode[1] == '1' && opcode[2] == '0' && opcode[3] == '0'){
		bgtz();
	}
	else if (opcode[0] == '0' && opcode[1] == '1' && opcode[2] == '0' && opcode[3] == '1'){
		progBreak();
	}
	else if (opcode[0] == '0' && opcode[1] == '1' && opcode[2] == '1' && opcode[3] == '0'){
		sw();
	}
	else if (opcode[0] == '0' && opcode[1] == '1' && opcode[2] == '1' && opcode[3] == '1'){
		lw();
	}
}

// Method to take Category 2 instructions, calculate proper registers, and call the proper method
void category2(char* instruct){
	for (int i = 0; i < 4; i++){
		opcode[i] = instruct[i + 12];
	}
	for (int i = 4; i >= 0; i--){
		if (instruct[i + 2] == '1')
			rs = rs + pow(2, 4-i);
	}
	for (int i = 4; i >= 0; i--){
		if (instruct[i + 7] == '1')
			rt = rt + pow(2, 4 - i);
	}
	for (int i = 4; i >= 0; i--){
		if (instruct[i + 16] == '1')
			rd = rd + pow(2, 4 - i);
	}
	if (opcode[0] == '0' && opcode[1] == '0' && opcode[2] == '0' && opcode[3] == '0'){
		add();
	}
	else if (opcode[0] == '0' && opcode[1] == '0' && opcode[2] == '0' && opcode[3] == '1'){
		sub();
	}
	else if (opcode[0] == '0' && opcode[1] == '0' && opcode[2] == '1' && opcode[3] == '0'){
		mul();
	}
	else if (opcode[0] == '0' && opcode[1] == '0' && opcode[2] == '1' && opcode[3] == '1'){
		and1();
	}
	else if (opcode[0] == '0' && opcode[1] == '1' && opcode[2] == '0' && opcode[3] == '0'){
		or1();
	}
	else if (opcode[0] == '0' && opcode[1] == '1' && opcode[2] == '0' && opcode[3] == '1'){
		xor1();
	}
	else if (opcode[0] == '0' && opcode[1] == '1' && opcode[2] == '1' && opcode[3] == '0'){
		nor();
	}
}

// Method to take Category 3 instructions, calculate proper register and immediate values, and call the proper method
void category3(char* instruct){
	for (int i = 0; i < 4; i++){
		opcode[i] = instruct[i + 12];
	}
	for (int i = 4; i >= 0; i--){
		if (instruct[i + 2] == '1')
			rs = rs + pow(2, 4 - i);
	}
	for (int i = 4; i >= 0; i--){
		if (instruct[i + 7] == '1')
			rt = rt + pow(2, 4 - i);
	}
	for (int i = 15; i >= 0; i--){
		if (instruct[i + 16] == '1')
			immediate = immediate + pow(2, 15 - i);
	}
	if (opcode[0] == '0' && opcode[1] == '0' && opcode[2] == '0' && opcode[3] == '0'){
		addi();
	}
	else if (opcode[0] == '0' && opcode[1] == '0' && opcode[2] == '0' && opcode[3] == '1'){
		andi();
	}
	else if (opcode[0] == '0' && opcode[1] == '0' && opcode[2] == '1' && opcode[3] == '0'){
		ori();
	}
	else if (opcode[0] == '0' && opcode[1] == '0' && opcode[2] == '1' && opcode[3] == '1'){
		xori();
	}
}

// Main method
int main(int argc, _TCHAR* argv[]){
	// Create variables for file
	ifstream myfile;
	string filename;
	string fileLine;
	vector<string> fileRead;
	int i = 0;
	
	// Read through command line file
	cin >> filename;
	myfile.open(filename.c_str());
	while (!myfile.eof()){
		myfile >> fileLine;
		fileRead.push_back(fileLine);
		i++;
	}
	myfile.close();

	// Loop through instructions and create disassembly file and set up values for simulation run through
	for (int j = 0; j < i; j++){
		string instruction = fileRead[j];
		std::copy(instruction.begin(), instruction.end(), instruct);
		if (instructType.compare("NUMBER") == 0){
			binaryConvert32(instruct);
			data[dataCount] = immediate;
			dataCount++;
		}
		else if (instruct[0] == '0' && instruct[1] == '0'){
			category1(instruct);
		}
		else if (instruct[0] == '0' && instruct[1] == '1'){
			category2(instruct);
		}
		else if (instruct[0] == '1' && instruct[1] == '0'){
			category3(instruct);
		}
		if (instructType.compare("BREAK") != 0 && instructType.compare("NUMBER") != 0){
			disassembly << instruction << "\t" << clockCount << "\t" << instructType << regAndImm << endl;
			dataClockStart += 4;
		}
		else if (instructType.compare("BREAK") == 0){
			disassembly << instruction << "\t" << clockCount << "\t" << instructType << endl;
			instructType = "NUMBER";
			dataClockStart += 4;
		}
		else if (instructType.compare("NUMBER") == 0){
			disassembly << instruction << "\t" << clockCount << "\t" << immediate << endl;
		}
		rs = 0;
		rt = 0;
		rd = 0;
		immediate = 0;
		clockCount += 4;
		regAndImm = "";
	}

	// Loop through instructions second time and create the simulation of the file
	instructType = "";
	sim = true;
	clockCount = 128;
	for (int j = 0; j < i; j++){
		j = ((clockCount - 128) / 4);
		string instruction = fileRead[j];
		std::copy(instruction.begin(), instruction.end(), instruct);
		if (instructType.compare("BREAK") == 0){
			break;
		}
		else if (instruct[0] == '0' && instruct[1] == '0'){
			category1(instruct);
		}
		else if (instruct[0] == '0' && instruct[1] == '1'){
			category2(instruct);
		}
		else if (instruct[0] == '1' && instruct[1] == '0'){
			category3(instruct);
		}
		cycle++;
		rs = 0;
		rt = 0;
		rd = 0;
		immediate = 0;
		clockCount += 4;
		regAndImm = "";
	}

	// Close output files
	disassembly.close();
	simulation.close();

	return 0;
};