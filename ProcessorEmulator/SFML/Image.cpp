#pragma once
#include "BaseObject.h"

class Image : public BaseObject {
public:
    Image(std::string name, sf::Vector2f size,
        const sf::Texture& texture,
        sf::Vector2f position = sf::Vector2f(0.f, 0.f),
        float rotation = 0.f,
        sf::Vector2f scale = sf::Vector2f(1.f, 1.f))
        : BaseObject(name, size, position, rotation, scale),
        m_sprite(texture)
    {
        updateSpriteScale(); // Подгоняем спрайт под размер m_size
    }

    // Сеттер для полной замены спрайта
    void setSprite(const sf::Sprite& sprite) {
        m_sprite = sprite;
        updateSpriteScale();
    }

    // Удобный сеттер текстуры напрямую в спрайт
    void setTexture(const sf::Texture& texture, bool resetRect = false) {
        m_sprite.setTexture(texture, resetRect);
        updateSpriteScale();
    }

    // Геттер для спрайта
    sf::Sprite& getSprite() { return m_sprite; }
    const sf::Sprite& getSprite() const { return m_sprite; }

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        // Применяем позицию, поворот, масштаб и шейдер из BaseObject
        states = prepareStates(states);
        // Отрисовываем внутренний отмасштабированный спрайт
        target.draw(m_sprite, states);
    }

private:
    // Вспомогательный метод для подгонки текстуры под размер m_size
    void updateSpriteScale() {
        sf::Vector2f targetSize = getSize();

        // Получаем локальные границы спрайта (учитывают textureRect, если он задан)
        sf::FloatRect bounds = m_sprite.getLocalBounds();

        if (bounds.size.x > 0.f && bounds.size.y > 0.f) {
            float scaleX = targetSize.x / bounds.size.x;
            float scaleY = targetSize.y / bounds.size.y;
            m_sprite.setScale({ scaleX, scaleY });
        }
    }

    sf::Sprite m_sprite;
};
