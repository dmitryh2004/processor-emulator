#pragma once
#include "BaseObject.h"
#include <iostream>
#include <map>
#include <vector>
#include <memory>
#include <SFML/OpenGL.hpp> // Необходим для GL_SCISSOR_TEST и glScissor

class Panel : public BaseObject {
public:
    Panel(std::string name,
        sf::Vector2f size,
        sf::Vector2f parentSize,
        sf::Vector2f offset = sf::Vector2f(0.f, 0.f),
        Anchor parentAnchor = Anchor::TopLeft,
        Anchor localAnchor = Anchor::TopLeft,
        float rotation = 0.f,
        sf::Vector2f scale = sf::Vector2f(1.f, 1.f))
        : BaseObject(name, size, parentSize, offset, parentAnchor, localAnchor, rotation, scale),
        m_isClippingEnabled(true)
    {
    }

    void addObject(std::shared_ptr<BaseObject> object, int zIndex = 0) {
        if (object) {
            m_layers[zIndex].push_back(object);
        }
    }

    void checkForEvents(const sf::Event& event, const sf::RenderWindow& window, sf::Vector2f parentMousePos) override {
        // 1. Переводим координаты мыши из пространства родителя в ЛОКАЛЬНОЕ пространство этой панели
        // Для самой верхней панели parentMousePos — это просто window.mapPixelToCoords(sf::Mouse::getPosition(window))
        sf::Vector2f localMousePos = getTransform().getInverse().transformPoint(parentMousePos);

        // Флаг для предотвращения сквозного клика (чтобы кнопка на слое 1 перехватила клик и он не ушел на слой 0)
        bool eventHandled = false;

        // 2. Идем по слоям в ОБРАТНОМ порядке (от большего z-index к меньшему), 
        // потому что пользователь видит и кликает сначала по верхним элементам!
        for (auto it = m_layers.rbegin(); it != m_layers.rend(); ++it) {
            auto& layer = it->second;

            // Внутри одного слоя идем с конца в начало (последний добавленный — самый верхний)
            for (auto objIt = layer.rbegin(); objIt != layer.rend(); ++objIt) {
                auto& object = *objIt;

                if (!object) continue;

                // Если клик уже был обработан элементом выше, мы можем либо пропустить событие мыши, 
                // либо передать его, но сбросив факт клика.
                // Но мы передаем координаты дальше:
                object->checkForEvents(event, window, localMousePos);
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
        // 1. Сохраняем исходные состояния
        sf::RenderStates originalStates = states;
        states = prepareStates(states);

        bool scissorApplied = false;

        if (m_isClippingEnabled) {
            // Вычисляем глобальные экранные координаты панели
            sf::Transform finalTransform = originalStates.transform * getTransform();
            sf::Vector2f size = getSize();
            sf::Vector2f topLeft = finalTransform.transformPoint({ 0.f, 0.f });
            sf::Vector2f topRight = finalTransform.transformPoint({ size.x, 0.f });
            sf::Vector2f bottomLeft = finalTransform.transformPoint({ 0.f, size.y });
            sf::Vector2f bottomRight = finalTransform.transformPoint({ size.x, size.y });

            float minX = std::min({ topLeft.x, topRight.x, bottomLeft.x, bottomRight.x });
            float maxX = std::max({ topLeft.x, topRight.x, bottomLeft.x, bottomRight.x });
            float minY = std::min({ topLeft.y, topRight.y, bottomLeft.y, bottomRight.y });
            float maxY = std::max({ topLeft.y, topRight.y, bottomLeft.y, bottomRight.y });

            sf::Vector2i targetTopLeft = target.mapCoordsToPixel({ minX, minY });
            sf::Vector2i targetBottomRight = target.mapCoordsToPixel({ maxX, maxY });

            int scissorX = targetTopLeft.x;
            int scissorWidth = targetBottomRight.x - targetTopLeft.x;
            int scissorHeight = targetBottomRight.y - targetTopLeft.y;
            int scissorY = static_cast<int>(target.getSize().y) - targetBottomRight.y;

            if (scissorWidth > 0 && scissorHeight > 0) {
                // Включаем сциссор-тест напрямую через OpenGL.
                // В SFML 3 это безопасно делать без pushGLStates.
                glEnable(GL_SCISSOR_TEST);
                glScissor(scissorX, scissorY, scissorWidth, scissorHeight);
                scissorApplied = true;
            }
            else {
                return; // Если панель за пределами экрана или сжата в 0
            }
        }

        // 2. Отрисовываем дочерние объекты в строгом порядке слоев.
        // Теперь SFML отрисует их ровно в том порядке, в котором мы вызываем draw!
        for (const auto& [zIndex, layer] : m_layers) {
            for (const auto& object : layer) {
                target.resetGLStates();
                target.draw(*object, states);
            }
        }

        // 3. Наводим за собой порядок
        if (scissorApplied) {
            glDisable(GL_SCISSOR_TEST);

            // Сбрасываем внутренние состояния SFML. Этот метод гарантирует, 
            // что SFML корректно восстановит свои текстурные юниты и матрицы 
            // для следующих объектов вне этой панели, предотвращая баги отрисовки.
            target.resetGLStates();
        }
    }


private:
    std::map<int, std::vector<std::shared_ptr<BaseObject>>> m_layers;
    bool m_isClippingEnabled;
};
