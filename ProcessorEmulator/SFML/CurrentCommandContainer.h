#pragma once
#include "BaseObject.h"
#include <iomanip>
#include <sstream>
#include <string>
#include <algorithm>

#include "Text.h" 
#include "Utilites/SFMLUtility.h"

using namespace SFMLUtility;

const char hexDigits[] = "0123456789ABCDEF";

const sf::String assemblerOperations[] = {
	"MOV", "INC", "ADD", "ADDI", "JNE", "JL", "JG", "JE",
	"MUL", "DIV", "OR", "NOT", "SUBD", "SUB", "DEC", "AND"
};

const sf::String assemblerCommandDescriptions[] = {
	"Команда LOAD загружает в регистр AC значение из ячейки памяти с переданным адресом."_sf,
	"Команда STOR загружает в ячейку памяти с переданным адресом значение регистра AC."_sf,
	"Команда ADD прибавляет к значению регистра AC значение из ячейки памяти с переданным адресом, и сохраняет результат в регистре AC."_sf,
	"Команда ADDI прибавляет к значению регистра AC значение из ячейки памяти с переданным адресом, увеличенное на 1, и сохраняет результат в регистре AC."_sf,
	"Команда INC прибавляет 1 к значению из ячейки памяти с переданным адресом, и сохраняет результат в регистре AC."_sf,
	"Команда SUB отнимает от значения регистра AC значение из ячейки памяти с переданным адресом, и сохраняет результат в регистре AC."_sf,
	"Команда SUBD отнимает от значения регистра AC значение из ячейки памяти с переданным адресом, а затем - еще 1, и сохраняет результат в регистре AC."_sf,
	"Команда DEC отнимает 1 от значения из ячейки памяти с переданным адресом, и сохраняет результат в регистре AC."_sf,
	"Команда MUL умножает значение регистра AC на значение из ячейки памяти с переданным адресом, и сохраняет результат в регистре AC."_sf,
	"Команда DIV делит нацело значение регистра AC на значение из ячейки памяти с переданным адресом, и сохраняет результат в регистре AC."_sf,
	"Команда AND побитово умножает значение регистра AC на значение из ячейки памяти с переданным адресом, и сохраняет результат в регистре AC."_sf,
	"Команда OR побитово суммирует значение регистра AC и значение из ячейки памяти с переданным адресом, и сохраняет результат в регистре AC."_sf,
	"Команда NOT побитово инвертирует значение из ячейки памяти с переданным адресом, и сохраняет результат в регистре AC."_sf,
	"Команда JMP записывает в регистр PC переданное значение, позволяя таким образом реализовать безусловные переходы в программе."_sf,
	"Команда JE записывает в регистр PC переданное значение, если сейчас поднят флаг ZF. Это позволяет реализовать условные переходы в программе."_sf,
	"Команда JL записывает в регистр PC переданное значение, если сейчас поднят флаг SF. Это позволяет реализовать условные переходы в программе."_sf,
	"Команда JG записывает в регистр PC переданное значение, если сейчас опущен флаг SF. Это позволяет реализовать условные переходы в программе."_sf,
	"Команда JNE записывает в регистр PC переданное значение, если сейчас опущен флаг ZF. Это позволяет реализовать условные переходы в программе."_sf,
	"Команда CLS устанавливает значение регистра AC равным 0."_sf
};

const sf::String assemblerCommands[] = {
	"LOAD", "STOR", "ADD", "ADDI", "INC", "SUB", "SUBD", "DEC",
	"MUL", "DIV", "AND", "OR", "NOT", "JMP", "JE", "JL", "JG", "JNE", "CLS"
};

