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
        sf::Vector2f offset = sf::Vector2f(0.f, 0.f),
        Anchor parentAnchor = Anchor::TopLeft,
        Anchor localAnchor = Anchor::TopLeft,
        float rotation = 0.f,
        sf::Vector2f scale = sf::Vector2f(1.f, 1.f))
        : BaseObject(name, sf::Vector2f(0.f, static_cast<float>(characterSize)), parentSize, offset, parentAnchor, localAnchor, rotation, scale),
        m_text(font)
    {
        m_text.setString(string);
        m_text.setCharacterSize(characterSize);
        m_text.setFillColor(color);
    }

    void setFillColor(sf::Color color) {
        m_text.setFillColor(color);
    }

    sf::Color getFillColor() const {
        return m_text.getFillColor();
    }

    void setText(const sf::Text& text) {
        m_text = text;
    }

    sf::Text& getText() {
        return m_text;
    }

    const sf::Text& getText() const {
        return m_text;
    }

    void setString(const sf::String& string) {
        m_text.setString(string);
    }

    sf::String getString() const {
        return m_text.getString();
    }

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        states = prepareStates(states);
        target.draw(m_text, states);
    }

private:
    sf::Text m_text;
};
