#pragma once
#include <SFML/Audio.hpp> 
#include <random>
#include <string>
#include <vector>
#include <stdexcept> // Для обработки пустых массивов ассетов
#include "../ResourceManager.h"
#include "../Random.h"

class RandomSoundPlayer {
public:
    RandomSoundPlayer(std::vector<std::string> assetsNames, ResourceManager& resourceManager) :
        resourceManager(resourceManager),
        soundAssets(std::move(assetsNames)),
        // 1. Инициализируем sf::Sound первым доступным буфером, чтобы избежать ошибки
        currentSound(resourceManager.GetSoundBuffer(soundAssets.at(0)))
    {
        // Проверка на случай, если передан пустой вектор ассетов
        if (soundAssets.empty()) {
            throw std::runtime_error("RandomSoundPlayer: assetsNames list cannot be empty!");
        }
    }

    bool IsPlaying() const {
        return isPlaying;
    }

    float GetLastChosenAssetLength() const {
        return currentAssetLength;
    }

    bool PlayNext() {
        std::cout << "Random sound player - current timer = " << clock.getElapsedTime().asSeconds() << 
            ", current asset duration = " << currentAssetLength << 
            ", is playing = " << IsPlaying() << std::endl;
        if (IsPlaying()) return false;

        // Выбираем случайный индекс
        currentAssetIndex = Random::getInt(0, soundAssets.size() - 1);

        // В SFML 3 мы меняем буфер через setBuffer
        currentSound.setBuffer(resourceManager.GetSoundBuffer(soundAssets[currentAssetIndex]));

        // Запускаем воспроизведение
        currentSound.play();

        // В SFML 3 метод getBuffer() возвращает прямую ссылку (а не указатель), это безопасно
        currentAssetLength = currentSound.getBuffer().getDuration().asSeconds();
        clock.restart();

        isPlaying = true;
        return true;
    }

    bool Play(int index) {
        std::cout << "Random sound player - current timer = " << clock.getElapsedTime().asSeconds() <<
            ", current asset duration = " << currentAssetLength <<
            ", is playing = " << IsPlaying() << std::endl;
        if (IsPlaying()) return false;

        // Выбираем случайный индекс
        currentAssetIndex = index;

        // В SFML 3 мы меняем буфер через setBuffer
        currentSound.setBuffer(resourceManager.GetSoundBuffer(soundAssets[currentAssetIndex]));

        // Запускаем воспроизведение
        currentSound.play();

        // В SFML 3 метод getBuffer() возвращает прямую ссылку (а не указатель), это безопасно
        currentAssetLength = currentSound.getBuffer().getDuration().asSeconds();
        clock.restart();

        isPlaying = true;
        return true;
    }

    void Update() {
        if (!isPlaying) return;

        // Проверяем таймер
        if (clock.getElapsedTime().asSeconds() >= currentAssetLength) {
            // std::cout << "Random sound player - Update: clock timer > currentAssetLength" << std::endl;
            isPlaying = false;
            clock.stop();
        }
    }

private:
    ResourceManager& resourceManager;
    std::vector<std::string> soundAssets;
    bool isPlaying = false;
    int currentAssetIndex = 0;
    float currentAssetLength = 0.f;

    // Поле теперь гарантированно инициализируется в списке инициализации конструктора
    sf::Sound currentSound;
    sf::Clock clock;
};
