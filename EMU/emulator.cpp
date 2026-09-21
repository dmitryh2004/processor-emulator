#include "emulator.hpp"
#include <fstream>
#include <iostream>
#include <cmath>
#include <iomanip>

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
	int table[128]={
		0,0,0,0,0,0,0,0,//7
		0,0,0,0,0,0,0,0,//15
		0,0,0,0,0,0,0,0,//23
		0,0,0,0,0,0,0,0,//31
		0,0,0,0,0,0,0,0,//39
		0,0,0,0,0,0,0,0,//47
		0,1,2,3,4,5,6,7,//55
		8,9,0,0,0,0,0,0,//63
		0,10,11,12,13,14,15,0,//71
		0,0,0,0,0,0,0,0,//79
		0,0,0,0,0,0,0,0,//87
		0,0,0,0,0,0,0,0,//95
		0,10,11,12,13,14,15,0,
		0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0,
		0,0,0,0,0,0,0,0
	}, size=0; int *program, *data=new int[128]; 
	char elbat[16]={'0','1','2','3','4','5','6','7','8','9','A','B','C','D','E','F'};
	
	int to_int(char* input){ int output=0;
		for(unsigned int i=0; i<8; i++) { output*=16; output+=table[input[i]]; }
		//std::cout<<input<<"\n";
		return output;
	}
	
	char* to_cod(int p){char *output=new char[8];
		for(unsigned int i=0,j=7;i<8;i++,j--) {output[j]=elbat[p/(int(std::pow(16,i)))%(int(std::pow(16,i+1)))];}
		return output;
	}
	
	int* instructions(char* tt){ int *output=new int[4];
		for(unsigned int i=0; i<8; i++) {output[i/2]*=16; output[i/2]+=table[tt[i]]; }
		return output;
	}
	
	void load_program(std::string file){
		std::ifstream F(file+".prg", std::ios::ate | std::ios::binary); if(!F) return;
		size=F.tellg(); F.seekg(0); program=new int[size];
		F.read(reinterpret_cast<char*>(program),size); F.close(); size/=4;
		F.open(file+".dat", std::ios::ate | std::ios::binary); if(!F) return;
		int s=F.tellg(); F.seekg(0); F.read(reinterpret_cast<char*>(data), s); F.close();
	}
	
	std::string run_program(){
		//int outr, inpr, *ir, ira, mdr, mar, ac, pc;
		int REGS[7]={0,0,0,0,0,0,0}, t, c, lim=10; //OUT, IR, MAR, MDR, AC, PC, F
		//char copy[8], *soup;//[AA][LL][BB][FF][1MEMADDR]
		while(REGS[5]<size){ if(!REGS[5]) lim--;
			REGS[1]=program[REGS[5]];
			REGS[5]++;
			REGS[2]=REGS[1]&ADDR;REGS[1]>>=16;
			REGS[3]=data[REGS[2]];
			//std::cout<<std::hex<<((REGS[1]&ADDR)>>12)<<"\n";
			switch((REGS[1]&ADDR)>>12){
				case MOV: REGS[(REGS[1]>>8)&MASK]=REGS[(REGS[1]>>4)&MASK]; break;
				case INC: REGS[(REGS[1]>>8)&MASK]=REGS[(REGS[1]>>4)&MASK]+1; break;
				case ADD: REGS[(REGS[1]>>8)&MASK]=REGS[(REGS[1]>>4)&MASK]+REGS[REGS[1]&MASK]; break;
				case ADDI: REGS[(REGS[1]>>8)&MASK]=REGS[(REGS[1]>>4)&MASK]+REGS[REGS[1]&MASK]+1; break;
				case DEC: c=REGS[(REGS[1]>>8)&MASK]=(t=REGS[(REGS[1]>>4)&MASK])-1; REGS[6]=(c)? ((c<0)? 1:0):2; break;
				case SUB: c=REGS[(REGS[1]>>8)&MASK]=(t=REGS[(REGS[1]>>4)&MASK])-REGS[REGS[1]&MASK]; REGS[6]=(c)? ((c<t)? 1:0):2; break;
				case SUBD: c=REGS[(REGS[1]>>8)&MASK]=(t=REGS[(REGS[1]>>4)&MASK])-REGS[REGS[1]&MASK]-1; REGS[6]=(c)? ((c<t)? 1:0):2; break;
				case NOT: REGS[(REGS[1]>>8)&MASK]=~REGS[(REGS[1]>>4)&MASK]; break;
				case OR: REGS[(REGS[1]>>8)&MASK]=REGS[(REGS[1]>>4)&MASK]|REGS[REGS[1]&MASK]; break;
				case AND: REGS[(REGS[1]>>8)&MASK]=REGS[(REGS[1]>>4)&MASK]&REGS[REGS[1]&MASK]; break;
				case MUL: REGS[(REGS[1]>>8)&MASK]=REGS[(REGS[1]>>4)&MASK]*REGS[REGS[1]&MASK]; break;
				case DIV: REGS[(REGS[1]>>8)&MASK]=REGS[(REGS[1]>>4)&MASK]/REGS[REGS[1]&MASK]; break;
				case JE: if(REGS[6]&0b10) REGS[(REGS[1]>>8)&MASK]=REGS[(REGS[1]>>4)&MASK]; break;
				case JL: if(REGS[6]&0b01) REGS[(REGS[1]>>8)&MASK]=REGS[(REGS[1]>>4)&MASK]; break;
				case JG: if(REGS[6]&0b01) REGS[(REGS[1]>>8)&MASK]=REGS[(REGS[1]>>4)&MASK]; break;
				case JNE: if(!(REGS[6]&0b10)) REGS[(REGS[1]>>8)&MASK]=REGS[(REGS[1]>>4)&MASK]; break;
				default: break;
			}for(unsigned int i=0; i<7; i++) std::cout<<std::setw(8)<<std::hex<<(REGS[i]&ADDR); std::cout<<"\n";
			//for(unsigned int i=0; i<6; i++) std::cout<<data[i]<<" "; std::cout<<"\n";
			//std::cout<<"C: "<<((REGS[1]>>8)&MASK)<<"\n";
			//std::cout<<"A: "<<((REGS[1]>>4)&MASK)<<"\n";
			//std::cout<<"B: "<<(REGS[1]&MASK)<<"\n";
			data[REGS[2]]=REGS[3];
			/*for(unsigned int i=0; i<8; i++) copy[i]=program[REGS[7]+i];
			std::cout<<copy<<"\n";
			ir=instructions(copy); REGS[7]+=8;
			for(unsigned int i=0; i<4; i++) std::cout<<ir[i]<<" "; std::cout<<"\n";
			for(unsigned int i=0; i<8; i++) copy[i]=program[REGS[7]+i];
			REGS[5]=to_int(copy);
			REGS[4]=REGS[5];
			
			switch(ir[1]){
				case 0: REGS[ir[0]]=REGS[ir[2]]; break;
				case 1: for(unsigned int i=0; i<8; i++) copy[i]=data[REGS[4]+i];
					REGS[3]=to_int(copy); break;
				case 2: soup=to_cod(REGS[3]);
					for(unsigned int i=0; i<8; i++) data[REGS[4]+i]=soup[i]; break;
				case 3: REGS[ir[0]]+=REGS[ir[2]]; break;
				case 4: REGS[ir[0]]-=REGS[ir[2]]; break;
				case 5: REGS[ir[0]]*=REGS[ir[2]]; break;
				case 6: REGS[ir[0]]/=REGS[ir[2]]; break;
				default: break;
			}
			
			REGS[7]+=8;
			for(unsigned int i=0;i<8;i++) std::cout<<REGS[i]<<" "; std::cout<<"\n";*/
		}return "output data: "+std::to_string(REGS[0]);
	}
	void show_insides(bool a){
		for(unsigned int i=0; i<6; i++) std::cout<<data[i]<<" "; std::cout<<"\n";
	}
}

