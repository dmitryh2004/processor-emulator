#pragma once
#include "BaseObject.h"
#include "RAMCellContainer.h"
#include "../Emulator/RAM.h"
#include "Text.h"
#include <vector>
#include <memory>
#include <string>
#include <cmath>

class RAMViewer : public BaseObject {
public:
    RAMViewer(std::string name,
        sf::Vector2f size,
        sf::Vector2f parentSize,
        RAM& ramRef,
        size_t cellCountX,
        size_t cellCountY,
        sf::Vector2f cellSize,       // Размер одной ячейки RAMCellContainer
        const sf::Font& font,
        const sf::Font& modalWindowFont,
        unsigned int characterSize = 20,
        sf::Color textColor = sf::Color::White,
        float spacing = 0.f,         // Вертикальный зазор между ячейками
        sf::Vector2f offset = sf::Vector2f(0.f, 0.f),
        Anchor parentAnchor = Anchor::TopLeft,
        Anchor localAnchor = Anchor::TopLeft,
        float rotation = 0.f,
        sf::Vector2f scale = sf::Vector2f(1.f, 1.f))
        : BaseObject(name, size, parentSize, offset, parentAnchor, localAnchor, rotation, scale),
        m_ram(ramRef),
        m_currentPage(1), // Нумерация страниц начинается с 1
        m_dumpFilename("memdump.bin")
    {
        m_cellsPerPage = cellCountX * cellCountY;
        // Вычисляем общее количество доступных страниц в RAM
        m_totalPages = static_cast<size_t>(std::ceil(static_cast<double>(m_ram.getSize()) / m_cellsPerPage));
        if (m_totalPages == 0) m_totalPages = 1;

        std::cout << "[" << name << "] Расчетное количество страниц памяти: " << m_totalPages << std::endl;

        // Создаем M ячеек RAMCellContainer и выстраиваем их сеткой
        for (size_t i = 0; i < m_cellsPerPage; ++i) {
            int x = i % cellCountX;
            int y = i / cellCountX;

            // Вычисляем смещение для i-й ячейки по вертикали
            float xPos = x * (cellSize.x + spacing);
            float yPos = y * (cellSize.y + spacing);

            auto cell = std::make_unique<RAMCellContainer>(
                name + "_cell_" + std::to_string(i),
                cellSize,
                size, // Родителем является сам RAMViewer
                font,
                modalWindowFont,
                0,    // Начальный адрес (обновится в updateCellsFromMemory)
                0,    // Начальное значение (обновится в updateCellsFromMemory)
                characterSize,
                textColor,
                sf::Vector2f(xPos, yPos), // Смещение относительно TopLeft вьювера
                Anchor::TopLeft,
                Anchor::TopLeft
            );
            m_cells.push_back(std::move(cell));
        }

        // Заполняем ячейки актуальными данными из RAM для первой страницы
        updateCellsFromMemory();
    }

    // --- Логика переключения страниц ---

    // Установка конкретной страницы с проверкой корректности границ
    bool setPage(size_t page) {
        if (page >= 1 && page <= m_totalPages) {
            m_currentPage = page;
            updateCellsFromMemory();
            return true;
        }
        return false; // Страница вне диапазона
    }

    // Переход на следующую страницу
    void nextPage() {
        if (m_currentPage < m_totalPages) {
            m_currentPage++;
            updateCellsFromMemory();
        }
    }

    // Переход на предыдущую страницу
    void prevPage() {
        if (m_currentPage > 1) {
            m_currentPage--;
            updateCellsFromMemory();
        }
    }

    size_t getCurrentPage() const { return m_currentPage; }
    size_t getTotalPages() const { return m_totalPages; }

    // --- Прокси-методы для управления памятью ---

    void saveDump() const {
        m_ram.saveDump(m_dumpFilename);
    }

    void loadDump() {
        m_ram.loadDump(m_dumpFilename);
        // После загрузки дампа данные в RAM изменились, обновляем экран
        updateCellsFromMemory();
    }

