#pragma once
#include "BaseObject.h"
#include <vector>
#include <cmath>

class InputField : public BaseObject {
public:
    InputField(std::string name,
        sf::Vector2f size,
        sf::Vector2f parentSize,
        const sf::Font& font,
        unsigned int characterSize = 24,
        sf::Vector2f offset = sf::Vector2f(0.f, 0.f),
        Anchor parentAnchor = Anchor::TopLeft,
        Anchor localAnchor = Anchor::TopLeft)
        : BaseObject(name, size, parentSize, offset, parentAnchor, localAnchor),
        m_font(&font), m_characterSize(characterSize), m_isActive(false), m_cursorIndex(0)
    {
        // Настройка фона
        m_background.setSize(getSize());
        m_background.setFillColor(sf::Color(30, 30, 30));
        m_background.setOutlineColor(sf::Color(100, 100, 100));
        m_background.setOutlineThickness(1.f);

        // Настройка геометрии букв
        m_vertices.setPrimitiveType(sf::PrimitiveType::Triangles);

        // Настройка курсора
        m_cursor.setSize({ 2.f, static_cast<float>(m_characterSize) });
        m_cursor.setFillColor(sf::Color::White);

        // Настройка выделения
        m_selectionVertices.setPrimitiveType(sf::PrimitiveType::Triangles);

        rebuildVertices();
    }

    std::string getTextString() const {
        return m_string.toAnsiString();
    }

    void setTextString(const std::string& text) {
        m_string = text;
        m_cursorIndex = m_string.getSize();
        rebuildVertices();
    }

