#include <iostream>
#include <functional> // <--- Добавили для std::function
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
        m_shape.setTexture(&backgroundTexture);
        onHoverSound = onClickSound = nullptr;
    }

    void SetOnHoverSound(sf::Sound* sound) { onHoverSound = sound; }
    void SetOnClickSound(sf::Sound* sound) { onClickSound = sound; }
    void setForegroundTexture(const sf::Texture& texture) { m_fgTexture = &texture; }

    // Метод для назначения действия кнопке. Принимает ЛЮБУЮ функцию без параметров.
    // Благодаря замыканиям лямбд, параметры «прячутся» внутри самой функции.
    void SetOnClickAction(std::function<void()> action) {
        m_clickAction = action;
    }

    // Тот самый метод onClick, теперь он вызывает сохраненное действие.
    // Мы делаем его виртуальным, чтобы при желании его ВСЕ ЕЩЕ можно было переопределить.
    virtual void onClick() {
        if (m_clickAction) {
            m_clickAction(); // Вызов пользовательской логики
        }
    }

    void checkForEvents(const sf::Event& event, const sf::RenderWindow& window, sf::Vector2f localMousePos) override {
        sf::FloatRect localBounds(getPosition(), getSize());

        if (localBounds.contains(localMousePos)) {
            if (!m_isHovered) {
                m_isHovered = true;
                if (onHoverSound != nullptr) { onHoverSound->play(); }
            }

            if (const auto* mouseButtonPressed = event.getIf<sf::Event::MouseButtonPressed>()) {
                if (mouseButtonPressed->button == sf::Mouse::Button::Left) {
                    std::cout << "[" << getName() << "] Элемент нажат на локальных координатах: " << localMousePos.x << ", " << localMousePos.y << std::endl;

                    if (onClickSound != nullptr) { onClickSound->play(); }

                    onClick();
                }
            }
        }
        else {
            if (m_isHovered) { m_isHovered = false; }
        }
    }

    void update(sf::Time deltaTime, const sf::RenderWindow& window, sf::Vector2f localMousePos) override {}

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        states = prepareStates(states);
        sf::Shader* shader = getShader();
        if (shader && m_fgTexture) {
            shader->setUniform("fgTexture", *m_fgTexture);
            if (m_shape.getTexture()) { shader->setUniform("bgTexture", *m_shape.getTexture()); }
        }
        target.draw(m_shape, states);
    }

private:
    sf::RectangleShape m_shape;
    const sf::Texture* m_fgTexture;
    bool m_isHovered = false;
    sf::Sound* onHoverSound = nullptr;
    sf::Sound* onClickSound = nullptr;

    std::function<void()> m_clickAction = nullptr; // Хранилище для действия
};
