#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <vector>
#include <sstream> // Добавлено, так как используется std::stringstream
#include "../SFML/LogText.h"
#include "../SFML/RandomSoundPlayer.h"
#include "RAM.h"
#include <SFML/Graphics.hpp>

#define MEMES

#define ADDR 0xFFFF
#define MASK 0xF

#define MOV 0b0000
#define INC 0b0001
#define ADD 0b0010
#define JC 0b0011
#define JNE 0b0100
#define JL 0b0101
#define JG 0b0110
#define JE 0b0111
#define MUL 0b1000
#define DIV 0b1001
#define NOT 0b1010
#define OR 0b1011
#define JNC 0b1100
#define SUB 0b1101
#define DEC 0b1110
#define AND 0b1111

class Processor {
public:
	Processor(int ramSize, std::shared_ptr<LogText> logFieldRef, RandomSoundPlayer& rsp) : ram(ramSize), logField(logFieldRef), randomSoundPlayer(rsp)
	{
		program = std::vector<std::string>(0);
		machineCodeProgram = std::vector<unsigned int>(0);
		parsedSuccessfully = false;
	}

	// загрузить программу
	void LoadProgram(sf::String newProgram) {
		std::string newProgramString = newProgram.toAnsiString();
		program = splitString(newProgramString, '\n');
		if (logField->getTextString().getSize()) {
			logField->appendLog("------------------------------");
		}
		logField->appendLog(GetRandomStringFromMap(startParsingPhrases));
	}

	// выполнить парсинг
	bool ParseProgram() {
		parsedSuccessfully = false;
		parsedProgram.clear();
		std::vector<std::string> errors = std::vector<std::string>();
		machineCodeProgram = std::vector<unsigned int>();

		if (!program.size()) {
			int randomIndex = Random::getInt(0, emptyProgramPhrases.size() - 1);
			std::string randomString = emptyProgramPhrases[randomIndex];
			logField->appendLog(randomString);
#ifdef MEMES
			randomSoundPlayer.Play(randomIndex);
#endif
			return false;
		}

		for (int i = 0; i < program.size(); i++) {
			std::string line = program.at(i);
			size_t pos = line.find_first_of(';');
			
			if (pos != std::string::npos) {
				// Удаляем всё от найденной позиции до конца строки
				line.erase(pos, line.size() - pos);
			}

			if (line.size() == 0) continue;

			std::vector<std::string> splitted = splitString(line, ' ');
			if ((splitted.size() > 2) || (splitted.size() > 1 && splitted[0] == "CLS")) {
				std::string error = "Строка " + std::to_string(i+1) + " - слишком много операндов\n" + line + "\n";
				errors.push_back(error);
				break;
			}

			std::string command = splitted[0];
			if (!operators[command]) {
				std::string error = "Строка " + std::to_string(i+1) + " - оператор " + command + " не существует\n" + line + "\n";
				errors.push_back(error);
				break;
			}

			if (command != "CLS") {
				std::string address = splitted[1];
				if (int errorCode = check_address(address)) {
					std::string error = "Строка " + std::to_string(i + 1) + " - некорректный адрес (код " + std::to_string(errorCode) + ")\n" + line + "\n";
					errors.push_back(error);
					break;
				}
			}
			

			machineCodeProgram.push_back(convertToMachineCode(splitted));
			parsedProgram.push_back(line);
		}

		// логи
		if (errors.size() > 0) {
			parsedSuccessfully = false;
			machineCodeProgram.clear();
			parsedProgram.clear();

			int randomIndex = Random::getInt(0, parsingFailedPhrases.size() - 1);
			std::string totalError = parsingFailedPhrases[randomIndex];
			for (int i = 0; i < errors.size(); i++) {
				totalError += "\n" + errors[i];
			}

			if (logField != nullptr)
				logField->appendLog(totalError);

#ifdef MEMES
			randomSoundPlayer.Play(randomIndex + emptyProgramPhrases.size());
#endif
		}
		else {
			parsedSuccessfully = true;
			for (int i = 0; i < 7; i++) {
				REGS[i] = 0;
			}

			std::string successText = GetRandomStringFromMap(parsingSuccessPhrases);
			
			/*
			for (int i = 0; i < machineCodeProgram.size(); i++) {
				successText.append(std::format("\n0x{:08X}", machineCodeProgram[i]));
			} 
			*/

			logField->appendLog(successText);
		}
		
		return parsedSuccessfully;
	}

