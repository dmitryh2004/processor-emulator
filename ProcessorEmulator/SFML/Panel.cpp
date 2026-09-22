#pragma once
#include "BaseObject.h"
#include <map>
#include <vector>
#include <memory>
#include <SFML/OpenGL.hpp> // Необходим для GL_SCISSOR_TEST и glScissor

class Panel : public BaseObject {
public:
    Panel(sf::Vector2f size,
        sf::Vector2f position = sf::Vector2f(0.f, 0.f),
        float rotation = 0.f,
        sf::Vector2f scale = sf::Vector2f(1.f, 1.f))
        : BaseObject(size, position, rotation, scale), m_isClippingEnabled(true)
    {
    }

    void addObject(std::shared_ptr<BaseObject> object, int zIndex = 0) {
        if (object) {
            m_layers[zIndex].push_back(object);
        }
    }

    void checkForEvents(const sf::Event& event, const sf::RenderWindow& window) override {
        for (auto& [zIndex, layer] : m_layers) {
            for (auto& object : layer) {
                object->checkForEvents(event, window);
            }
        }
    }

    void update(sf::Time deltaTime) override {
        for (auto& [zIndex, layer] : m_layers) {
            for (auto& object : layer) {
                object->update(deltaTime);
            }
        }
    }

    void clear() {
        m_layers.clear();
    }

    // Включение / выключение отсечения
    void setClippingEnabled(bool enabled) { m_isClippingEnabled = enabled; }
    bool isClippingEnabled() const { return m_isClippingEnabled; }

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        // Сохраняем текущую трансформацию панели
        sf::RenderStates originalStates = states;
        states = prepareStates(states);

        bool scissorApplied = false;

        // Применяем клиппинг, если он включен
        if (m_isClippingEnabled) {
            // Считаем полную матрицу трансформации панели относительно целевого окна
            sf::Transform finalTransform = originalStates.transform * getTransform();

            // Переводим четыре угла локального m_size панели в экранные координаты
            sf::Vector2f size = getSize();
            sf::Vector2f topLeft = finalTransform.transformPoint({ 0.f, 0.f });
            sf::Vector2f topRight = finalTransform.transformPoint({ size.x, 0.f });
            sf::Vector2f bottomLeft = finalTransform.transformPoint({ 0.f, size.y });
            sf::Vector2f bottomRight = finalTransform.transformPoint({ size.x, size.y });

            // Находим минимальные и максимальные экранные координаты (на случай, если панель повернута)
            float minX = std::min({ topLeft.x, topRight.x, bottomLeft.x, bottomRight.x });
            float maxX = std::max({ topLeft.x, topRight.x, bottomLeft.x, bottomRight.x });
            float minY = std::min({ topLeft.y, topRight.y, bottomLeft.y, bottomRight.y });
            float maxY = std::max({ topLeft.y, topRight.y, bottomLeft.y, bottomRight.y });

            // Переводим координаты в пиксели внутри RenderTarget
            sf::Vector2i targetTopLeft = target.mapCoordsToPixel({ minX, minY });
            sf::Vector2i targetBottomRight = target.mapCoordsToPixel({ maxX, maxY });

            int scissorX = targetTopLeft.x;
            int scissorWidth = targetBottomRight.x - targetTopLeft.x;
            int scissorHeight = targetBottomRight.y - targetTopLeft.y;

            // В OpenGL (0,0) — это левый НИЖНИЙ угол, пересчитываем координату Y [1, 2]
            int scissorY = static_cast<int>(target.getSize().y) - targetBottomRight.y;

            if (scissorWidth > 0 && scissorHeight > 0) {
                // Перед вызовами OpenGL принудительно сбрасываем буфер SFML на видеокарту
                target.pushGLStates();

                // Включаем scissor test и задаем область отсечения [1]
                glEnable(GL_SCISSOR_TEST);
                glScissor(scissorX, scissorY, scissorWidth, scissorHeight);

                target.popGLStates();
                scissorApplied = true;
            }
            else {
                // Если ширина или высота нулевые/отрицательные, ничего не рисуем
                return;
            }
        }

        // Рисуем дочерние объекты
        for (const auto& [zIndex, layer] : m_layers) {
            for (const auto& object : layer) {
                target.draw(*object, states);
            }
        }

        // Выключаем отсечение, чтобы оно не влияло на другие объекты вне панели
        if (scissorApplied) {
            target.pushGLStates();
            glDisable(GL_SCISSOR_TEST);
            target.popGLStates();
        }
    }

private:
    std::map<int, std::vector<std::shared_ptr<BaseObject>>> m_layers;
    bool m_isClippingEnabled;
};
