#pragma once
#include "BaseObject.h"
#include <iomanip>
#include <sstream>

#include "../Emulator/Processor.h"
#include "Text.h"
#include "Image.h"

class FlagsController : public BaseObject {
public:
    FlagsController(std::string name,
        const sf::Texture& flagActiveTexture,
        Processor& processor,
        sf::Vector2f parentSize,
        sf::Color color = sf::Color::White,
        sf::Vector2f offset = { 0.f, 0.f },
        Anchor parentAnchor = Anchor::TopLeft,
        Anchor localAnchor = Anchor::TopLeft)
        : BaseObject(name, { 36.f, 98.f }, parentSize, offset, parentAnchor, localAnchor),
        m_value(0), processor(processor) 
    {
        m_CFImage = std::make_unique<Image>(name + "_CF",
            sf::Vector2f(8.f, 8.f), 
            flagActiveTexture, 
            sf::Vector2f(36.f, 98.f),
            sf::Vector2f(-8.f, 5.f),
            BaseObject::Anchor::TopRight, 
            BaseObject::Anchor::TopRight);

        m_ZFImage = std::make_unique<Image>(name + "_ZF",
            sf::Vector2f(8.f, 8.f),
            flagActiveTexture,
            sf::Vector2f(36.f, 98.f),
            sf::Vector2f(-8.f, 25.f),
            BaseObject::Anchor::TopRight,
            BaseObject::Anchor::TopRight);

        m_SFImage = std::make_unique<Image>(name + "_SF",
            sf::Vector2f(8.f, 8.f),
            flagActiveTexture,
            sf::Vector2f(36.f, 98.f),
            sf::Vector2f(-8.f, 44.f),
            BaseObject::Anchor::TopRight,
            BaseObject::Anchor::TopRight);

        m_CFImage->getSprite().setColor(color);
        m_ZFImage->getSprite().setColor(color);
        m_SFImage->getSprite().setColor(color);
    }

    // Метод записи значения в регистр (модель изменилась -> обновляем визуал)
    void setValue(int value) {
        if (m_value != value) {
            m_value = value;
        }
    }

    // Метод чтения значения из регистра
    int getValue() const {
        return m_value;
    }

    void onProcessorStep() {
        int newValue = processor.GetRegisterValue(6);
        // std::cout << "[" << getName() << "] - onProcessorStep. Register #" << m_rn << ", value = " << newValue << std::endl;
        setValue(newValue);
    }

    // Перегрузка обновления (если дочерним элементам нужно передавать deltaTime)
    void update(sf::Time deltaTime, const sf::RenderWindow& window, sf::Vector2f localMousePos) override {
        m_CFImage->update(deltaTime, window, localMousePos - m_CFImage->getPosition());
        m_SFImage->update(deltaTime, window, localMousePos - m_SFImage->getPosition());
        m_ZFImage->update(deltaTime, window, localMousePos - m_ZFImage->getPosition());
    }

    // Обработка событий для дочерних элементов
    void checkForEvents(const sf::Event& event, const sf::RenderWindow& window, sf::Vector2f localMousePos) override {
        // Трансформируем мышь относительно этого контейнера, если дочерние элементы полагаются на нее
        m_CFImage->checkForEvents(event, window, localMousePos);
        m_SFImage->checkForEvents(event, window, localMousePos);
        m_ZFImage->checkForEvents(event, window, localMousePos);
    }

protected:
    // Отрисовка контейнера и его содержимого
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        // Применяем трансформации и шейдеры текущего контейнера
        states = prepareStates(states);

        if (m_value & 4) 
            target.draw(*m_CFImage, states); // CF - 100
        if (m_value & 2) 
            target.draw(*m_ZFImage, states); // ZF - 010
        if (m_value & 1) 
            target.draw(*m_SFImage, states); // SF - 001
    }

private:
    int m_value; // Хранит 4 байта данных
    Processor& processor;

    std::unique_ptr<Image> m_CFImage;
    std::unique_ptr<Image> m_ZFImage;
    std::unique_ptr<Image> m_SFImage;
};