    void reset() {
        m_ram.reset();
        // После обнуления обновляем экран
        updateCellsFromMemory();
    }

    // --- Переопределение методов базового класса ---

    void checkForEvents(const sf::Event& event, const sf::RenderWindow& window, sf::Vector2f localMousePos) override {
        // Пробрасываем событие во все дочерние ячейки.
        // Для каждой ячейки нужно скорректировать координаты мыши, 
        // вычтя из текущей позиции мыши локальную позицию самой ячейки.
        if (auto* mouseButtonEvent = event.getIf<sf::Event::MouseButtonPressed>()) {
            if (mouseButtonEvent->button == sf::Mouse::Button::Left) {
                sf::Vector2f cellLocalMouse = localMousePos - getPosition();

                for (auto& cell : m_cells) {
                    cell->checkForEvents(event, window, cellLocalMouse - cell->getPosition());
                }

                // Если дочерняя ячейка изменила значение внутри своего модального окна, 
                // нам необходимо синхронизировать это изменение обратно в оперативную память.
                // А также обновить отображение (на случай, если изменилась ячейка 0x0 и сработал "reserved")
                syncChangesToRAM();
            }
        }
    }

    void update(sf::Time deltaTime) override {
        for (auto& cell : m_cells) {
            cell->update(deltaTime);
        }
    }

protected:
    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        // Объединяем трансформации
        sf::RenderStates localStates = prepareStates(states);

        // Отрисовываем все ячейки памяти
        for (const auto& cell : m_cells) {
            target.draw(*cell, localStates);
        }
    }

private:
    RAM& m_ram;                                             // Ссылка на объект эмуляции памяти
    std::vector<std::unique_ptr<RAMCellContainer>> m_cells; // Массив контейнеров ячеек (M штук)
    size_t m_cellsPerPage;                                  // Количество ячеек на странице (M)
    size_t m_currentPage;                                   // Индекс текущей страницы (от 1)
    size_t m_totalPages;                                    // Общее количество страниц
    const std::string m_dumpFilename;                       // Фиксированное имя файла

    // Метод обновления данных в графических ячейках на основе текущей страницы RAM
    void updateCellsFromMemory() {
        // Вычисляем начальный индекс ячейки памяти для текущей страницы
        size_t startAddress = (m_currentPage - 1) * m_cellsPerPage;

        for (size_t i = 0; i < m_cellsPerPage; ++i) {
            size_t currentAddress = startAddress + i;

            if (currentAddress < m_ram.getSize()) {
                // Если адрес существует в RAM, считываем данные
                m_cells[i]->setAddress(static_cast<uint32_t>(currentAddress));
                m_cells[i]->setValue(m_ram.read(currentAddress));
            }
            else {
                // Если память RAM закончилась (последняя страница заполнена не до конца),
                // принудительно зануляем или скрываем неиспользуемые контейнеры
                m_cells[i]->setAddress(0); // Это автоматически выведет "reserved"
            }
        }
    }

    // Метод для синхронизации изменений из UI-ячеек обратно в объект RAM
    void syncChangesToRAM() {
        size_t startAddress = (m_currentPage - 1) * m_cellsPerPage;

        for (size_t i = 0; i < m_cellsPerPage; ++i) {
            size_t currentAddress = startAddress + i;

            if (currentAddress < m_ram.getSize()) {
                uint32_t uiValue = m_cells[i]->getValue();

                // Если значение в графической ячейке отличается от того, что в RAM,
                // значит пользователь изменил его через модальное окно. Записываем в RAM.
                if (m_ram.read(currentAddress) != uiValue) {
                    m_ram.write(currentAddress, uiValue);

                    // Перечитываем обратно на случай, если запись была заблокирована (как для 0x0)
                    m_cells[i]->setValue(m_ram.read(currentAddress));
                }
            }
        }
    }
};