    void checkForEvents(const sf::Event& event, const sf::RenderWindow& window, sf::Vector2f localMousePos) override {
        sf::FloatRect globalBounds({ 0.f, 0.f }, getSize());
        globalBounds = getTransform().transformRect(globalBounds);

        sf::Vector2f worldMousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));
        sf::Vector2f localMouse = getTransform().getInverse().transformPoint(worldMousePos);

        // 1. Обработка мыши (Клик, Отпускание, Выделение)
        if (auto* mouseBtnEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (globalBounds.contains(worldMousePos)) {
                if (mouseBtnEvent->button == sf::Mouse::Button::Left) {
                    setActive(true);
                    m_isSelectingWithMouse = true;

                    m_cursorIndex = findClosestCharIndex(localMouse);
                    m_selectionStart = m_cursorIndex;
                    m_selectionEnd = m_cursorIndex;
                    rebuildVertices();
                }
            }
            else {
                if (mouseBtnEvent->button == sf::Mouse::Button::Left || mouseBtnEvent->button == sf::Mouse::Button::Right) {
                    setActive(false);
                }
            }
        }

        if (auto* mouseReleaseEvent = event.getIf<sf::Event::MouseButtonReleased>()) {
            if (mouseReleaseEvent->button == sf::Mouse::Button::Left) {
                m_isSelectingWithMouse = false;
            }
        }

        if (m_isActive && m_isSelectingWithMouse) {
            m_cursorIndex = findClosestCharIndex(localMouse);
            m_selectionEnd = m_cursorIndex;
            rebuildVertices();
        }

        // 2. Обработка клавиатуры (Только если поле активно)
        if (m_isActive) {

            // БЛОК А: Системные клавиши, сочетания и стрелки (KeyPressed)
            if (auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {

                // Опрашиваем реальное состояние модификаторов в обход кодов событий
                bool ctrlPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl);

                bool shiftPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);

                // Узнаем границы выделения (если оно есть)
                size_t selMin = std::min(m_selectionStart, m_selectionEnd);
                size_t selMax = std::max(m_selectionStart, m_selectionEnd);
                bool hasSelection = (m_selectionStart != m_selectionEnd);

                // Ctrl + A (Выделить всё)
                if (keyEvent->code == sf::Keyboard::Key::A && ctrlPressed) {
                    m_selectionStart = 0;
                    m_selectionEnd = m_string.getSize();
                    m_cursorIndex = m_string.getSize();
                    rebuildVertices();
                    return;
                }
                // Ctrl + C (Копировать)
                else if (keyEvent->code == sf::Keyboard::Key::C && ctrlPressed) {
                    sf::String selected = getSelectedText();
                    if (!selected.isEmpty()) {
                        sf::Clipboard::setString(selected);
                    }
                    return;
                }
                // Ctrl + X (Вырезать) — ДОБАВЛЕНО
                else if (keyEvent->code == sf::Keyboard::Key::X && ctrlPressed) {
                    sf::String selected = getSelectedText();
                    if (!selected.isEmpty()) {
                        sf::Clipboard::setString(selected); // Копируем в буфер
                        deleteSelectedText();               // Удаляем из поля ввода
                        rebuildVertices();
                    }
                    return;
                }
                // Ctrl + V (Вставить)
                else if (keyEvent->code == sf::Keyboard::Key::V && ctrlPressed) {
                    deleteSelectedText();
                    sf::String clipboardStr = sf::Clipboard::getString();
                    m_string.insert(m_cursorIndex, clipboardStr);
                    m_cursorIndex += clipboardStr.getSize();
                    clearSelection();
                    rebuildVertices();
                    return;
                }

                // Навигация стрелочками
                if (keyEvent->code == sf::Keyboard::Key::Left) {
                    if (shiftPressed) {
                        // Расширяем выделение влево
                        if (m_cursorIndex > 0) {
                            m_cursorIndex--;
                            m_selectionEnd = m_cursorIndex;
                        }
                    }
                    else {
                        // ИСПРАВЛЕНО: Сброс выделения при движении влево
                        if (hasSelection) {
                            m_cursorIndex = selMin; // Встаем на левый край выделения
                        }
                        else if (m_cursorIndex > 0) {
                            m_cursorIndex--;        // Обычное смещение, если ничего не выделено
                        }
                        clearSelection();
                    }
                    resetCursorBlink();
                    rebuildVertices();
                }
                else if (keyEvent->code == sf::Keyboard::Key::Right) {
                    if (shiftPressed) {
                        // Расширяем выделение вправо
                        if (m_cursorIndex < m_string.getSize()) {
                            m_cursorIndex++;
                            m_selectionEnd = m_cursorIndex;
                        }
                    }
                    else {
                        // ИСПРАВЛЕНО: Сброс выделения при движении вправо
                        if (hasSelection) {
                            m_cursorIndex = selMax; // Встаем на правый край выделения
                        }
                        else if (m_cursorIndex < m_string.getSize()) {
                            m_cursorIndex++;        // Обычное смещение, если ничего не выделено
                        }
                        clearSelection();
                    }
                    resetCursorBlink();
                    rebuildVertices();
                }
                else if (keyEvent->code == sf::Keyboard::Key::Up || keyEvent->code == sf::Keyboard::Key::Down) {
                    moveCursorUpDown(keyEvent->code == sf::Keyboard::Key::Up ? -1 : 1);
                    if (shiftPressed) m_selectionEnd = m_cursorIndex;
                    else clearSelection();
                }
                else if (keyEvent->code == sf::Keyboard::Key::Delete) {
                    if (hasSelection) {
                        deleteSelectedText();
                    }
                    else if (m_cursorIndex < m_string.getSize()) {
                        m_string.erase(m_cursorIndex, 1);
                    }
                    rebuildVertices();
                }
            }


            // БЛОК Б: Независимая обработка текстового ввода (TextEntered)
            if (auto* textEvent = event.getIf<sf::Event::TextEntered>()) {
                char32_t unicode = textEvent->unicode;

                // Защита от системных кодов при зажатом Ctrl (например, прерываем ввод символа 'A' при Ctrl+A)
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl)) {
                    return;
                }

                // Фильтруем технические коды ОС меньше 32, оставляя Backspace (8), Tab (9), Enter (10/13)
                if (unicode < 32 && unicode != 8 && unicode != 9 && unicode != 10 && unicode != 13) {
                    return;
                }

                if (unicode == 8) { // Backspace
                    if (m_selectionStart != m_selectionEnd) {
                        deleteSelectedText();
                    }
                    else if (m_cursorIndex > 0) {
                        m_string.erase(m_cursorIndex - 1, 1);
                        m_cursorIndex--;
                        clearSelection();
                    }
                    rebuildVertices();
                }
                else if (unicode == 13 || unicode == 10) { // Enter
                    deleteSelectedText();
                    m_string.insert(m_cursorIndex, "\n");
                    m_cursorIndex++;
                    clearSelection();
                    rebuildVertices();
                }
                else if (unicode >= 32 && unicode != 127) { // Обычный текст
                    deleteSelectedText();
                    m_string.insert(m_cursorIndex, sf::String(unicode));
                    m_cursorIndex++;
                    clearSelection();
                    rebuildVertices();
                }
            }
        }
    }

    void update(sf::Time deltaTime) override {
        if (m_isActive) {
            m_cursorTimer += deltaTime;
            if (m_cursorTimer >= sf::seconds(0.5f)) {
                m_showCursor = !m_showCursor;
                m_cursorTimer = sf::Time::Zero;
            }
        }
        else {
            m_showCursor = false;
        }
    }

