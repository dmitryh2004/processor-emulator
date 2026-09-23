#pragma once
#include <SFML/Graphics.hpp>
#include <string>

class BaseObject : public sf::Drawable, public sf::Transformable {
public:
    // Перечисление для сторон и углов привязки
    enum class Anchor {
        TopLeft, TopCenter, TopRight,
        CenterLeft, Center, CenterRight,
        BottomLeft, BottomCenter, BottomRight
    };

    BaseObject(std::string name,
        sf::Vector2f size,
        sf::Vector2f parentSize, // Размеры родителя (объекта или окна)
        sf::Vector2f offset = sf::Vector2f(0.f, 0.f), // Смещение относительно точки привязки
        Anchor parentAnchor = Anchor::TopLeft,        // Точка на родителе
        Anchor localAnchor = Anchor::TopLeft,         // Точка на самом объекте
        float rotation = 0.f,
        sf::Vector2f scale = sf::Vector2f(1.f, 1.f))
        : name(name), m_size(size), m_shader(nullptr)
    {
        // 1. Находим мировые координаты точки привязки на родителе
        sf::Vector2f parentAnchorPos = calculateAnchorPosition(parentAnchor, parentSize);

        // 2. Находим локальные координаты точки привязки внутри самого объекта
        sf::Vector2f localAnchorPos = calculateAnchorPosition(localAnchor, m_size);

        // 3. Вычисляем итоговую позицию левого верхнего угла (глобальный Origin) объекта
        // Позиция = Точка_Родителя + Смещение - Точка_Объекта
        sf::Vector2f finalPosition = parentAnchorPos + offset - localAnchorPos;

        setPosition(finalPosition);
        setRotation(sf::degrees(rotation)); // В SFML 3.x используется sf::Angle
        setScale(scale);
    }

    virtual ~BaseObject() = default;

    virtual void checkForEvents(const sf::Event& event, const sf::RenderWindow& window, sf::Vector2f localMousePos) {}
    virtual void update(sf::Time deltaTime) {}

    sf::Vector2f getSize() const { return m_size; }
    std::string getName() const { return name; }

    void setShader(sf::Shader* shader) {
        m_shader = shader;
    }

    // Возвращаем неконстантный указатель
    sf::Shader* getShader() const {
        return m_shader;
    }

protected:
    virtual void draw(sf::RenderTarget& target, sf::RenderStates states) const override = 0;

    sf::RenderStates prepareStates(sf::RenderStates states) const {
        states.transform *= getTransform();
        states.shader = m_shader; // sf::RenderStates::shader в SFML принимает const sf::Shader*, так что это сработает
        return states;
    }

private:
    std::string name;
    sf::Vector2f m_size;
    sf::Shader* m_shader;

    // Вспомогательный метод для расчета координат точки привязки относительно прямоугольника (размера)
    sf::Vector2f calculateAnchorPosition(Anchor anchor, sf::Vector2f size) const {
        switch (anchor) {
        case Anchor::TopLeft:      return { 0.f, 0.f };
        case Anchor::TopCenter:    return { size.x / 2.f, 0.f };
        case Anchor::TopRight:     return { size.x, 0.f };

        case Anchor::CenterLeft:   return { 0.f, size.y / 2.f };
        case Anchor::Center:       return { size.x / 2.f, size.y / 2.f };
        case Anchor::CenterRight:  return { size.x, size.y / 2.f };

        case Anchor::BottomLeft:   return { 0.f, size.y };
        case Anchor::BottomCenter: return { size.x / 2.f, size.y };
        case Anchor::BottomRight:  return { size.x, size.y };
        default:                   return { 0.f, 0.f };
        }
    }
};
