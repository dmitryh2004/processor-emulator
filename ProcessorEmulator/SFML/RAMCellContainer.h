#pragma once
#include "BaseObject.h" // Путь к вашему базовому классу
#include "Text.h"       // Путь к вашему классу Text
#include <format>
#include <cstdint>

class RAMCellContainer : public BaseObject {
public:
    RAMCellContainer(std::string name,
        sf::Vector2f size,
        sf::Vector2f parentSize,
        const sf::Font& font,
        const sf::Font& modalWindowFont,
        uint32_t address = 0,
        uint32_t value = 0,
        unsigned int characterSize = 20,
        sf::Color textColor = sf::Color::White,
        sf::Vector2f offset = sf::Vector2f(0.f, 0.f),
        Anchor parentAnchor = Anchor::TopLeft,
        Anchor localAnchor = Anchor::TopLeft,
        float rotation = 0.f,
        sf::Vector2f scale = sf::Vector2f(1.f, 1.f))
        : BaseObject(name, size, parentSize, offset, parentAnchor, localAnchor, rotation, scale),
        m_modalWindowFont(modalWindowFont),
        m_address(address),
        m_value(value)
    {
        // Инициализируем текстовое поле адреса
        // Привязка: parentAnchor = TopRight, localAnchor = TopRight, смещение 10 пикселей внутрь (-10.f, 10.f)
        m_addressText = std::make_unique<Text>(
            name + "_address",
            font,
            size, // Родитель — сам контейнер
            "",   // Текст установим через метод обновления
            characterSize,
            textColor,
            false, // Без переноса строк
            sf::Vector2f(-10.f, 10.f),
            Anchor::TopRight,
            Anchor::TopRight
        );

        // Инициализируем текстовое поле значения
        // Привязка: parentAnchor = BottomLeft, localAnchor = BottomLeft, смещение 10 пикселей внутрь (10.f, -10.f)
        m_valueText = std::make_unique<Text>(
            name + "_value",
            font,
            size, // Родитель — сам контейнер
            "",   // Текст установим через метод обновления
            characterSize,
            textColor,
            false,
            sf::Vector2f(10.f, -10.f),
            Anchor::BottomLeft,
            Anchor::BottomLeft
        );

        // Обновляем строковые представления
        updateTextDisplays();
    }

    // Метод изменения адреса ячейки
    void setAddress(uint32_t address) {
        m_address = address;
        updateTextDisplays();
    }

    // Метод изменения значения ячейки
    void setValue(uint32_t value) {
        m_value = value;
        updateTextDisplays();
    }

    uint32_t getAddress() const { return m_address; }
    uint32_t getValue() const { return m_value; }

    // Логика обновления дочерних элементов (если требуется)
    void update(sf::Time deltaTime) override {
        m_addressText->update(deltaTime);
        m_valueText->update(deltaTime);
    }

    void checkForEvents(const sf::Event& event, const sf::RenderWindow& window, sf::Vector2f localMousePos) override {
        // Если адрес равен 0, значение "reserved" и его нельзя редактировать
        if (m_address == 0) return;

        // Проверяем нажатие левой кнопки мыши
        if (auto* mouseButtonEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseButtonEvent->button == sf::Mouse::Button::Left) {

                // Предполагаем, что у вашего класса Text есть метод для получения границ (sf::FloatRect).
                // Так как localMousePos передается в координатах этого контейнера, 
                // мы проверяем попадание мыши в локальные границы текста значения.
                // Примечание: если getGlobalBounds() вашего текста возвращает мировые координаты, 
                // нужно будет учесть трансформацию, но обычно для UI-компонентов проверяют локально.
                sf::FloatRect textBounds = m_valueText->getBounds();

                if (textBounds.contains(localMousePos)) {
                    // Метод клика по тексту значения — открываем модальное окно
                    openEditDialog(window.getSettings(), m_modalWindowFont, window); // Передаем шрифт
                }
            }
        }
    }
protected:
    // Отрисовка контейнера и вложенных текстов
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        // Применяем трансформацию и шейдер текущего контейнера к состояниям отрисовки
        sf::RenderStates localStates = prepareStates(states);

        // Отрисовываем дочерние текстовые поля с учетом накопленной трансформации
        target.draw(*m_addressText, localStates);
        target.draw(*m_valueText, localStates);
    }