private:
    void setActive(bool active) {
        m_isActive = active;
        if (m_isActive) {
            m_background.setOutlineColor(sf::Color(0, 122, 204));
            resetCursorBlink();
        }
        else {
            m_background.setOutlineColor(sf::Color(100, 100, 100));
            m_showCursor = false;
        }
    }

    void resetCursorBlink() {
        m_showCursor = true;
        m_cursorTimer = sf::Time::Zero;
    }

    sf::Color getCharacterColor(char32_t ch, size_t index) const {
        if (ch >= '0' && ch <= '9') return sf::Color(181, 206, 168);
        if (ch == '+' || ch == '-' || ch == '*' || ch == '/' || ch == '=') return sf::Color(214, 157, 224);
        return sf::Color(220, 220, 220);
    }

    void rebuildVertices() {
        m_vertices.clear();
        m_charPositions.clear();
        if (!m_font) return;

        const float startX = 5.f;
        const float startY = 5.f;

        float xOffset = startX;
        float yOffset = startY;
        float lineSpacing = m_font->getLineSpacing(m_characterSize);

        // Резервируем память под структуру позиций для каждого символа + 1 (для позиции в самом конце)
        m_charPositions.resize(m_string.getSize() + 1);

        // Сохраняем начальную позицию (индекс 0)
        m_charPositions[0] = { xOffset, yOffset };

        for (size_t i = 0; i < m_string.getSize(); ++i) {
            char32_t curChar = m_string[i];

            if (curChar == '\n') {
                xOffset = startX;
                yOffset += lineSpacing;
                m_charPositions[i + 1] = { xOffset, yOffset };
                continue;
            }

            if (curChar == '\t') {
                const sf::Glyph& spaceGlyph = m_font->getGlyph(' ', m_characterSize, false);
                xOffset += spaceGlyph.advance * 4;
                m_charPositions[i + 1] = { xOffset, yOffset };
                continue;
            }

            const sf::Glyph& glyph = m_font->getGlyph(curChar, m_characterSize, false);

            if (i > 0 && m_string[i - 1] != '\n') {
                xOffset += m_font->getKerning(m_string[i - 1], curChar, m_characterSize);
            }

            float left = xOffset + glyph.bounds.position.x;
            float top = yOffset + glyph.bounds.position.y + m_characterSize;
            float right = left + glyph.bounds.size.x;
            float bottom = top + glyph.bounds.size.y;

            float u1 = static_cast<float>(glyph.textureRect.position.x);
            float v1 = static_cast<float>(glyph.textureRect.position.y);
            float u2 = u1 + static_cast<float>(glyph.textureRect.size.x);
            float v2 = v1 + static_cast<float>(glyph.textureRect.size.y);

            sf::Color charColor = getCharacterColor(curChar, i);

            m_vertices.append(sf::Vertex({ left,  top }, charColor, { u1, v1 }));
            m_vertices.append(sf::Vertex({ right, top }, charColor, { u2, v1 }));
            m_vertices.append(sf::Vertex({ left,  bottom }, charColor, { u1, v2 }));
            m_vertices.append(sf::Vertex({ left,  bottom }, charColor, { u1, v2 }));
            m_vertices.append(sf::Vertex({ right, top }, charColor, { u2, v1 }));
            m_vertices.append(sf::Vertex({ right, bottom }, charColor, { u2, v2 }));

            xOffset += glyph.advance;

            // Записываем координату, где начнется СЛЕДУЮЩИЙ символ
            m_charPositions[i + 1] = { xOffset, yOffset };
        }

        // Обновляем визуальное положение курсора на экране на основе его текущего индекса
        if (m_cursorIndex > m_string.getSize()) {
            m_cursorIndex = m_string.getSize();
        }
        m_cursor.setPosition(m_charPositions[m_cursorIndex]);

        // обработка выделения
        m_selectionVertices.clear();
        size_t start = std::min(m_selectionStart, m_selectionEnd);
        size_t end = std::max(m_selectionStart, m_selectionEnd);

        if (start != end && !m_charPositions.empty()) {
            float lineSpacing = m_font->getLineSpacing(m_characterSize);
            sf::Color selectionColor(0, 120, 215, 100); // Полупрозрачный синий цвет выделения

            for (size_t i = start; i < end; ++i) {
                // Если это перевод строки, синий прямоугольник рисовать не нужно
                if (m_string[i] == '\n') continue;

                sf::Vector2f curr = m_charPositions[i];
                sf::Vector2f next = m_charPositions[i + 1];

                // Если следующий символ перенесся на новую строку, берем край текущей строки
                float width = (next.y > curr.y) ? (getSize().x - curr.x) : (next.x - curr.x);

                float left = curr.x;
                float top = curr.y;
                float right = left + width;
                float bottom = top + lineSpacing;

                // Формируем квад выделения из двух треугольников
                m_selectionVertices.append(sf::Vertex({ left,  top }, selectionColor));
                m_selectionVertices.append(sf::Vertex({ right, top }, selectionColor));
                m_selectionVertices.append(sf::Vertex({ left,  bottom }, selectionColor));

                m_selectionVertices.append(sf::Vertex({ left,  bottom }, selectionColor));
                m_selectionVertices.append(sf::Vertex({ right, top }, selectionColor));
                m_selectionVertices.append(sf::Vertex({ right, bottom }, selectionColor));
            }
        }
    }

    // Умное перемещение курсора вверх/вниз
    void moveCursorUpDown(int direction) {
        if (m_string.isEmpty() || m_charPositions.empty()) return;

        sf::Vector2f currentPos = m_charPositions[m_cursorIndex];
        float lineSpacing = m_font->getLineSpacing(m_characterSize);
        float targetY = currentPos.y + (direction * lineSpacing);

        size_t bestIndex = m_cursorIndex;
        float minDistanceX = 999999.f;
        bool lineFound = false;

        // Ищем символ на целевой строке, который ближе всего к текущей координате X
        for (size_t i = 0; i < m_charPositions.size(); ++i) {
            // Проверяем, находится ли символ на нужной нам строке (с небольшой погрешностью)
            if (std::abs(m_charPositions[i].y - targetY) < 2.f) {
                lineFound = true;
                float distX = std::abs(m_charPositions[i].x - currentPos.x);
                if (distX < minDistanceX) {
                    minDistanceX = distX;
                    bestIndex = i;
                }
            }
        }

        // Если целевая строка существует, перемещаем туда индекс курсора
        if (lineFound) {
            m_cursorIndex = bestIndex;
            resetCursorBlink();
            rebuildVertices();
        }
    }

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        states = prepareStates(states);
        target.draw(m_background, states);

        // 1. Сначала рисуем выделение под текстом (без текстуры шрифта)
        if (m_isActive && m_selectionStart != m_selectionEnd) {
            sf::RenderStates selectionStates = states;
            selectionStates.texture = nullptr;
            target.draw(m_selectionVertices, selectionStates);
        }

        // 2. Затем рисуем сам текст
        if (m_font) {
            states.texture = &m_font->getTexture(m_characterSize);
            target.draw(m_vertices, states);
        }

        // 3. В конце рисуем каретку курсора
        if (m_isActive && m_showCursor) {
            sf::RenderStates cursorStates = states;
            cursorStates.texture = nullptr;
            target.draw(m_cursor, cursorStates);
        }
    }


