#pragma once
#include "BaseObject.h"
#include <iomanip>
#include <sstream>

// Предполагаем, что ваш класс Text выглядит примерно так и корректно наследует BaseObject.
// Для демонстрации используем гипотетический класс Text, обертывающий sf::Text.
#include "Text.h" 

class RegisterContainer : public BaseObject {
public:
    RegisterContainer(std::string name,
        std::string regName, // Имя регистра, например "EAX"
        const sf::Font& font,
        unsigned int characterSize,
        sf::Vector2f parentSize,
        sf::Color color = sf::Color::White,
        sf::Vector2f offset = { 0.f, 0.f },
        Anchor parentAnchor = Anchor::TopLeft,
        Anchor localAnchor = Anchor::TopLeft)
        : BaseObject(name, { 250.f, 40.f }, parentSize, offset, parentAnchor, localAnchor),
        m_value(0)
    {
        // Ограничиваем имя регистра до 3 символов
        if (regName.length() > 3) {
            regName = regName.substr(0, 3);
        }

        // Передаем размер текущего контейнера (getSize()) как parentSize для дочерних текстовых полей.
        // Координаты смещения (offset) задаются относительно выбранной точки parentAnchor.

        // 1. Имя регистра
        m_textName = std::make_unique<Text>(
            name + "_lbl", font, getSize(), regName, characterSize, color,
            sf::Vector2f(5.f, 0.f), Anchor::CenterLeft, Anchor::CenterLeft
        );

        // 2. HEX значение
        m_textHex = std::make_unique<Text>(
            name + "_hex", font, getSize(), "0x00000000", characterSize, color,
            sf::Vector2f(51.f, 0.f), Anchor::CenterLeft, Anchor::CenterLeft
        );

        // 3. DEC значение
        m_textDec = std::make_unique<Text>(
            name + "_dec", font, getSize(), "0", characterSize, color,
            sf::Vector2f(176.f, 0.f), Anchor::CenterLeft, Anchor::CenterLeft
        );

        // Синхронизируем строковые значения с m_value
        updateTextVisuals();
    }

    // Метод записи значения в регистр (модель изменилась -> обновляем визуал)
    void setValue(int value) {
        if (m_value != value) {
            m_value = value;
            updateTextVisuals();
        }
    }

    // Метод чтения значения из регистра
    int getValue() const {
        return m_value;
    }

    // Перегрузка обновления (если дочерним элементам нужно передавать deltaTime)
    void update(sf::Time deltaTime) override {
        m_textName->update(deltaTime);
        m_textHex->update(deltaTime);
        m_textDec->update(deltaTime);
    }

    // Обработка событий для дочерних элементов
    void checkForEvents(const sf::Event& event, const sf::RenderWindow& window, sf::Vector2f localMousePos) override {
        // Трансформируем мышь относительно этого контейнера, если дочерние элементы полагаются на нее
        m_textName->checkForEvents(event, window, localMousePos);
        m_textHex->checkForEvents(event, window, localMousePos);
        m_textDec->checkForEvents(event, window, localMousePos);
    }

protected:
    // Отрисовка контейнера и его содержимого
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        // Применяем трансформации и шейдеры текущего контейнера
        states = prepareStates(states);

        // Рисуем дочерние текстовые поля
        // Так как states уже содержит getTransform() этого контейнера, 
        // позиции дочерних элементов будут рассчитываться локально относительно него.
        target.draw(*m_textName, states);
        target.draw(*m_textHex, states);
        target.draw(*m_textDec, states);
    }

private:
    int m_value; // Хранит 4 байта данных

    std::unique_ptr<Text> m_textName;
    std::unique_ptr<Text> m_textHex;
    std::unique_ptr<Text> m_textDec;

    // Вспомогательный метод для форматирования строк
    void updateTextVisuals() {
        // Форматируем HEX (дополняем нулями до 8 символов для 32-битного int)
        std::stringstream hexStream;
        hexStream << "0x" << std::setw(8) << std::setfill('0') << std::uppercase << std::hex << static_cast<unsigned int>(m_value);
        m_textHex->setString(hexStream.str());

        // Форматируем DEC
        m_textDec->setString(std::to_string(m_value));
    }
};
