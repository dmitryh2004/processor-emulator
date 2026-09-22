#include <iostream>
#include <SFML/Audio.hpp>
#include "BaseObject.h"

class Button : public BaseObject {
public:
    Button(sf::Vector2f size, sf::Vector2f position, const sf::Texture& texture)
        : BaseObject(size, position)
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

    void checkForEvents(const sf::Event& event, const sf::RenderWindow& window) override {
        // Получаем глобальные координаты мыши в игровом мире
        sf::Vector2f mousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        // В SFML 3.x для получения трансформированных границ объекта 
        // мы комбинируем его локальные границы и матрицу трансформации getTransform()
        sf::FloatRect bounds = getTransform().transformRect(sf::FloatRect({ 0.f, 0.f }, getSize()));

        // Проверка наведения мыши (вход / выход)
        if (bounds.contains(mousePos)) {
            if (!m_isHovered) {
                m_isHovered = true;
                if (onHoverSound != NULL) {
                    onHoverSound->play();
                }
            }

            // Проверяем нажатие (в SFML 3.0+ события мыши проверяются через event.getIf)
            if (const auto* mouseButtonPressed = event.getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
                    std::cout << "Элемент нажат!" << std::endl;
                    
                    if (onClickSound != NULL) {
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
        target.draw(m_shape, prepareStates(states));
    }

private:
    sf::RectangleShape m_shape;
    bool m_isHovered = false;
    sf::Sound* onHoverSound = nullptr;
    sf::Sound* onClickSound = nullptr;
};
