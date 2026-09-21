#include "parser.hpp"
#include <unordered_map>
#include <vector>
#include <fstream>
#include <iostream>
#include <iomanip>

namespace emu{
	std::unordered_map<std::string, int> operators={
		{"LOAD", 0x04000000}, {"STOR", 0x00400000}, {"INC",  0x14000000}, {"ADD",  0x24400000},
		{"ADDI", 0x34400000}, {"JNE",  0x45200000}, {"JL",   0x55200000}, {"JG",   0x65200000},
		{"JE",   0x75200000}, {"MUL",  0x84400000}, {"DIV",  0x94400000}, {"NOT",  0xa4000000},
		{"OR",   0xb4400000}, {"SUBD", 0xc4400000}, {"SUB",  0xd4400000}, {"DEC",  0xe4000000},
		{"AND",  0xf4400000}, {"JMP",  0x05200000}, {"CLS",  0x04200000}
	}, regs={
		{"OUT", 0x1}, {"IR",0x2}, {"MAR",0x3}, {"MDR",0x4}, {"AC",0x5}, {"PC",0x6}
	};
	
	int check_address(std::string addr){
		if(addr.size()>10) return 1;
		if(addr[0]!='0') return 2;
		int error_code=0;
		for(unsigned int i=2; i<addr.size(); i++){
			switch(addr[1]){
				case 'x':
					switch(addr[i]){
						case '0' ... '9': case 'A' ... 'F': case 'a' ... 'f': break;
						default: return 10;
					}break;
				case 'b':
					switch(addr[i]){
						case '0' ... '1': break;
						default: return 11;
					}break;
				case 'd':
					switch(addr[i]){
						case '0' ... '9': break;
						default: return 12;
					}break;
				case '0' ... '7':
					switch(addr[i]){
						case '0' ... '7': break;
						default: return 13;
					}break;
				default: return 100;
			}
		}return error_code;
	}
	
	std::vector<std::string> breakdown(std::string t){
		std::vector<std::string> out={""}, err={""};
		for(unsigned int c=0, i=0; i<t.size(); i++){
			switch(t[i]){
				case ';': goto END; break;
				case ' ': out.push_back(""); c++; break;
				default: out[c]+=t[i]; break;
			}
		}END: if(out.size()>2||(out.size()>1&&(operators[out[0]]==operators["CLS"]))) err.push_back("TOO MANY OPERANDS");
		if(!operators[out[0]]) err.push_back("OPERATOR "+out[0]+" NOT INCLUDED");
		int error;
		if(operators["CLS"]==operators[out[0]]) goto FIN;
		if(!regs[out[1]]&&(error=check_address(out[1]))) err.push_back("INCORRECT OPERAND: "+out[1]+"; ERROR CODE: "+std::to_string(error));
		FIN: if(err.size()>1) return err;
		return out;
	}
	
	int assemble(std::vector<std::string> t){
		if(!t[0].size()) return 0;
		int out=operators[t[0]], posa=0, res=3, reg, base=10;
		switch((out>>28)&0xF){
			case 0x0: case 0x1: case 0xa: case 0xe: posa=20; break;
			case 0x4 ... 0x7: res=0; posa=0; break;
			default: posa=16; break;
		}if(out==0x00400000) posa=24;
		if(out==0x05200000) res=0;
		if(t[0]=="CLS") return out;
		if(reg=regs[t[1]]) { reg--; out|=(reg<<posa); return out; }
		//std::cout<<std::setfill(' ')<<t[1]<<"\n";
		switch(t[1][1]){
			case 'x': base=16; break;
			case 'b': base=2; break;
			case '0' ... '7': base=8; break;
			default: base=10; break;
		}std::cout<<posa<<"\n";
		reg=std::stoi(t[1],0,16); out|=(res<<posa); out|=reg; return out;
	}
	
	std::string parse(std::string file){
		std::ifstream F(file); std::string error_list="";
		int line=0; bool flawed=0;
		std::vector<int> output; std::vector<std::string> p;
		for(std::string t; getline(F,t); line++){ //std::cout<<"line: "<<line<<";";
			//std::cout<<"value "<<t<<"\n";
			if(!flawed&&(p=breakdown(t))[0].size()) { //std::cout<<"check\n";
				output.push_back(assemble(p));
				//std::cout<<"check\n";
			}
			else{ flawed=1; error_list+=std::to_string(line); for(auto i:p) error_list+=i+"\n"; }
		}F.close(); if(flawed) return error_list;
		std::ofstream W(file+".prg",std::ios::binary);
		W.write(reinterpret_cast<char*>(&output[0]),output.size()*4);
		W.close();
		for(auto i:output) std::cout<<std::setfill('0')<<std::setw(8)<<std::hex<<i<<"\n";
		return "successfully assembled\n";
	}
}

//#ifdef TESTING

int main(){
	std::cout<<emu::parse("hello");
}
//#endif
