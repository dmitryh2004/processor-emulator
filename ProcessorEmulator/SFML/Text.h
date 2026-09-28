#pragma once
#include "BaseObject.h"

class Text : public BaseObject {
public:
    Text(std::string name,
        const sf::Font& font,
        sf::Vector2f parentSize,
        const sf::String& string = "",
        unsigned int characterSize = 30,
        sf::Color color = sf::Color::White,
        bool isWrapped = true, // Новый параметр в конструкторе
        sf::Vector2f offset = sf::Vector2f(0.f, 0.f),
        Anchor parentAnchor = Anchor::TopLeft,
        Anchor localAnchor = Anchor::TopLeft,
        float rotation = 0.f,
        sf::Vector2f scale = sf::Vector2f(1.f, 1.f))
        : BaseObject(name, sf::Vector2f(0.f, static_cast<float>(characterSize)), parentSize, offset, parentAnchor, localAnchor, rotation, scale),
        m_text(font),
        m_originalString(string),
        m_isWrapped(isWrapped), // Инициализируем флаг
        m_parentSize(parentSize)
    {
        m_text.setCharacterSize(characterSize);
        m_text.setFillColor(color);
        updateWrappedText(); // Применяет логику с учетом флага m_isWrapped
    }

    void setFillColor(sf::Color color) {
        m_text.setFillColor(color);
    }

    sf::Color getFillColor() const {
        return m_text.getFillColor();
    }

    void setText(const sf::Text& text) {
        m_text = text;
        m_originalString = text.getString();
        updateWrappedText();
    }

    sf::Text& getText() {
        return m_text;
    }

    const sf::Text& getText() const {
        return m_text;
    }

    void setString(const sf::String& string) {
        m_originalString = string;
        updateWrappedText();
    }

    sf::String getString() const {
        return m_originalString;
    }

    // Динамическое управление переносом текста
    void setWrapped(bool wrapped) {
        if (m_isWrapped != wrapped) {
            m_isWrapped = wrapped;
            updateWrappedText(); // Пересчитываем текст при изменении режима
        }
    }

    bool isWrapped() const {
        return m_isWrapped;
    }

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        states = prepareStates(states);
        target.draw(m_text, states);
    }

private:
    sf::Text m_text;
    sf::String m_originalString;
    bool m_isWrapped; // Флаг: включен ли перенос текста
    sf::Vector2f m_parentSize;

    void updateWrappedText() {
        // Если текст пустой — очищаем и выходим
        if (m_originalString.isEmpty()) {
            m_text.setString("");
            return;
        }

        // Если перенос отключен или ширина родителя некорректна, 
        // просто выводим оригинальный текст как есть
        float maxWidth = m_parentSize.x;
        if (!m_isWrapped || maxWidth <= 0.f) {
            m_text.setString(m_originalString);
            return;
        }

        sf::String finalString = "";
        sf::String currentLine = "";
        sf::String currentWord = "";
        sf::Text testText = m_text;

        for (std::size_t i = 0; i < m_originalString.getSize(); ++i) {
            char32_t character = m_originalString[i];
            
            if (character == '\n') {
                currentLine += currentWord + "\n";
                finalString += currentLine;
                currentLine = "";
                currentWord = "";
                continue;
            }

            currentWord += character;

            if (character == ' ' || i == m_originalString.getSize() - 1) {
                testText.setString(currentLine + currentWord);

                if (testText.getLocalBounds().size.x > maxWidth) {
                    if (!currentLine.isEmpty()) {
                        finalString += currentLine + "\n";
                        currentLine = currentWord;
                    }
                    else {
                        finalString += currentWord + "\n";
                        currentLine = "";
                    }
                }
                else {
                    currentLine += currentWord;
                }
                currentWord = "";
            }
        }

        finalString += currentLine;
        m_text.setString(finalString);
    }
};
