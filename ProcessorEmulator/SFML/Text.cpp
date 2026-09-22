#pragma once
#include "BaseObject.h"

class Text : public BaseObject {
public:
    Text(std::string name, const sf::Font& font,
        const sf::String& string = "",
        unsigned int characterSize = 30,
        sf::Vector2f position = sf::Vector2f(0.f, 0.f),
        float rotation = 0.f,
        sf::Vector2f scale = sf::Vector2f(1.f, 1.f))
        // Размер текста динамический, поэтому в конструктор базы передаем (0, 0)
        : BaseObject(name, sf::Vector2f(0.f, 0.f), position, rotation, scale),
        m_text(font)
    {
        m_text.setString(string);
        m_text.setCharacterSize(characterSize);
    }

    // Сеттер для объекта sf::Text целиком
    void setText(const sf::Text& text) {
        m_text = text;
    }

    // Геттер для sf::Text
    sf::Text& getText() {
        return m_text;
    }

    // Константный геттер для sf::Text
    const sf::Text& getText() const {
        return m_text;
    }

    // Быстрый сеттер для изменения строки текста
    void setString(const sf::String& string) {
        m_text.setString(string);
    }

    // Быстрый геттер строки текста
    sf::String getString() const {
        return m_text.getString();
    }

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        // Применяем трансформации и шейдер базового класса
        states = prepareStates(states);
        // Отрисовываем внутренний текст с новыми состояниями
        target.draw(m_text, states);
    }

private:
    sf::Text m_text;
};
