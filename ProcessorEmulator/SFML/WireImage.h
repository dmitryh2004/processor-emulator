#pragma once
#include "Image.h"

class WireImage : public Image {
public:
    WireImage(std::string name,
        sf::Vector2f size,
        const sf::Texture& texture,
        float visibilityTimer,
        sf::Vector2f parentSize,
        sf::Color wireColor = sf::Color::White,
        sf::Vector2f offset = sf::Vector2f(0.f, 0.f),
        Anchor parentAnchor = Anchor::TopLeft,
        Anchor localAnchor = Anchor::TopLeft,
        float rotation = 0.f,
        sf::Vector2f scale = sf::Vector2f(1.f, 1.f))
        : Image(name, size, texture, parentSize, offset, parentAnchor, localAnchor, rotation, scale), 
        m_vt(visibilityTimer)
    {
        clock = sf::Clock();
        getSprite().setColor(wireColor);
    }

    bool isVisible() {
        return visible;
    }

    void setVisible(bool visible) {
        this->visible = visible;
        if (this->visible) {
            clock.restart();
        }
        else {
            clock.stop();
        }
    }

    void update(sf::Time deltaTime, const sf::RenderWindow& window, sf::Vector2f localMousePos) override {
        if (visible && clock.getElapsedTime().asSeconds() > m_vt) {
            setVisible(false);
        }
    }

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        if (visible) 
            Image::draw(target, states);
    }

private:
    float m_vt;
    bool visible = false;
    sf::Clock clock;
};