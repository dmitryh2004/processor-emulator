#pragma once
#include <string>
#include <fstream>
#include <iostream>
#include <cmath>
#include <iomanip>
#include <vector>
#include <sstream> // Добавлено, так как используется std::stringstream
#include "RAM.h"
#include <SFML/Graphics.hpp>

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

class Processor {
public:
	Processor(int ramSize = 65536)
	{
		ram = RAM(ramSize);
		program = std::vector<std::string>(0);
		machineCodeProgram = std::vector<unsigned int>(0);
		parsedSuccessfully = false;
	}

	// загрузить программу
	void LoadProgram(sf::String newProgram) {
		std::string newProgramString = newProgram.toAnsiString();
		program = splitString(newProgramString, '\n');
	}

	// выполнить парсинг
	bool ParseProgram() {
		parsedSuccessfully = false;
		return parsedSuccessfully;
	}

	void RunProgram() {
		// not implemented
	}

	void RunProgramOneStep() {
		// not implemented
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

	void Reset() {
		REGS[0] = REGS[1] = REGS[2] = REGS[3] = REGS[4] = REGS[5] = REGS[6] = 0;
		ram.reset();
		parsedSuccessfully = false;
		program = std::vector<std::string>(0);
		machineCodeProgram = std::vector<unsigned int>(0);
	}
private:
	unsigned int REGS[7]{ 0, 0, 0, 0, 0, 0, 0 };
	RAM ram;
	bool parsedSuccessfully = false;
	std::vector<unsigned int> machineCodeProgram;
	std::vector<std::string> program;

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
};