	bool RunProgramOneStep() {
		if (!parsedSuccessfully || (REGS[5] >= machineCodeProgram.size())) return false;

		int t, c, k;
		REGS[1] = machineCodeProgram[REGS[5]];
		REGS[5]++;
		k = REGS[1] & ADDR;
		if (k) REGS[2] = k; 
		// k = (REGS[2] = (k) ? k : REGS[2]);

		int instruction = REGS[1] >> 16;
		REGS[3] = ram.read(REGS[2]);

		int s8 = (instruction >> 8) & MASK, s4 = (instruction >> 4) & MASK, s0 = instruction & MASK;

		// Вспомогательные переменные для точного вычисления флага CF (без потери старшего бита)
		unsigned int op1, op2, res_u;

		switch ((instruction & ADDR) >> 12) {
		case MOV:
			if (s4 == 2) REGS[2] = k;
			if (s8 == 5)
				REGS[s8] = REGS[s4] - 1;
			else
				REGS[s8] = REGS[s4];
			break;

		case INC:
			op1 = (unsigned int)REGS[s4];
			res_u = op1 + 1;
			c = REGS[s8] = (int)res_u;

			// Выставляем флаги: CF(4) если был перенос, ZF(2) если ноль, SF(1) если c < 0
			REGS[6] = ((res_u < op1) ? 4 : 0) | ((c == 0) ? 2 : 0) | ((c < 0) ? 1 : 0);
			break;

		case ADD:
			op1 = (unsigned int)REGS[s4];
			op2 = (unsigned int)REGS[s0];
			res_u = op1 + op2;
			c = REGS[s8] = (int)res_u;

			REGS[6] = ((res_u < op1) ? 4 : 0) | ((c == 0) ? 2 : 0) | ((c < 0) ? 1 : 0);
			break;

		case DEC:
			op1 = (unsigned int)REGS[s4];
			res_u = op1 - 1;
			c = REGS[s8] = (int)res_u;

			// При вычитании CF означает заем (borrow), т.е. когда вычитаемое больше уменьшаемого
			REGS[6] = ((op1 < 1) ? 4 : 0) | ((c == 0) ? 2 : 0) | ((c < 0) ? 1 : 0);
			break;

		case SUB:
			op1 = (unsigned int)REGS[s4];
			op2 = (unsigned int)REGS[s0];
			res_u = op1 - op2;
			c = REGS[s8] = (int)res_u;

			REGS[6] = ((op1 < op2) ? 4 : 0) | ((c == 0) ? 2 : 0) | ((c < 0) ? 1 : 0);
			break;

		case NOT:	REGS[s8] = ~REGS[s4]; break;
		case OR:	REGS[s8] = REGS[s4] | REGS[s0]; break;
		case AND:	REGS[s8] = REGS[s4] & REGS[s0]; break;
		case MUL:	REGS[s8] = REGS[s4] * REGS[s0]; break;
		case DIV:	REGS[s8] = REGS[s4] / REGS[s0]; break;

			// Логика переходов: исправлена приоритетность оператора !
		case JE:	if (REGS[6] & 2) REGS[s8] = REGS[s4] - 1; break;
		case JL:	if (REGS[6] & 1) REGS[s8] = REGS[s4] - 1; break;
		case JG:	if (!(REGS[6] & 1)) REGS[s8] = REGS[s4] - 1; break;
		case JNE:	if (!(REGS[6] & 2)) REGS[s8] = REGS[s4] - 1; break;

		case JC:	if (REGS[6] & 4) REGS[s8] = REGS[s4] - 1; break;
		case JNC:	if (!(REGS[6] & 4)) REGS[s8] = REGS[s4] - 1; break;

		default: break;
		}
		if (k == REGS[2]) ram.write(REGS[2], REGS[3]);

		return REGS[5] < machineCodeProgram.size();
	}


