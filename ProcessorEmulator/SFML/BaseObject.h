#pragma once
#include <SFML/Graphics.hpp>

class BaseObject : public sf::Drawable, public sf::Transformable {
public:
    BaseObject(sf::Vector2f size,
        sf::Vector2f position = sf::Vector2f(0.f, 0.f),
        float rotation = 0.f,
        sf::Vector2f scale = sf::Vector2f(1.f, 1.f))
        : m_size(size), m_shader(nullptr)
    {
        setPosition(position);
        setRotation(sf::degrees(rotation));
        setScale(scale);
    }

    virtual ~BaseObject() = default;

    virtual void checkForEvents(const sf::Event& event, const sf::RenderWindow& window) {}
    virtual void update(sf::Time deltaTime) {}

    sf::Vector2f getSize() const { return m_size; }

    // Сеттер для установки шейдера
    void setShader(const sf::Shader* shader) {
        m_shader = shader;
    }

    // Геттер для получения текущего шейдера
    const sf::Shader* getShader() const {
        return m_shader;
    }

protected:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override = 0;
    // Вспомогательный метод для применения трансформаций и шейдера
    sf::RenderStates prepareStates(sf::RenderStates states) const {
        states.transform *= getTransform();
        if (m_shader) {
            states.shader = m_shader;
        }
        return states;
    }

private:
    sf::Vector2f m_size;
    const sf::Shader* m_shader;
};
