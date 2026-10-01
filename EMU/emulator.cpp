#include "emulator.hpp"
#include <fstream>

#define ADDR 0xFFFF
#define MASK 0xF

#define MOV 0b0000
#define INC 0b0001
#define ADD 0b0010
#define ADDI 0b0011
#define JNE 0b0100
#define JL 0b0101
#define JG 0b0110
#define JE 0b0111
#define MUL 0b1000
#define DIV 0b1001
#define NOT 0b1010
#define OR 0b1011
#define SUBD 0b1100
#define SUB 0b1101
#define DEC 0b1110
#define AND 0b1111

namespace emu{
	program::program(std::string file){
		std::ifstream F(file+".prg", std::ios::ate | std::ios::binary); if(!F) return;
		size=F.tellg(); F.seekg(0); code=new int[size];
		F.read(reinterpret_cast<char*>(code),size); F.close(); size/=4;
		F.open(file+".dat", std::ios::ate | std::ios::binary); if(!F) return;
		int s=F.tellg(); F.seekg(0); F.read(reinterpret_cast<char*>(data), s); F.close();
	}
	
	bool program::operator()(){
		int t, c, k;
		REGS[1]=code[REGS[5]]; REGS[5]++;
		k=REGS[1]&ADDR; k=(REGS[2]=(k)? k:REGS[2]);
		REGS[1]>>=16; REGS[3]=data[REGS[2]];
		int s8=(REGS[1]>>8)&MASK, s4=(REGS[1]>>4)&MASK, s0=REGS[1]&MASK;
		switch((REGS[1]&ADDR)>>12){
			case MOV:	REGS[s8]=REGS[s4]; break;
			case INC:	REGS[s8]=REGS[s4]+1; break;
			case ADD:	REGS[s8]=REGS[s4]+REGS[s0]; break;
			case ADDI:	REGS[s8]=REGS[s4]+REGS[s0]+1; break;
			case DEC:c=	REGS[s8]=(t=REGS[s4])-1;			REGS[6]=(c)? ((c<0)? 1:0):2; break;
			case SUB:c= REGS[s8]=(t=REGS[s4])-REGS[s0];		REGS[6]=(c)? ((c<t)? 1:0):2; break;
			case SUBD:c=REGS[s8]=(t=REGS[s4])-REGS[s0]-1;	REGS[6]=(c)? ((c<t)? 1:0):2; break;
			case NOT:	REGS[s8]=~REGS[s4]; break;
			case OR:	REGS[s8]=REGS[s4]|REGS[s0]; break;
			case AND:	REGS[s8]=REGS[s4]&REGS[s0]; break;
			case MUL:	REGS[s8]=REGS[s4]*REGS[s0]; break;
			case DIV:	REGS[s8]=REGS[s4]/REGS[s0]; break;
			case JE:	if(REGS[6]&2) REGS[s8]=REGS[s4]; break;
			case JL:	if(REGS[6]&1) REGS[s8]=REGS[s4]; break;
			case JG:	if(!REGS[6]&1) REGS[s8]=REGS[s4]; break;
			case JNE:	if(!(REGS[6]&2)) REGS[s8]=REGS[s4]; break;
			default: break;
		}if(k==REGS[2]) data[REGS[2]]=REGS[3];
		return REGS[5]<=size;
	}
	
	int program::operator[](long unsigned int r){
		return REGS[r];
	}
	
	std::vector<int> program::show_RAM(bool a){
		if(a) return std::vector<int>(data,data+128);
		return std::vector<int>(code, code+size);
	}
	
	program::~program(){
		if(code) delete [] code;
		if(data) delete [] data;
	}
}

#ifdef TESTING
#include <iostream>
#include <iomanip>
int main(){
	emu::program RUNNER("hello");
	std::cout<<"Начальная память: ";
	for(auto i:RUNNER.show_RAM(1)) std::cout<<i<<" ";; std::cout<<"\n";
	std::cout<<"Существующие операции по нумерации:\n";
	std::cout<<"MOV: "<<std::hex<<MOV<<"\t";std::cout<<"INC: "<<std::hex<<INC<<"\t";
	std::cout<<"ADD: "<<std::hex<<ADD<<"\t";std::cout<<"ADDI:"<<std::hex<<ADDI<<"\n\n";
	std::cout<<"JNE: "<<std::hex<<JNE<<"\t";std::cout<<"JL:  "<<std::hex<<JL<<"\t";
	std::cout<<"JG:  "<<std::hex<<JG<<"\t";std::cout<<"JE:  "<<std::hex<<JE<<"\n\n";
	std::cout<<"MUL: "<<std::hex<<MUL<<"\t";std::cout<<"DIV: "<<std::hex<<DIV<<"\t";
	std::cout<<"NOT: "<<std::hex<<NOT<<"\t";std::cout<<"OR:  "<<std::hex<<OR<<"\n\n";
	std::cout<<"SUBD:"<<std::hex<<SUBD<<"\t";std::cout<<"SUB: "<<std::hex<<SUB<<"\t";
	std::cout<<"DEC: "<<std::hex<<DEC<<"\t";std::cout<<"AND: "<<std::hex<<AND<<"\n\n";
	std::cout<<"Регистры\n";
	std::cout<<std::setw(8)<<"OUT"<<std::setw(8)<<"IR"<<std::setw(8)<<"MAR"<<
				std::setw(8)<<"MDR"<<std::setw(8)<<"AC"<<std::setw(8)<<"PC"<<std::setw(8)<<"F"<<"\n";
	
	while(RUNNER()) if(RUNNER[0]) std::cout<<"output: "<<std::dec<<RUNNER[0]<<"\n";
	std::cout<<"Дикая память: ";
	for(auto i:RUNNER.show_RAM(1)) std::cout<<i<<" ";; std::cout<<"\n";
	for(auto i:RUNNER.show_RAM(0)) std::cout<<i<<" ";; std::cout<<"\n";
}
#endif