private:
    sf::RectangleShape m_background;
    sf::VertexArray m_vertices;
    sf::String m_string;
    const sf::Font* m_font;
    unsigned int m_characterSize;
    bool m_isActive;
    sf::RectangleShape m_cursor;
    sf::Time m_cursorTimer;
    bool m_showCursor = false;
    size_t m_cursorIndex; // Текущая позиция курсора в строке
    std::vector<sf::Vector2f> m_charPositions; // Массив экранных координат для каждого индекса строки

    // Новые поля для выделения текста
    sf::VertexArray m_selectionVertices; // Геометрия синих прямоугольников выделения
    size_t m_selectionStart = 0;         // Индекс, где началось выделение
    size_t m_selectionEnd = 0;           // Индекс, где закончилось выделение
    bool m_isSelectingWithMouse = false; // Флаг удержания ЛКМ для выделения

    // Метод для получения текста, который выделен в данный момент
    sf::String getSelectedText() const {
        size_t start = std::min(m_selectionStart, m_selectionEnd);
        size_t end = std::max(m_selectionStart, m_selectionEnd);
        if (start == end) return "";
        return m_string.substring(start, end - start);
    }

    // Метод удаления выделенного фрагмента (используется при вводе нового текста поверх)
    void deleteSelectedText() {
        size_t start = std::min(m_selectionStart, m_selectionEnd);
        size_t end = std::max(m_selectionStart, m_selectionEnd);
        if (start != end) {
            m_string.erase(start, end - start);
            m_cursorIndex = start;
            m_selectionStart = m_cursorIndex;
            m_selectionEnd = m_cursorIndex;
        }
    }

    // Сброс выделения в одну точку
    void clearSelection() {
        m_selectionStart = m_cursorIndex;
        m_selectionEnd = m_cursorIndex;
    }

    size_t findClosestCharIndex(sf::Vector2f localMousePos) const {
        if (m_string.isEmpty() || m_charPositions.empty()) return 0;

        float lineSpacing = m_font->getLineSpacing(m_characterSize);
        size_t closestIndex = 0;
        float minDistance = 999999.f;

        for (size_t i = 0; i < m_charPositions.size(); ++i) {
            sf::Vector2f charPos = m_charPositions[i];

            // Проверяем попадание по высоте строки (с запасом в половину межстрочного интервала)
            if (localMousePos.y >= charPos.y && localMousePos.y <= charPos.y + lineSpacing) {
                float distX = std::abs(charPos.x - localMousePos.x);
                if (distX < minDistance) {
                    minDistance = distX;
                    closestIndex = i;
                }
            }
        }

        // Если кликнули ниже всех строк, возвращаем последний символ
        if (minDistance == 999999.f) {
            return m_string.getSize();
        }

        return closestIndex;
    }

};