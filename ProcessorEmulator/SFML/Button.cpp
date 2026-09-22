#include <iostream>
#include <SFML/Audio.hpp>
#include "BaseObject.h"

class Button : public BaseObject {
public:
    Button(std::string name, sf::Vector2f size, sf::Vector2f position, const sf::Texture& texture)
        : BaseObject(name, size, position)
    {
        m_shape.setSize(getSize());
        m_shape.setTexture(&texture); // Привязываем текстуру к форме
        onHoverSound = onClickSound = nullptr;
    }

    void SetOnHoverSound(sf::Sound* sound) {
        onHoverSound = sound;
    }
    void SetOnClickSound(sf::Sound* sound) {
        onClickSound = sound;
    }

    void checkForEvents(const sf::Event& event, const sf::RenderWindow& window, sf::Vector2f localMousePos) override {
        // Границы кнопки в её СОБСТВЕННЫХ локальных координатах
        sf::FloatRect localBounds(getPosition(), getSize());

        // Проверка наведения мыши по локальным координатам
        if (localBounds.contains(localMousePos)) {
            if (!m_isHovered) {
                m_isHovered = true;
                if (onHoverSound != nullptr) {
                    onHoverSound->play();
                }
            }

            // Проверяем нажатие в SFML 3.x
            if (const auto* mouseButtonPressed = event.getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
                    std::cout << "[" << getName() << "] Элемент нажат на локальных координатах: " << localMousePos.x << ", " << localMousePos.y << std::endl;

                    if (onClickSound != nullptr) {
                        onClickSound->play();
                    }
                }
            }
        }
        else {
            if (m_isHovered) {
                m_isHovered = false;
            }
        }
    }


    void update(sf::Time deltaTime) override {
        // Здесь может быть какая-то покадровая логика, например, легкое покачивание или пульсация
    }
protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        // Рисуем внутреннюю форму с учетом этих трансформаций
        states = prepareStates(states);
        target.draw(m_shape, states);
    }

private:
    sf::RectangleShape m_shape;
    bool m_isHovered = false;
    sf::Sound* onHoverSound = nullptr;
    sf::Sound* onClickSound = nullptr;
};
