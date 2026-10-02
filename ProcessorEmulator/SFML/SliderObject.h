#pragma once
#include "BaseObject.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include <limits>

class SliderObject : public BaseObject {
public:
    struct AnchorPoint {
        float x;
        float value;
    };

    SliderObject(std::string name,
        sf::Vector2f size,
        sf::Vector2f parentSize,
        const std::vector<AnchorPoint>& points,
        float initialValue,
        sf::Vector2f offset = sf::Vector2f(0.f, 0.f),
        Anchor parentAnchor = Anchor::TopLeft,
        Anchor localAnchor = Anchor::TopLeft,
        float rotation = 0.f,
        sf::Vector2f scale = sf::Vector2f(1.f, 1.f),
        const sf::Texture* bgTexture = nullptr,     // Опциональная текстура фона
        const sf::Texture* handleTexture = nullptr) // Опциональная текстура рычажка
        : BaseObject(name, size, parentSize, offset, parentAnchor, localAnchor, rotation, scale),
        m_points(points), m_isDragging(false), m_currentValue(initialValue)
    {
        // 1. Настройка фона
        m_background.setSize(getSize());
        if (bgTexture) {
            m_background.setTexture(bgTexture);
            m_background.setFillColor(sf::Color::White); // Сбрасываем цвет в белый для корректного отображения текстуры
        }
        else {
            m_background.setFillColor(sf::Color(100, 100, 100)); // Цвет по умолчанию
            m_background.setOutlineThickness(1.f);
            m_background.setOutlineColor(sf::Color::White);
        }

        // 2. Настройка рычажка
        // Если передана текстура рычажка, можем автоматически взять её размеры или задать дефолтные
        float handleWidth = bgTexture ? size.y : 15.f;
        sf::Vector2f handleSize = handleTexture ? sf::Vector2f(handleTexture->getSize()) : sf::Vector2f(handleWidth, size.y + 4.f);

        m_handle.setSize(handleSize);
        m_handle.setOrigin({ m_handle.getSize().x / 2.f, m_handle.getSize().y / 2.f });

        if (&handleTexture) {
            m_handle.setTexture(handleTexture);
            m_handle.setFillColor(sf::Color::White); // Сбрасываем цвет для текстуры
        }
        else {
            m_handle.setFillColor(sf::Color::Red); // Цвет по умолчанию
        }

        // 3. Устанавливаем рычажок в начальное положение
        snapToValue(initialValue);
    }

    // Методы для динамической смены текстур во время работы программы
    void setBackgroundTexture(const sf::Texture* texture, bool resetColor = true) {
        m_background.setTexture(texture);
        if (texture && resetColor) {
            m_background.setFillColor(sf::Color::White);
            m_background.setOutlineThickness(0.f); // Убираем рамку по умолчанию, если есть текстура
        }
    }

    void setHandleTexture(const sf::Texture* texture, bool resizeToTexture = true, bool resetColor = true) {
        m_handle.setTexture(texture);
        if (texture) {
            if (resizeToTexture) {
                m_handle.setSize(sf::Vector2f(texture->getSize()));
                m_handle.setOrigin({ m_handle.getSize().x / 2.f, m_handle.getSize().y / 2.f });
            }
            if (resetColor) {
                m_handle.setFillColor(sf::Color::White);
            }
        }
        // Пересчитываем позицию рычажка, так как его размеры/origin могли измениться
        snapToValue(m_currentValue);
    }

    void checkForEvents(const sf::Event& event, const sf::RenderWindow& window, sf::Vector2f localMousePos) override {
        if (const auto* mouseButtonEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseButtonEvent->button == sf::Mouse::Button::Left) {
                sf::Vector2f objMousePos = getTransform().getInverse().transformPoint(localMousePos);

                if (m_background.getGlobalBounds().contains(objMousePos) ||
                    m_handle.getGlobalBounds().contains(objMousePos)) {
                    m_isDragging = true;
                    updateHandlePosition(objMousePos.x);
                }
            }
        }

        if (const auto* mouseButtonEvent = event.getIf<sf::Event::MouseButtonReleased>()) {
            if (mouseButtonEvent->button == sf::Mouse::Button::Left) {
                m_isDragging = false;
            }
        }
    }

    void update(sf::Time deltaTime, const sf::RenderWindow& window, sf::Vector2f localMousePos) override {
        if (m_isDragging) {
            sf::Vector2f objMousePos = getTransform().getInverse().transformPoint(localMousePos);
            updateHandlePosition(objMousePos.x);
        }
    }

    float getValue() const { return m_currentValue; }
    void setValue(float value) { snapToValue(value); }

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        states = prepareStates(states);
        target.draw(m_background, states);
        target.draw(m_handle, states);
    }

private:
    std::vector<AnchorPoint> m_points;
    sf::RectangleShape m_background;
    sf::RectangleShape m_handle;

    bool m_isDragging;
    float m_currentValue;

    void updateHandlePosition(float mouseLocalX) {
        if (m_points.empty()) return;

        float minDistance = std::numeric_limits<float>::max();
        const AnchorPoint* closestPoint = &m_points[0];

        for (const auto& point : m_points) {
            float distance = std::abs(point.x - mouseLocalX);
            if (distance < minDistance) {
                minDistance = distance;
                closestPoint = &point;
            }
        }

        m_handle.setPosition({ closestPoint->x, getSize().y / 2.f });
        m_currentValue = closestPoint->value;
    }

    void snapToValue(float value) {
        if (m_points.empty()) return;

        float minDelta = std::numeric_limits<float>::max();
        const AnchorPoint* closestPoint = &m_points[0];

        for (const auto& point : m_points) {
            float delta = std::abs(point.value - value);
            if (delta < minDelta) {
                minDelta = delta;
                closestPoint = &point;
            }
        }

        m_handle.setPosition({ closestPoint->x, getSize().y / 2.f });
        m_currentValue = closestPoint->value;
    }
};
