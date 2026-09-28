#pragma once
#include "BaseObject.h"
#include <iomanip>
#include <sstream>

#include "Text.h" 

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
	
	// неизменяемый текст активного состояния
	std::unique_ptr<Text> machineCodeKey, assemblerCommandKey, assemblerDecodedHeader,
		assemblerOperationKey, destinationKey, opAKey, opBKey,
		opAddrKey, descriptionKey;

	// изменяемый текст активного состояния
	std::unique_ptr<Text> machineCodeValue, assemblerCommandValue, assemblerOperationValue, destinationValue,
		opAValue, opBValue, opAddrValue, descriptionValue;

	// заглушка неактивного состояния
	std::unique_ptr<Text> notActiveText;
};