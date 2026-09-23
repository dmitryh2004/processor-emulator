#include <iostream>
#include <SFML/Audio.hpp>
#include "BaseObject.h"

class Button : public BaseObject {
public:
    Button(std::string name,
        sf::Vector2f size,
        sf::Vector2f parentSize,
        sf::Vector2f offset,
        const sf::Texture& backgroundTexture,
        const sf::Texture& foregroundTexture,
        Anchor parentAnchor = Anchor::TopLeft,
        Anchor localAnchor = Anchor::TopLeft)
        : BaseObject(name, size, parentSize, offset, parentAnchor, localAnchor), m_fgTexture(&foregroundTexture)
    {
        m_shape.setSize(getSize());
        m_shape.setTexture(&backgroundTexture); // Привязываем текстуру к форме
        onHoverSound = onClickSound = nullptr;
    }

    void SetOnHoverSound(sf::Sound* sound) {
        onHoverSound = sound;
    }
    void SetOnClickSound(sf::Sound* sound) {
        onClickSound = sound;
    }
    void setForegroundTexture(const sf::Texture& texture) {
        m_fgTexture = &texture;
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
        // 1. Применяем трансформации базы
        states = prepareStates(states);

        // 2. Получаем неконстантный указатель для изменения Uniform-переменных
        sf::Shader* shader = getShader();

        if (shader && m_fgTexture) {
            // Теперь компилятор пропустит вызовы, так как shader не константный
            shader->setUniform("fgTexture", *m_fgTexture);

            if (m_shape.getTexture()) {
                shader->setUniform("bgTexture", *m_shape.getTexture());
            }
        }

        // 3. Рисуем форму с подготовленными состояниями
        target.draw(m_shape, states);
    }

private:
    sf::RectangleShape m_shape;
    const sf::Texture* m_fgTexture;
    bool m_isHovered = false;
    sf::Sound* onHoverSound = nullptr;
    sf::Sound* onClickSound = nullptr;
};
