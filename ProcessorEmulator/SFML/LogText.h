#pragma once
#include "InputField.h"
#include <string>
#include <chrono>
#include <iomanip>
#include <sstream>

class LogText : public InputField {
public:
    LogText(std::string name,
        sf::Vector2f size,
        sf::Vector2f parentSize,
        const sf::Font& font,
        unsigned int characterSize = 24,
        sf::Vector2f offset = sf::Vector2f(0.f, 0.f),
        Anchor parentAnchor = Anchor::TopLeft,
        Anchor localAnchor = Anchor::TopLeft)
        : InputField(name, size, parentSize, font, characterSize, offset, parentAnchor, localAnchor)
    {
    }

    void checkForEvents(const sf::Event& event, const sf::RenderWindow& window, sf::Vector2f localMousePos) override {

        // 1. ПРИНУДИТЕЛЬНАЯ АКТИВАЦИЯ ПРИ КЛИКЕ (чтобы работал скролл и выделение)
        if (auto* mouseBtnEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseBtnEvent->button == sf::Mouse::Button::Left) {
                sf::FloatRect globalBounds({ 0.f, 0.f }, getSize());
                globalBounds = getTransform().transformRect(globalBounds);
                sf::Vector2f worldMousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

                if (globalBounds.contains(worldMousePos)) {
                    // Используем защищенный метод базового класса (он меняет рамку и ставит m_isActive = true)
                    // Так как в InputField метод setActive приватный, мы можем продублировать его логику 
                    // или просто позволить базовому методу отработать ниже. 
                    // Но для надежности мы гарантируем, что события мыши всегда идут дальше.
                }
            }
        }

        // 2. БЛОКИРОВКА ИЗМЕНЕНИЯ ТЕКСТА
        // Полностью игнорируем ввод символов с клавиатуры
        if (event.is<sf::Event::TextEntered>()) {
            return;
        }

        // Блокируем клавиши редактирования (Backspace, Delete, Enter, Ctrl+V, Ctrl+X)
        if (auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            bool ctrlPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl);

            if (keyEvent->code == sf::Keyboard::Key::Delete ||
                keyEvent->code == sf::Keyboard::Key::Backspace ||
                keyEvent->code == sf::Keyboard::Key::Enter ||
                (keyEvent->code == sf::Keyboard::Key::V && ctrlPressed) ||
                (keyEvent->code == sf::Keyboard::Key::X && ctrlPressed))
            {
                return; // Запрещаем базовому классу удалять или вставлять текст
            }
        }

        // 3. ПЕРЕДАЧА ОСТАЛЬНЫХ СОБЫТИЙ В БАЗОВЫЙ КЛАСС
        // Мышь, клики, выделение, скролл колесиком, Ctrl+C и навигация стрелочками обработаются штатно
        InputField::checkForEvents(event, window, localMousePos);
    }

    void appendLog(const std::string& logMessage) {
        sf::String currentText = getTextString();

        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm local_tm;

#if defined(_MSC_VER)
        localtime_s(&local_tm, &now_time);
#else
        localtime_r(&now_time, &local_tm);
#endif

        std::ostringstream timeStream;
        timeStream << "[" << std::put_time(&local_tm, "%H:%M:%S") << "] ";
        std::string timeStamp = timeStream.str();

        if (!currentText.isEmpty()) {
            currentText += "\n";
        }

        currentText += timeStamp + sf::String::fromUtf8(logMessage.begin(), logMessage.end());
        setTextString(currentText);
    }

    void clearLog() {
        setTextString("");
    }
};