class CurrentCommandContainer : public BaseObject {
public:
	CurrentCommandContainer(std::string name,
		const sf::Font& font,
		unsigned int characterSize,
		sf::Vector2f size,
		sf::Vector2f parentSize,
		sf::Color activeColor = sf::Color::White,
		sf::Color notActiveColor = sf::Color::Color(128, 128, 128),
		sf::Vector2f offset = { 0.f, 0.f },
		Anchor parentAnchor = Anchor::TopLeft,
		Anchor localAnchor = Anchor::TopLeft)
		: BaseObject(name, size, parentSize, offset, parentAnchor, localAnchor) 
	{
		isActive = false;

		notActiveText = std::make_unique<Text>(name + "_NotActive", font, size, 
			"Запустите программу по шагам, чтобы просмотреть текущую команду!"_sf,
			characterSize, notActiveColor, true, sf::Vector2f(10.f, 10.f),
			BaseObject::Anchor::TopLeft, 
			BaseObject::Anchor::TopLeft);

		// keys

		machineCodeKey = std::make_unique<Text>(name + "_key_mc", font, size,
			"Машинный код"_sf,
			characterSize, activeColor, true, sf::Vector2f(10.f, 10.f),
			BaseObject::Anchor::TopLeft,
			BaseObject::Anchor::TopLeft);

		assemblerCommandKey = std::make_unique<Text>(name + "_key_ac", font, size,
			"Команда ассемблера"_sf,
			characterSize, activeColor, true, sf::Vector2f(10.f, 30.f),
			BaseObject::Anchor::TopLeft,
			BaseObject::Anchor::TopLeft);

		assemblerDecodedHeader = std::make_unique<Text>(name + "_header_ad", font, size,
			"Расшифровка команды:"_sf,
			characterSize, activeColor, true, sf::Vector2f(10.f, 70.f),
			BaseObject::Anchor::TopLeft,
			BaseObject::Anchor::TopLeft);

		assemblerOperationKey = std::make_unique<Text>(name + "_key_ao", font, size,
			"Операция ассемблера"_sf,
			characterSize, activeColor, true, sf::Vector2f(10.f, 95.f),
			BaseObject::Anchor::TopLeft,
			BaseObject::Anchor::TopLeft);

		destinationKey = std::make_unique<Text>(name + "_key_d", font, size,
			"Запись"_sf,
			characterSize, activeColor, true, sf::Vector2f(10.f, 115.f),
			BaseObject::Anchor::TopLeft,
			BaseObject::Anchor::TopLeft);

		opAKey = std::make_unique<Text>(name + "_key_opA", font, size,
			"Операнд А"_sf,
			characterSize, activeColor, true, sf::Vector2f(10.f, 135.f),
			BaseObject::Anchor::TopLeft,
			BaseObject::Anchor::TopLeft);

		opBKey = std::make_unique<Text>(name + "_key_opB", font, size,
			"Операнд B"_sf,
			characterSize, activeColor, true, sf::Vector2f(10.f, 155.f),
			BaseObject::Anchor::TopLeft,
			BaseObject::Anchor::TopLeft);

		opAddrKey = std::make_unique<Text>(name + "_key_opAddr", font, size,
			"Адрес"_sf,
			characterSize, activeColor, true, sf::Vector2f(10.f, 175.f),
			BaseObject::Anchor::TopLeft,
			BaseObject::Anchor::TopLeft);

		descriptionKey = std::make_unique<Text>(name + "_key_descr", font, size,
			"Описание"_sf,
			characterSize, activeColor, true, sf::Vector2f(10.f, 215.f),
			BaseObject::Anchor::TopLeft,
			BaseObject::Anchor::TopLeft);

		// values

		machineCodeValue = std::make_unique<Text>(name + "_value_mc", font, size,
			"0000 0000 0000 0000"_sf,
			characterSize, activeColor, true, sf::Vector2f(10.f, 10.f),
			BaseObject::Anchor::TopCenter,
			BaseObject::Anchor::TopLeft);

		assemblerCommandValue = std::make_unique<Text>(name + "_value_ac", font, size,
			"STOR"_sf,
			characterSize, activeColor, true, sf::Vector2f(10.f, 30.f),
			BaseObject::Anchor::TopCenter,
			BaseObject::Anchor::TopLeft);

		assemblerOperationValue = std::make_unique<Text>(name + "_value_ao", font, size,
			"MOV"_sf,
			characterSize, activeColor, true, sf::Vector2f(10.f, 95.f),
			BaseObject::Anchor::TopCenter,
			BaseObject::Anchor::TopLeft);

		destinationValue = std::make_unique<Text>(name + "_value_d", font, size,
			"3"_sf,
			characterSize, activeColor, true, sf::Vector2f(10.f, 115.f),
			BaseObject::Anchor::TopCenter,
			BaseObject::Anchor::TopLeft);

		opAValue = std::make_unique<Text>(name + "_value_opA", font, size,
			"4"_sf,
			characterSize, activeColor, true, sf::Vector2f(10.f, 135.f),
			BaseObject::Anchor::TopCenter,
			BaseObject::Anchor::TopLeft);

		opBValue = std::make_unique<Text>(name + "_value_opB", font, size,
			"0"_sf,
			characterSize, activeColor, true, sf::Vector2f(10.f, 155.f),
			BaseObject::Anchor::TopCenter,
			BaseObject::Anchor::TopLeft);

		opAddrValue = std::make_unique<Text>(name + "_value_opAddr", font, size,
			"0x4"_sf,
			characterSize, activeColor, true, sf::Vector2f(10.f, 175.f),
			BaseObject::Anchor::TopCenter,
			BaseObject::Anchor::TopLeft);

		descriptionValue = std::make_unique<Text>(name + "_value_descr", font, size,
			"Команда MOV что-то делает"_sf,
			characterSize, activeColor, true, sf::Vector2f(10.f, 235.f),
			BaseObject::Anchor::TopLeft,
			BaseObject::Anchor::TopLeft);
	}

