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
        // 1. Сначала полностью блокируем системное событие ввода текста (символов)
        if (event.is<sf::Event::TextEntered>()) {
            return;
        }

        // 2. Блокируем конкретные клавиши редактирования до того, как их обработает InputField
        if (auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
            bool ctrlPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) ||
                sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl);

            // Список запрещенных клавиш модификации
            if (keyEvent->code == sf::Keyboard::Key::Delete ||
                keyEvent->code == sf::Keyboard::Key::Backspace ||
                keyEvent->code == sf::Keyboard::Key::Enter ||
                (keyEvent->code == sf::Keyboard::Key::V && ctrlPressed) || // Вставка Ctrl+V
                (keyEvent->code == sf::Keyboard::Key::X && ctrlPressed))   // Вырезание Ctrl+X
            {
                return; // Запрещаем базовому классу реагировать на эти клавиши
            }
        }

        // 3. ВСЕ остальные события (клики мыши, перемещения для выделения, 
        // прокрутка колесиком, Ctrl+A, Ctrl+C, стрелочки навигации) 
        // беспрепятственно отдаем в базовый класс.
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