	unsigned int GetCurrentMachineCodeOperation() {
		return (0 <= REGS[5] && REGS[5] < machineCodeProgram.size()) ? machineCodeProgram[REGS[5]] : 0;
	}

	std::string GetCurrentCommand() {
		if (!(0 <= REGS[5] && REGS[5] < parsedProgram.size())) return "";
		std::string commandStr = splitString(parsedProgram[REGS[5]], ' ')[0];
		return commandStr;
	}

	unsigned int GetRegisterValue(int index) {
		if (index < 0 || index > 6) {
			throw std::out_of_range("Регистра с индексом " + std::to_string(index) + " не существует.");
		}
		return REGS[index];
	}

	// Метод для получения неконстантной ссылки на RAM (позволяет модифицировать память)
	RAM& GetRAM() {
		return ram;
	}

	// Перегрузка метода для получения константной ссылки на RAM (для безопасного чтения)
	const RAM& GetRAM() const {
		return ram;
	}

	void SetLogField(std::shared_ptr<LogText> logFieldRef) { 
		logField = logFieldRef;
	};

	void Reset() {
		REGS[0] = REGS[1] = REGS[2] = REGS[3] = REGS[4] = REGS[5] = REGS[6] = 0;
		// ram.reset();
		parsedSuccessfully = false;
		program = std::vector<std::string>(0);
		machineCodeProgram = std::vector<unsigned int>(0);
	}
private:
	std::unordered_map<std::string, int> operators = {
		{"LOAD", 0x04000000}, {"STOR", 0x00400000}, {"INC",  0x14000000}, {"ADD",  0x24400000},
		{"JC", 0x35200000}, {"JNE",  0x45200000}, {"JL",   0x55200000}, {"JG",   0x65200000},
		{"JE",   0x75200000}, {"MUL",  0x84400000}, {"DIV",  0x94400000}, {"NOT",  0xa4000000},
		{"OR",   0xb4400000}, {"JNC", 0xc5200000}, {"SUB",  0xd4400000}, {"DEC",  0xe4000000},
		{"AND",  0xf4400000}, {"JMP",  0x05200000}, {"CLS",  0x04200000}
	}, regs = {
		{"OUT", 0x1}, {"IR",0x2}, {"MAR",0x3}, {"MDR",0x4}, {"AC",0x5}, {"PC",0x6}
	};

#ifdef MEMES
	std::map<int, std::string> startParsingPhrases = { 
		{0, "Опять ассемблер? Какое у нас там стоп-слово..."},
		{1, "Опять работать? Тебе меня не жалко?"},
		{2, "Может быть, сам на листочке скомпилируешь? Ну ладно, ладно, так уж и быть..."}
	};
	std::map<int, std::string> emptyProgramPhrases = { 
		{0, "Чтобы запустить программу на процессоре, ее надо сначала написать, наверное? Сам-то как считаешь?"},
		{1, "Нет ТЗ - результат хз\n(с) Конфуций, 485 г. до н.э."},
		{2, "По-моему, ты забыл написать ассемблерный код. Или я его телепатией должен получить?"},
		{3, "Код, видимо, это алмаз из \"Большого куша\"..."},
		{4, "Ну, бездействие - это тоже в каком-то роде действие..."}
	};
	std::map<int, std::string> parsingFailedPhrases = { 
		{0, "Если бы мне платили по одному рублю за каждую ошибку в ассемблере, я бы уже разбогател. Что тут не так:"},
		{1, "Похоже, кто-то не читал методичку по ассемблеру этого процессора:"},
		{2, "Прежде чем записывать сюда код, убедись, что он написан правильно:"},
		{3, "Ты пишешь код на моем ассемблере, но делаешь это без знания синтаксиса:"}
	};
	std::map<int, std::string> parsingSuccessPhrases = {
		{0, "Ну, похоже на правду. А вот работает ли это, это уже другой вопрос"},
		{1, "Ладно, методичку по коду ты читал. На работу программы сейчас посмотрим..."}
	};
#else
	std::map<int, std::string> startParsingPhrases = { {0, "Начинаем парсинг программы..."}};
	std::map<int, std::string> emptyProgramPhrases = { {0, "Парсинг невозможен: программа пуста"} };
	std::map<int, std::string> parsingFailedPhrases = { {0, "При парсинге найдены следующие ошибки: "} };
	std::map<int, std::string> parsingSuccessPhrases = { {0, "Парсинг программы выполнен успешно!"} };
#endif