//#ifdef TESTING
int main(){ 
	emu::load_program("hello");
	std::cout<<"Начальная память: ";
	emu::show_insides(0);
	std::cout<<"Существующие операции по нумерации:\n";
	std::cout<<"MOV: "<<std::hex<<MOV<<"\t";
	std::cout<<"INC: "<<std::hex<<INC<<"\t";
	std::cout<<"ADD: "<<std::hex<<ADD<<"\t";
	std::cout<<"ADDI:"<<std::hex<<ADDI<<"\n\n";
	std::cout<<"JNE: "<<std::hex<<JNE<<"\t";
	std::cout<<"JL:  "<<std::hex<<JL<<"\t";
	std::cout<<"JG:  "<<std::hex<<JG<<"\t";
	std::cout<<"JE:  "<<std::hex<<JE<<"\n\n";
	std::cout<<"MUL: "<<std::hex<<MUL<<"\t";
	std::cout<<"DIV: "<<std::hex<<DIV<<"\t";
	std::cout<<"NOT: "<<std::hex<<NOT<<"\t";
	std::cout<<"OR:  "<<std::hex<<OR<<"\n\n";
	std::cout<<"SUBD:"<<std::hex<<SUBD<<"\t";
	std::cout<<"SUB: "<<std::hex<<SUB<<"\t";
	std::cout<<"DEC: "<<std::hex<<DEC<<"\t";
	std::cout<<"AND: "<<std::hex<<AND<<"\n\n";
	std::cout<<"Регистры\n";
	std::cout<<std::setw(8)<<"OUT"<<std::setw(8)<<"IR"<<std::setw(8)<<"MAR"<<
				std::setw(8)<<"MDR"<<std::setw(8)<<"AC"<<std::setw(8)<<"PC"<<std::setw(8)<<"F"<<"\n";
	std::cout<<emu::run_program()<<"\n";
	std::cout<<"Дикая память: ";
	emu::show_insides(0);
}

//#endif

