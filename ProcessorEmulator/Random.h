#pragma once
#include <random>

class Random {
public:
    // Удаляем конструктор, чтобы класс нельзя было создать через new или в стеке
    Random() = delete;

    // Статический метод для генерации чисел в диапазоне [min, max] (включительно)
    static int getInt(int min, int max) {
        // thread_local гарантирует потокобезопасность и инициализацию генератора один раз на поток
        thread_local std::mt19937 generator([]() {
            std::random_device rd;
            return rd();
        }());

        std::uniform_int_distribution<int> distribution(min, max);
        return distribution(generator);
    }
};