private:
    const sf::Font& m_modalWindowFont;

    uint32_t m_address;
    uint32_t m_value;

    std::unique_ptr<Text> m_addressText;
    std::unique_ptr<Text> m_valueText;

    // Вспомогательный метод для обновления строк и обработки условия "reserved"
    void updateTextDisplays() {
        // Форматируем адрес в 0xHEX (например, 0x0000004A)
        std::string addressStr = std::format("0x{:08X}", m_address);
        m_addressText->setString(addressStr);

        // Проверяем условие зарезервированного адреса
        if (m_address == 0) {
            m_valueText->setString("reserved");
        }
        else {
            // Форматируем значение в 0xHEX (например, 0x000000FF)
            std::string valueStr = std::format("0x{:08X}", m_value);
            m_valueText->setString(valueStr);
        }
    }

    void openEditDialog(const sf::ContextSettings& settings, const sf::Font& font, const sf::RenderWindow& mainWindowRef) {
        sf::RenderWindow dialog(sf::VideoMode({ 400, 250 }), "Изменение значения ячейки ОЗУ", sf::State::Windowed, settings);
        dialog.setFramerateLimit(60);

        std::string decInput = std::to_string(m_value);

        // Настройка UI элементов (остается прежней)
        sf::Text titleText(font, std::format("Адрес ячейки: 0x{:08X}", m_address), 18);
        titleText.setPosition({ 20.f, 20.f });
        titleText.setFillColor(sf::Color::White);

        sf::Text inputLabel(font, "Десятичное значение:", 14);
        inputLabel.setPosition({ 20.f, 60.f });
        inputLabel.setFillColor(sf::Color::Cyan);

        sf::Text inputDisplay(font, decInput + "|", 16);
        inputDisplay.setPosition({ 20.f, 85.f });
        inputDisplay.setFillColor(sf::Color::White);

        sf::Text hexLabel(font, "Hex-значение:", 14);
        hexLabel.setPosition({ 20.f, 125.f });
        hexLabel.setFillColor(sf::Color::Cyan);

        sf::Text hexDisplay(font, std::format("0x{:08X}", m_value), 16);
        hexDisplay.setPosition({ 20.f, 150.f });
        hexDisplay.setFillColor(sf::Color::Yellow);

        sf::RectangleShape saveBtn({ 100.f, 35.f });
        saveBtn.setPosition({ 160.f, 200.f });
        saveBtn.setFillColor(sf::Color(0, 150, 0));

        sf::Text saveText(font, "Сохранить", 14);
        saveText.setPosition({ 190.f, 208.f });

        sf::RectangleShape cancelBtn({ 100.f, 35.f });
        cancelBtn.setPosition({ 280.f, 200.f });
        cancelBtn.setFillColor(sf::Color(150, 0, 0));

        sf::Text cancelText(font, "Отмена", 14);
        cancelText.setPosition({ 305.f, 208.f });

        // Получаем неконстантную ссылку на главное окно для очистки его очереди событий
        auto& mainWindow = const_cast<sf::RenderWindow&>(mainWindowRef);

        // Вложенный игровой цикл модального окна
        while (dialog.isOpen()) {

            // --- РЕШЕНИЕ ПРОБЛЕМЫ ЗАВИСАНИЯ ОС ---
            // Опрашиваем события главного окна. Игнорируем всё, кроме закрытия.
            while (const std::optional<sf::Event> mainEvent = mainWindow.pollEvent()) {
                if (mainEvent->is<sf::Event::Closed>()) {
                    mainWindow.close();
                    dialog.close();
                    return;
                }
            }

            // Опрашиваем события модального диалога
            while (const std::optional<sf::Event> optEvent = dialog.pollEvent()) {
                const sf::Event& event = *optEvent;

                if (event.is<sf::Event::Closed>()) {
                    dialog.close();
                }

                // Обработка ввода текста с автозаменой при переполнении
                if (auto* textEvent = event.getIf<sf::Event::TextEntered>()) {
                    uint32_t unicode = textEvent->unicode;

                    if (unicode >= '0' && unicode <= '9') {
                        // Позволяем ввести до 12 символов, чтобы триггерить проверку на переполнение
                        if (decInput.length() < 12) {
                            decInput += static_cast<char>(unicode);
                        }
                    }
                    else if (unicode == 8 || unicode == 127) { // Backspace
                        if (!decInput.empty()) {
                            decInput.pop_back();
                        }
                    }

                    // --- АВТОМАТИЧЕСКАЯ УСТАНОВКА МАКСИМУМА (CLAMPING) ---
                    if (!decInput.empty()) {
                        try {
                            unsigned long long val = std::stoull(decInput);

                            // Если введено число больше макс. значения uint32_t (4294967295)
                            if (val > 4294967295ULL) {
                                decInput = "4294967295";
                                val = 4294967295ULL;
                            }

                            uint32_t currentVal = static_cast<uint32_t>(val);
                            hexDisplay.setString(std::format("0x{:08X}", currentVal));
                        }
                        catch (...) {
                            // Обработка исключения на случай ввода экстремально длинного числа
                            decInput = "4294967295";
                            hexDisplay.setString("0xFFFFFFFF");
                        }
                    }
                    else {
                        hexDisplay.setString("0x00000000");
                    }

                    // Обновляем текст на экране с курсором
                    inputDisplay.setString(decInput + "|");
                }

                // Обработка кликов по кнопкам
                if (auto* mouseButtonEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
                    if (mouseButtonEvent->button == sf::Mouse::Button::Left) {
                        sf::Vector2f mousePos = dialog.mapPixelToCoords(mouseButtonEvent->position);

                        if (cancelBtn.getGlobalBounds().contains(mousePos)) {
                            dialog.close();
                        }
                        else if (saveBtn.getGlobalBounds().contains(mousePos)) {
                            if (!decInput.empty()) {
                                setValue(static_cast<uint32_t>(std::stoull(decInput)));
                            }
                            else {
                                setValue(0);
                            }
                            dialog.close();
                        }
                    }
                }
            }

            // Рендеринг модального окна
            dialog.clear(sf::Color(30, 30, 30));
            dialog.draw(titleText);
            dialog.draw(inputLabel);
            dialog.draw(inputDisplay);
            dialog.draw(hexLabel);
            dialog.draw(hexDisplay);
            dialog.draw(saveBtn);
            dialog.draw(saveText);
            dialog.draw(cancelBtn);
            dialog.draw(cancelText);
            dialog.display();
        }
    }
};