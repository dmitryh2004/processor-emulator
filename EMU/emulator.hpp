#pragma once
#include <string>
#include <vector>

namespace emu{
	class program{
		int size=0, 
			*code, 
			*data=new int[128], //size, ROM, RAM
			REGS[7]={0,0,0,0,0,0,0}; //OUT, IR, MAR, MDR, AC, PC, F
	public:
		program(std::string); /**<Стандартный конструктор, загружает программу и память в объект. @param std::string - имена файлов ROM и RAM с которыми будет работать программа.*/
		int operator[](long unsigned int); /**<Возвращает Выбранный регистр от 0 до 6. @param "long unsigned int" - Регистры: вывод (0), инструкция (1), адрес (2), ячейка (3), аккумулятор (4), счетчик (5), флаги (6).*/
		bool operator()(); /**<Прогон 1 такта. Возвращает 1 если это не последняя команда.*/
		std::vector<int> show_RAM(bool); /**<Возвращает память в форме std::vector<int>. @param bool - выводит полностью память ROM (0) или RAM (1)*/
		~program(); /**<Уничтожает загруженные программы.*/
	};
}
