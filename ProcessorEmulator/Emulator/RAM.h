#pragma once
#include <iostream>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <algorithm>
#include <cstring> // Для std::memcmp

class RAM {
private:
    std::vector<unsigned int> memory;

    // Уникальная сигнатура нашей программы (4 байта)
    static constexpr char MAGIC_SIGNATURE[4] = { 'R', 'A', 'M', 'D' };

public:
    RAM(size_t N) {
        if (N == 0) {
            throw std::invalid_argument("Размер памяти должен быть больше 0.");
        }
        memory.resize(N, 0);
    }

    unsigned int read(size_t address) const {
        if (address >= memory.size()) {
            throw std::out_of_range("Ошибка чтения: Адрес выходит за пределы памяти.");
        }
        return memory[address];
    }

    void write(size_t address, unsigned int value) {
        if (address >= memory.size()) {
            throw std::out_of_range("Ошибка записи: Адрес выходит за пределы памяти.");
        }

        if (address == 0) {
            std::cout << "Предупреждение: Попытка записи в зарезервированную ячейку 0x0 проигнорирована.\n";
            return;
        }

        memory[address] = value;
    }

    void reset() {
        std::fill(memory.begin(), memory.end(), 0);
        std::cout << "Память успешно сброшена (все ячейки обнулены).\n";
    }

    // Сохранение дампа вместе с заголовком безопасности
    void saveDump(const std::string& filename) const {
        std::ofstream outFile(filename, std::ios::binary);
        if (!outFile) {
            throw std::runtime_error("Не удалось открыть файл для записи дампа: " + filename);
        }

        // 1. Записываем сигнатуру "RAMD"
        outFile.write(MAGIC_SIGNATURE, sizeof(MAGIC_SIGNATURE));

        // 2. Записываем количество ячеек памяти (размер типа size_t)
        size_t numElements = memory.size();
        outFile.write(reinterpret_cast<const char*>(&numElements), sizeof(numElements));

        // 3. Записываем сами данные памяти
        outFile.write(reinterpret_cast<const char*>(memory.data()), numElements * sizeof(unsigned int));

        std::cout << "Дамп памяти успешно сохранен в файл: " << filename << "\n";
    }

    // Загрузка дампа с жесткой проверкой формата и размера
    void loadDump(const std::string& filename) {
        std::ifstream inFile(filename, std::ios::binary);
        if (!inFile) {
            throw std::runtime_error("Не удалось открыть файл для чтения дампа: " + filename);
        }

        // 1. Проверяем сигнатуру файла
        char fileSignature[4];
        if (!inFile.read(fileSignature, sizeof(fileSignature))) {
            throw std::runtime_error("Ошибка чтения: Файл слишком короткий или поврежден.");
        }

        if (std::memcmp(fileSignature, MAGIC_SIGNATURE, sizeof(MAGIC_SIGNATURE)) != 0) {
            throw std::runtime_error("Ошибка валидации: Указанный файл не является дампом памяти этой программы.");
        }

        // 2. Считываем сохраненный размер памяти из файла
        size_t savedSize = 0;
        if (!inFile.read(reinterpret_cast<char*>(&savedSize), sizeof(savedSize))) {
            throw std::runtime_error("Ошибка чтения: Не удалось прочитать метаданные размера памяти.");
        }

        // 3. Сравниваем сохраненный размер с текущим размером объекта
        if (savedSize != memory.size()) {
            throw std::runtime_error("Ошибка валидации: Размер дампа в файле (" + std::to_string(savedSize) +
                " ячеек) не совпадает со строгим текущим размером памяти (" +
                std::to_string(memory.size()) + " ячеек).");
        }

        // 4. Считываем данные памяти напрямую в наш существующий вектор
        inFile.read(reinterpret_cast<char*>(memory.data()), savedSize * sizeof(unsigned int));

        if (inFile.gcount() != static_cast<std::streamsize>(savedSize * sizeof(unsigned int))) {
            throw std::runtime_error("Ошибка чтения: Файл дампа поврежден или содержит меньше данных, чем заявлено.");
        }

        // На всякий случай гарантируем чистоту ячейки 0x0
        memory[0] = 0;

        std::cout << "Дамп памяти успешно проверен и загружен из файла: " << filename << "\n";
    }

    size_t getSize() const {
        return memory.size();
    }
};