	void SetActive(bool active) {
		isActive = active;
	}

	void SetCurrentOperation(unsigned int co, bool update = true) {
		currentOperation = co;
		if (update) UpdateText();
	}

	void SetCurrentCommand(int cc, bool update = true) {
		currentCommand = cc;
		if (update) UpdateText();
	}

	void SetCurrentValues(unsigned int co, int cc) {
		SetCurrentCommand(cc, false);
		SetCurrentOperation(co, false);
		UpdateText();
	}
protected:
	// Отрисовка контейнера и его содержимого
	void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
		// Применяем трансформации и шейдеры текущего контейнера
		states = prepareStates(states);

		// Рисуем дочерние текстовые поля в зависимости от активности контейнера
		// Так как states уже содержит getTransform() этого контейнера, 
		// позиции дочерних элементов будут рассчитываться локально относительно него.
		if (!isActive) {
			target.draw(*notActiveText, states);
		}
		else {
			target.draw(*machineCodeKey, states);
			target.draw(*assemblerCommandKey, states);
			target.draw(*assemblerDecodedHeader, states);
			target.draw(*assemblerOperationKey, states);
			target.draw(*destinationKey, states);
			target.draw(*opAKey, states);
			target.draw(*opBKey, states);
			target.draw(*opAddrKey, states);
			target.draw(*descriptionKey, states);

			target.draw(*machineCodeValue, states);
			target.draw(*assemblerCommandValue, states);
			target.draw(*assemblerOperationValue, states);
			target.draw(*destinationValue, states);
			target.draw(*opAValue, states);
			target.draw(*opBValue, states);
			target.draw(*opAddrValue, states);
			target.draw(*descriptionValue, states);
		}
	}
private:
	bool isActive = false;
	int currentCommand = 0;
	unsigned int currentOperation = 0;
	
	// неизменяемый текст активного состояния
	std::unique_ptr<Text> machineCodeKey, assemblerCommandKey, assemblerDecodedHeader,
		assemblerOperationKey, destinationKey, opAKey, opBKey,
		opAddrKey, descriptionKey;

	// изменяемый текст активного состояния
	std::unique_ptr<Text> machineCodeValue, assemblerCommandValue, assemblerOperationValue, destinationValue,
		opAValue, opBValue, opAddrValue, descriptionValue;

	// заглушка неактивного состояния
	std::unique_ptr<Text> notActiveText;

	void UpdateText() {
		// command
		int commandIndex = currentCommand;
		assemblerCommandValue->setString(assemblerCommands[commandIndex]);
		descriptionValue->setString(assemblerCommandDescriptions[commandIndex]);

		// operation
		int instruction = currentOperation >> 16;
		int mar = currentOperation & 0xffff;

		int operationIndex = instruction >> 12;
		int c = (instruction >> 8) & 0xf;
		int a = (instruction >> 4) & 0xf;
		int b = instruction & 0xf;

		machineCodeValue->setString(DecToHexGrouped(currentOperation, 8));
		assemblerOperationValue->setString(assemblerOperations[operationIndex]);
		destinationValue->setString(sf::String(std::to_string(c)));
		opAValue->setString(sf::String(std::to_string(a)));
		opBValue->setString(sf::String(std::to_string(b)));
		opAddrValue->setString(DecToHexGrouped(mar, 4));
	}

	sf::String DecToHexGrouped(int dec, size_t autoComplete = 0) {
		std::string rawHex = "";

		// Работаем с беззнаковым типом во избежание проблем с отрицательными числами
		unsigned int i = static_cast<unsigned int>(dec);

		// 1. Переводим число в HEX-строку (в обратном порядке)
		do {
			rawHex += hexDigits[i & 0xf];
			i >>= 4;
		} while (i > 0);

		// 2. Дополняем нулями слева до нужной длины (так как строка перевернута, добавляем в конец)
		while (rawHex.length() < autoComplete) {
			rawHex += '0';
		}

		// Разворачиваем строку в правильный порядок
		std::reverse(rawHex.begin(), rawHex.end());

		// 3. Формируем финальную строку с разделением по 4 разряда
		std::string groupedHex = "";
		size_t len = rawHex.length();

		for (size_t idx = 0; idx < len; ++idx) {
			// Добавляем пробел перед каждым 4-м символом, если мы идем СПРАВА налево.
			// Магическая формула (len - idx) % 4 == 0 проверяет, осталось ли до конца кратное 4 число символов.
			if (idx > 0 && (len - idx) % 4 == 0) {
				groupedHex += ' ';
			}
			groupedHex += rawHex[idx];
		}

		return sf::String(groupedHex);
	}
};