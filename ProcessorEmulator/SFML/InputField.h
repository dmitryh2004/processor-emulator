#pragma once
#include "BaseObject.h"
#include <vector>
#include <cmath>
#include <algorithm>
#include "SyntaxHighlighter.h"
#include <SFML/OpenGL.hpp> // Добавлено для glScissor

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
        m_font(&font), m_characterSize(characterSize), m_isActive(false), m_cursorIndex(0),
        m_scrollOffset(0.f, 0.f), m_lineNumbersWidth(50.f) // Начальная ширина панели номеров строк
    {
        // Настройка фона
        m_background.setSize(getSize());
        m_background.setFillColor(sf::Color(30, 30, 30));
        m_background.setOutlineColor(sf::Color(100, 100, 100));
        m_background.setOutlineThickness(1.f);

        // Настройка фона панели номеров строк
        m_lineNumbersBackground.setFillColor(sf::Color(40, 40, 40));
        m_lineNumbersBackground.setOutlineColor(sf::Color(60, 60, 60));
        m_lineNumbersBackground.setOutlineThickness(1.f);

        // Настройка геометрии букв и выделения
        m_vertices.setPrimitiveType(sf::PrimitiveType::Triangles);
        m_selectionVertices.setPrimitiveType(sf::PrimitiveType::Triangles);
        m_lineNumbersVertices.setPrimitiveType(sf::PrimitiveType::Triangles);

        // Настройка курсора
        m_cursor.setSize({ 2.f, static_cast<float>(m_characterSize) });
        m_cursor.setFillColor(sf::Color::White);

        loadSyntaxScheme(defaultColorConfigPath);
        rebuildVertices();
    }

    bool loadSyntaxScheme(const std::string& configPath) {
        bool success = m_highlighter.loadFromConf(configPath);
        if (success) {
            rebuildVertices();
        }
        return success;
    }

    std::string getTextString() const {
        return m_string.toAnsiString();
    }

    void setTextString(const std::string& text) {
        m_string = text;
        m_cursorIndex = m_string.getSize();
        rebuildVertices();
        scrollToCursor();
    }

    void checkForEvents(const sf::Event& event, const sf::RenderWindow& window, sf::Vector2f localMousePos) override {
        sf::FloatRect globalBounds({ 0.f, 0.f }, getSize());
        globalBounds = getTransform().transformRect(globalBounds);

        sf::Vector2f worldMousePos = window.mapPixelToCoords(sf::Mouse::getPosition(window));

        // Переводим координаты из пространства родительской Panel в локальное пространство InputField
        sf::Vector2f trueLocalMouse = getTransform().getInverse().transformPoint(localMousePos);

        // 1. Обработка прокрутки колесиком мыши
        if (auto* scrollEvent = event.getIf<sf::Event::MouseWheelScrolled>()) {
            if (globalBounds.contains(worldMousePos)) {
                if (scrollEvent->wheel == sf::Mouse::Wheel::Vertical) {
                    // Вертикальный скролл (обычный или с Shift для горизонтального, если мышь поддерживает)
                    float delta = scrollEvent->delta * m_font->getLineSpacing(m_characterSize) * 1.5f;
                    if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift)) {
                        m_scrollOffset.x = std::max(0.f, m_scrollOffset.x - delta);
                    }
                    else {
                        m_scrollOffset.y = std::max(0.f, m_scrollOffset.y - delta);
                    }
                }
                clampScroll();
            }
        }

        // 2. Обработка мыши (Клик, Отпускание, Выделение)
        if (auto* mouseBtnEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (globalBounds.contains(worldMousePos)) {
                if (mouseBtnEvent->button == sf::Mouse::Button::Left) {
                    setActive(true);
                    m_isSelectingWithMouse = true;

                    // Учитываем прокрутку при клике мышкой
                    sf::Vector2f scrolledMouse = trueLocalMouse + m_scrollOffset;
                    m_cursorIndex = findClosestCharIndex(scrolledMouse);
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
            sf::Vector2f scrolledMouse = trueLocalMouse + m_scrollOffset;
            m_cursorIndex = findClosestCharIndex(scrolledMouse);
            m_selectionEnd = m_cursorIndex;
            rebuildVertices();
        }

        // 3. Обработка клавиатуры (Только если поле активно)
        if (m_isActive) {
            if (auto* keyEvent = event.getIf<sf::Event::KeyPressed>()) {
                bool ctrlPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl);
                bool shiftPressed = sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LShift) ||
                    sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RShift);

                size_t selMin = std::min(m_selectionStart, m_selectionEnd);
                size_t selMax = std::max(m_selectionStart, m_selectionEnd);
                bool hasSelection = (m_selectionStart != m_selectionEnd);

                // Ctrl + A
                if (keyEvent->code == sf::Keyboard::Key::A && ctrlPressed) {
                    m_selectionStart = 0;
                    m_selectionEnd = m_string.getSize();
                    m_cursorIndex = m_string.getSize();
                    rebuildVertices();
                    scrollToCursor();
                    return;
                }
                // Ctrl + C
                else if (keyEvent->code == sf::Keyboard::Key::C && ctrlPressed) {
                    sf::String selected = getSelectedText();
                    if (!selected.isEmpty()) sf::Clipboard::setString(selected);
                    return;
                }
                // Ctrl + X
                else if (keyEvent->code == sf::Keyboard::Key::X && ctrlPressed) {
                    sf::String selected = getSelectedText();
                    if (!selected.isEmpty()) {
                        sf::Clipboard::setString(selected);
                        deleteSelectedText();
                        rebuildVertices();
                        scrollToCursor();
                    }
                    return;
                }
                // Ctrl + V
                else if (keyEvent->code == sf::Keyboard::Key::V && ctrlPressed) {
                    deleteSelectedText();
                    sf::String clipboardStr = sf::Clipboard::getString();
                    m_string.insert(m_cursorIndex, clipboardStr);
                    m_cursorIndex += clipboardStr.getSize();
                    clearSelection();
                    rebuildVertices();
                    scrollToCursor();
                    return;
                }

                // Навигация
                if (keyEvent->code == sf::Keyboard::Key::Left) {
                    if (shiftPressed) {
                        if (m_cursorIndex > 0) { m_cursorIndex--; m_selectionEnd = m_cursorIndex; }
                    }
                    else {
                        if (hasSelection) m_cursorIndex = selMin;
                        else if (m_cursorIndex > 0) m_cursorIndex--;
                        clearSelection();
                    }
                    resetCursorBlink();
                    rebuildVertices();
                    scrollToCursor();
                }
                else if (keyEvent->code == sf::Keyboard::Key::Right) {
                    if (shiftPressed) {
                        if (m_cursorIndex < m_string.getSize()) { m_cursorIndex++; m_selectionEnd = m_cursorIndex; }
                    }
                    else {
                        if (hasSelection) m_cursorIndex = selMax;
                        else if (m_cursorIndex < m_string.getSize()) m_cursorIndex++;
                        clearSelection();
                    }
                    resetCursorBlink();
                    rebuildVertices();
                    scrollToCursor();
                }
                else if (keyEvent->code == sf::Keyboard::Key::Up || keyEvent->code == sf::Keyboard::Key::Down) {
                    moveCursorUpDown(keyEvent->code == sf::Keyboard::Key::Up ? -1 : 1);
                    if (shiftPressed) m_selectionEnd = m_cursorIndex;
                    else clearSelection();
                    rebuildVertices();
                    scrollToCursor();
                }
                else if (keyEvent->code == sf::Keyboard::Key::Delete) {
                    if (hasSelection) deleteSelectedText();
                    else if (m_cursorIndex < m_string.getSize()) m_string.erase(m_cursorIndex, 1);
                    rebuildVertices();
                    scrollToCursor();
                }
            }

            if (auto* textEvent = event.getIf<sf::Event::TextEntered>()) {
                char32_t unicode = textEvent->unicode;
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::LControl) || sf::Keyboard::isKeyPressed(sf::Keyboard::Key::RControl)) return;
                if (unicode < 32 && unicode != 8 && unicode != 9 && unicode != 10 && unicode != 13) return;

                if (unicode == 8) { // Backspace
                    if (m_selectionStart != m_selectionEnd) deleteSelectedText();
                    else if (m_cursorIndex > 0) {
                        m_string.erase(m_cursorIndex - 1, 1);
                        m_cursorIndex--;
                        clearSelection();
                    }
                    rebuildVertices();
                    scrollToCursor();
                }
                else if (unicode == 13 || unicode == 10) {
                    // Enter
                    deleteSelectedText();
                    m_string.insert(m_cursorIndex, "\n");
                    m_cursorIndex++;
                    clearSelection();
                    rebuildVertices();
                    scrollToCursor();
                }
                else if (unicode >= 32 && unicode != 127) {
                    // Текст
                    deleteSelectedText();
                    m_string.insert(m_cursorIndex, sf::String(unicode));
                    m_cursorIndex++;
                    clearSelection();
                    rebuildVertices();
                    scrollToCursor();
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
       // Автоматическая подгонка ширины колонки номеров строк в зависимости от их количества
       void updateLineNumbersWidth(size_t totalLines) {
           int digits = 1;
           size_t temp = totalLines;
           while (temp /= 10) digits++;
           // Примерно по 12 пикселей на цифру + отступы (минимум 40px)
           m_lineNumbersWidth = std::max(40.f, digits * 12.f + 15.f);
           m_lineNumbersBackground.setSize({
            m_lineNumbersWidth, getSize().y }
            );
       }
       void rebuildVertices() {
           m_vertices.clear();
           m_charPositions.clear();
           m_lineNumbersVertices.clear();
           if (!m_font) return;
           // Определяем динамическую ширину панели под нумерацию строк
           size_t totalLines = 1;
           for (size_t i = 0;
               i < m_string.getSize();
               ++i) {
               if (m_string[i] == '\n') totalLines++;
           }
           updateLineNumbersWidth(totalLines);
           // Текст начинается СРАЗУ после панели номеров строк
           const float startX = m_lineNumbersWidth + 5.f;
           const float startY = 5.f;
           float xOffset = startX;
           float yOffset = startY;
           float lineSpacing = m_font->getLineSpacing(m_characterSize);
           m_charPositions.resize(m_string.getSize() + 1);
           m_charPositions[0] = { xOffset, yOffset };
           // Сборка номеров строк (всегда генерируем первую строку)
           size_t currentLineNum = 1;
           auto addLineNumberVertices = [&](size_t num, float y) {
               sf::String numStr = std::to_string(num);
               float numX = m_lineNumbersWidth - 10.f;
               // Выравнивание по правому краю колонки
               for (int i = numStr.getSize() - 1; i >= 0; --i) {
                   char32_t c = numStr[i];
                   const sf::Glyph& glyph = m_font->getGlyph(c, m_characterSize, false);
                   numX -= glyph.advance;
                   float left = numX + glyph.bounds.position.x;
                   float top = y + glyph.bounds.position.y + m_characterSize;
                   float right = left + glyph.bounds.size.x;
                   float bottom = top + glyph.bounds.size.y;
                   float u1 = static_cast<float>(glyph.textureRect.position.x);
                   float v1 = static_cast<float>(glyph.textureRect.position.y);
                   float u2 = u1 + static_cast<float>(glyph.textureRect.size.x);
                   float v2 = v1 + static_cast<float>(glyph.textureRect.size.y);
                   sf::Color numColor(120, 120, 120);
                   // Серый цвет для номеров
                   m_lineNumbersVertices.append(sf::Vertex({
                   left,  top }
                   , numColor, {
                    u1, v1 }
                    ));
                   m_lineNumbersVertices.append(sf::Vertex({
                    right, top }
                    , numColor, {
                     u2, v1 }
                     ));
                   m_lineNumbersVertices.append(sf::Vertex({
                    left,  bottom }
                    , numColor, {
                     u1, v2 }
                     ));
                   m_lineNumbersVertices.append(sf::Vertex({
                    left,  bottom }
                    , numColor, {
                     u1, v2 }
                     ));
                   m_lineNumbersVertices.append(sf::Vertex({
                    right, top }
                    , numColor, {
                     u2, v1 }
                     ));
                   m_lineNumbersVertices.append(sf::Vertex({
                    right, bottom }
                    , numColor, {
                     u2, v2 }
                     ));
               }
           }
           ;
           addLineNumberVertices(currentLineNum, yOffset);
           // Поддержка кириллицы через Юникод-конвертер UTF-8
           sf::U8String utf8Str = m_string.toUtf8();
           std::string ansiStr(utf8Str.begin(), utf8Str.end());
           std::vector<sf::Color> textColors = m_highlighter.highlight(ansiStr);
           m_contentSize = sf::Vector2f(startX, yOffset + lineSpacing);
           for (size_t i = 0;
               i < m_string.getSize();
               ++i) {
               char32_t curChar = m_string[i];
               if (curChar == '\n') {
                   xOffset = startX;
                   yOffset += lineSpacing;
                   m_charPositions[i + 1] = {
                    xOffset, yOffset }
                   ;
                   currentLineNum++;
                   addLineNumberVertices(currentLineNum, yOffset);
                   m_contentSize.y = std::max(m_contentSize.y, yOffset + lineSpacing);
                   continue;
               }
               if (curChar == '\t') {
                   const sf::Glyph& spaceGlyph = m_font->getGlyph(' ', m_characterSize, false);
                   xOffset += spaceGlyph.advance * 4;
                   m_charPositions[i + 1] = {
                    xOffset, yOffset }
                   ;
                   m_contentSize.x = std::max(m_contentSize.x, xOffset);
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
               sf::Color charColor = (i < textColors.size()) ? textColors[i] : sf::Color::White;
               m_vertices.append(sf::Vertex({
                left,  top }
                , charColor, {
                 u1, v1 }
                 ));
               m_vertices.append(sf::Vertex({
                right, top }
                , charColor, {
                 u2, v1 }
                 ));
               m_vertices.append(sf::Vertex({
                left,  bottom }
                , charColor, {
                 u1, v2 }
                 ));
               m_vertices.append(sf::Vertex({
                left,  bottom }
                , charColor, {
                 u1, v2 }
                 ));
               m_vertices.append(sf::Vertex({
                right, top }
                , charColor, {
                 u2, v1 }
                 ));
               m_vertices.append(sf::Vertex({
                right, bottom }
                , charColor, {
                 u2, v2 }
                 ));
               xOffset += glyph.advance;
               m_charPositions[i + 1] = {
                xOffset, yOffset }
               ;
               m_contentSize.x = std::max(m_contentSize.x, xOffset);
           }
           if (m_cursorIndex > m_string.getSize())  m_cursorIndex = m_string.getSize();
           m_cursor.setPosition(m_charPositions[m_cursorIndex]);
           // Сборка полигонов выделения
           m_selectionVertices.clear();
           size_t start = std::min(m_selectionStart, m_selectionEnd);
           size_t end = std::max(m_selectionStart, m_selectionEnd);
           if (start != end && !m_charPositions.empty()) {
               sf::Color selectionColor(0, 120, 215, 100);
               for (size_t i = start;
                   i < end;
                   ++i) {
                   if (m_string[i] == '\n') continue;
                   sf::Vector2f curr = m_charPositions[i];
                   sf::Vector2f next = m_charPositions[i + 1];
                   float width = (next.y > curr.y) ? (m_contentSize.x - curr.x) : (next.x - curr.x);
                   float left = curr.x;
                   float top = curr.y;
                   float right = left + width;
                   float bottom = top + lineSpacing;
                   m_selectionVertices.append(sf::Vertex({
                    left,  top }
                   , selectionColor));
                   m_selectionVertices.append(sf::Vertex({
                    right, top }
                   , selectionColor));
                   m_selectionVertices.append(sf::Vertex({
                    left,  bottom }
                   , selectionColor));
                   m_selectionVertices.append(sf::Vertex({
                    left,  bottom }
                   , selectionColor));
                   m_selectionVertices.append(sf::Vertex({
                    right, top }
                   , selectionColor));
                   m_selectionVertices.append(sf::Vertex({
                    right, bottom }
                   , selectionColor));
               }
           }
           clampScroll();
       }
       // Автоматическая корректировка камеры вслед за кареткой
       void scrollToCursor() {
           if (m_charPositions.empty()) return;
           sf::Vector2f cursorBox = m_charPositions[m_cursorIndex];
           float lineSpacing = m_font->getLineSpacing(m_characterSize);
           // Горизонтальный скролл
           float minVisibleX = m_scrollOffset.x + m_lineNumbersWidth + 10.f;
           float maxVisibleX = m_scrollOffset.x + getSize().x - 20.f;
           if (cursorBox.x < minVisibleX) {
               m_scrollOffset.x = std::max(0.f, cursorBox.x - m_lineNumbersWidth - 10.f);
           }
           else if (cursorBox.x > maxVisibleX) {
               m_scrollOffset.x = cursorBox.x - getSize().x + 20.f;
           }
           // Вертикальный скролл
           float minVisibleY = m_scrollOffset.y + 5.f;
           float maxVisibleY = m_scrollOffset.y + getSize().y - lineSpacing - 5.f;
           if (cursorBox.y < minVisibleY) {
               m_scrollOffset.y = std::max(0.f, cursorBox.y - 5.f);
           }
           else if (cursorBox.y > maxVisibleY) {
               m_scrollOffset.y = cursorBox.y - getSize().y + lineSpacing + 5.f;
           }
           clampScroll();
       }
       void clampScroll() {
           float maxScrollX = std::max(0.f, m_contentSize.x - getSize().x + 20.f);
           float maxScrollY = std::max(0.f, m_contentSize.y - getSize().y + 10.f);
           m_scrollOffset.x = std::clamp(m_scrollOffset.x, 0.f, maxScrollX);
           m_scrollOffset.y = std::clamp(m_scrollOffset.y, 0.f, maxScrollY);
       }
       void moveCursorUpDown(int direction) {
           if (m_string.isEmpty() || m_charPositions.empty()) return;
           sf::Vector2f currentPos = m_charPositions[m_cursorIndex];
           float lineSpacing = m_font->getLineSpacing(m_characterSize);
           float targetY = currentPos.y + (direction * lineSpacing);
           size_t bestIndex = m_cursorIndex;
           float minDistanceX = 999999.f;
           bool lineFound = false;
           for (size_t i = 0;
               i < m_charPositions.size();
               ++i) {
               if (std::abs(m_charPositions[i].y - targetY) < 2.f) {
                   lineFound = true;
                   float distX = std::abs(m_charPositions[i].x - currentPos.x);
                   if (distX < minDistanceX) {
                       minDistanceX = distX;
                       bestIndex = i;
                   }
               }
           }
           if (lineFound) {
               m_cursorIndex = bestIndex;
               resetCursorBlink();
           }
       }
protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
    sf::RenderStates originalStates = states;
    states = prepareStates(states);
    // Рисуем общий фон InputField (не подлежит скроллингу)
    target.draw(m_background, states);
    // Настройка OpenGL Scissor-теста для отсечения текста, вылезающего за рамки
    bool scissorApplied = false;
    sf::Transform finalTransform = originalStates.transform * getTransform();
    sf::Vector2f mySize = getSize();
    sf::Vector2f topLeft = finalTransform.transformPoint({
     0.f, 0.f }
     );
    sf::Vector2f bottomRight = finalTransform.transformPoint(mySize);
    sf::Vector2i targetTopLeft = target.mapCoordsToPixel(topLeft);
    sf::Vector2i targetBottomRight = target.mapCoordsToPixel(bottomRight);
    int scissorX = targetTopLeft.x;
    int scissorWidth = targetBottomRight.x - targetTopLeft.x;
    int scissorHeight = targetBottomRight.y - targetTopLeft.y;
    int scissorY = static_cast<float>(target.getSize().y) - targetBottomRight.y;
    if (scissorWidth > 0 && scissorHeight > 0) {
        glEnable(GL_SCISSOR_TEST);
        glScissor(scissorX, scissorY, scissorWidth, scissorHeight);
        scissorApplied = true;
    }
    else {
        return;
    }
    // РЕНДЕРИНГ КОНТЕНТА С УЧЕТОМ СКРОЛЛА
    sf::RenderStates scrolledStates = states;
    // Смещаем матрицу рендеринга на вектор скролла (за исключением оси X для номеров строк)
    scrolledStates.transform.translate(-m_scrollOffset);
    // 1. Отрисовка выделения (под текстом)
    if (m_isActive && m_selectionStart != m_selectionEnd) {
        sf::RenderStates selectStates = scrolledStates;
        selectStates.texture = nullptr;
        target.draw(m_selectionVertices, selectStates);
    }
    // 2. Отрисовка кода
    if (m_font) {
        scrolledStates.texture = &m_font->getTexture(m_characterSize);
        target.draw(m_vertices, scrolledStates);
    }
    // 3. Отрисовка каретки курсора
    if (m_isActive && m_showCursor) {
        sf::RenderStates cursorStates = scrolledStates;
        cursorStates.texture = nullptr;
        target.draw(m_cursor, cursorStates);
    }
    // РЕНДЕРИНГ ПАНЕЛИ НОМЕРОВ СТРОК (она скроллится только по VERTICAL, по HORIZONTAL зафиксирована)
    sf::RenderStates lineNumStates = states;
    target.draw(m_lineNumbersBackground, lineNumStates);
    // Статичный фон колонки
    // Применяем вертикальный скролл к цифрам номеров строк
    lineNumStates.transform.translate({
    0.f, -m_scrollOffset.y }
    );
    if (m_font) {
        lineNumStates.texture = &m_font->getTexture(m_characterSize);
        target.draw(m_lineNumbersVertices, lineNumStates);
    }
    if (scissorApplied) {
        glDisable(GL_SCISSOR_TEST);
        target.resetGLStates();
    }
}
private:
    sf::RectangleShape m_background;
       sf::VertexArray m_vertices;
       sf::String m_string;
       const sf::Font* m_font;
       const sf::String defaultColorConfigPath = "SFML/ColorConfigs/default.conf";
       unsigned int m_characterSize;
       bool m_isActive;
       sf::RectangleShape m_cursor;
       sf::Time m_cursorTimer;
       bool m_showCursor = false;
       size_t m_cursorIndex;
       std::vector<sf::Vector2f> m_charPositions;
       SyntaxHighlighter m_highlighter;
       sf::VertexArray m_selectionVertices;
       size_t m_selectionStart = 0;
       size_t m_selectionEnd = 0;
       bool m_isSelectingWithMouse = false;
       // НОВЫЕ ПОЛЯ ДЛЯ СКРОЛЛИНГА И НУМЕРАЦИИ
       sf::Vector2f m_scrollOffset;
       // Текущее смещение камеры (x - горизонтальное, y - вертикальное)
       sf::Vector2f m_contentSize;
       // Реальный геометрический размер всего текста в пикселях
       float m_lineNumbersWidth;
       // Текущая ширина колонки номеров строк
       sf::RectangleShape m_lineNumbersBackground;
       // Фон для левой колонки номеров
       sf::VertexArray m_lineNumbersVertices;
       // Текстурированные полигоны для цифр номеров строк
       sf::String getSelectedText() const {
           size_t start = std::min(m_selectionStart, m_selectionEnd);
           size_t end = std::max(m_selectionStart, m_selectionEnd);
           if (start == end) return "";
           return m_string.substring(start, end - start);
       }
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
       void clearSelection() {
           m_selectionStart = m_cursorIndex;
           m_selectionEnd = m_cursorIndex;
       }
       size_t findClosestCharIndex(sf::Vector2f localMousePos) const {
           if (m_string.isEmpty() || m_charPositions.empty()) return 0;
           float lineSpacing = m_font->getLineSpacing(m_characterSize);
           size_t closestIndex = 0;
           float minDistance = 999999.f;
           for (size_t i = 0;
               i < m_charPositions.size();
               ++i) {
               sf::Vector2f charPos = m_charPositions[i];
               if (localMousePos.y >= charPos.y && localMousePos.y <= charPos.y + lineSpacing) {
                   float distX = std::abs(charPos.x - localMousePos.x);
                   if (distX < minDistance) {
                       minDistance = distX;
                       closestIndex = i;
                   }
               }
           }
           if (minDistance == 999999.f) {
               return m_string.getSize();
           }
           return closestIndex;
       }
}
;