	unsigned int REGS[7]{ 0, 0, 0, 0, 0, 0, 0 };
	RAM ram;
	std::shared_ptr<LogText> logField;
	bool parsedSuccessfully = false;
	std::vector<unsigned int> machineCodeProgram;
	std::vector<std::string> program, parsedProgram;
	RandomSoundPlayer& randomSoundPlayer;

	std::string GetRandomStringFromMap(std::map<int, std::string>& map) {
		return map[Random::getInt(0, map.size() - 1)];
	}

	std::vector<std::string> splitString(const std::string& input, char delimiter) {
		std::vector<std::string> tokens;
		std::stringstream ss(input); // Создаём поток из строки
		std::string token;

		// Извлекаем токены, разделённые delimiter, и добавляем в вектор
		while (getline(ss, token, delimiter)) {
			tokens.push_back(token);
		}

		return tokens;
	}

	int check_address(std::string addr) {
		if (regs.contains(addr)) return 0;
		if (addr.size() > 10) return 1; 
		if (addr[0] != '0') return 2; 
		int error_code = 0;
		for (unsigned int i = 2; i < addr.size(); i++) {
			switch (addr[1]) {
			case 'x':
				if (((addr[i] < '0') || (addr[i] > '9')) && 
					((addr[i] < 'A') || (addr[i] > 'F')) && 
					((addr[i] < 'a') || (addr[i] > 'f'))) {
					return 10;
				}
				break;
			case 'b':
				if (((addr[i] < '0') || (addr[i] > '1'))) {
					return 11;
				}
				break;
			case 'd':
				if ((addr[i] < '0') || (addr[i] > '9')) {
					return 12;
				}
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4': 
			case '5':
			case '6':
			case '7':
				if ((addr[i] < '0') || (addr[i] > '7')) {
					return 13;
				}
				break;
			default: 
				return 100;
			}
		}
		return error_code;
	}

	uint32_t convertToMachineCode(std::vector<std::string> command) {
		if (!command[0].size()) return 0;
		uint32_t out = operators[command[0]], posa = 0, res = 3, reg;
		
		// Выделяем старшую тетраду для общего разделения
		uint32_t op_group = (out >> 28) & 0xF;
		// Выделяем код операции (например, 0x052 для JMP)
		uint32_t op_code = (out >> 20) & 0xFF;

		// Если это JMP (у него op_code == 0x05) или другие переходы (0x45, 0x55 и т.д.)
		if (op_group >= 0x4 && op_group <= 0x7 || op_code == 0x52) {
			res = 0;
			posa = 0;
		}
		else {
			switch (op_group) {
			case 0x0:
			case 0x1:
			case 0xa:
			case 0xe:
				posa = 20;
				break;
			default:
				posa = 16;
				break;
			}
		}

		if (out == 0x00400000) 
			posa = 24; 
		if (out == 0xe5200000) 
			res = 0;
		if (command[0] == "CLS") 
			return out; 
		
		if ((reg = regs[command[1]])) 
		{ 
			reg--; 
			out |= (reg << posa); 
			return out; 
		}

		int base; 
		switch (command[1][1]) {
			case 'x':
				base = 16;
				break;
			case 'b': 
				base = 2; 
				break;
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
				base = 8; 
				break;
			default: 
				base = 10; 
				command[1] = command[1].substr(2, command[1].size() - 2); 
				break;
		}
		reg = std::stoi(command[1], 0, base); 
		out |= (res << posa); 
		out |= reg;
		return out;
	}
};