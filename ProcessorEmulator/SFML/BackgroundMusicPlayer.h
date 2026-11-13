#pragma once

#include <SFML/Audio.hpp>
#include <SFML/System/Clock.hpp>
#include <SFML/System/Time.hpp>
#include <vector>
#include <string>
#include <random>
#include <stdexcept>
#include "../ResourceManager.h"

class BackgroundMusicPlayer {
public:
    BackgroundMusicPlayer(const std::vector<std::string>& trackPaths,
        ResourceManager& resources,
        float switchIntervalMinutes = 5.0f)
        : m_trackPaths(trackPaths),
        m_resources(resources),
        m_switchInterval(sf::seconds(switchIntervalMinutes * 60.0f)),
        m_rng(std::random_device{}())
    {
        if (m_trackPaths.empty()) {
            throw std::invalid_argument("Track list cannot be empty");
        }
        playNextRandomTrack();
    }

    // Метод необходимо вызывать в основном игровом цикле (вместе с update)
    void update() {
        if (!m_currentMusic) {
            return;
        }

        // 1. Проверяем, не пора ли разрешить треку завершиться
        if (!m_waitingForTrackEnd && m_timer.getElapsedTime() >= m_switchInterval) {
            m_currentMusic->setLooping(false); // Отключаем повтор, чтобы трек доиграл до конца
            m_waitingForTrackEnd = true;
        }

        // 2. Если мы ждём окончания трека и он остановился — запускаем следующий
        if (m_waitingForTrackEnd && m_currentMusic->getStatus() == sf::SoundSource::Status::Stopped) {
            playNextRandomTrack();
        }
    }

    void stop() {
        if (m_currentMusic) {
            m_currentMusic->stop();
        }
    }

    void pause() {
        if (m_currentMusic) {
            m_currentMusic->pause();
        }
    }

    void resume() {
        if (m_currentMusic && m_currentMusic->getStatus() == sf::SoundSource::Status::Paused) {
            m_currentMusic->play();
        }
    }

private:
    void playNextRandomTrack() {
        if (m_currentMusic) {
            m_currentMusic->stop();
        }

        std::size_t nextIndex = pickRandomIndex();
        m_currentIndex = nextIndex;

        std::cout << "[BackgroundMusicPlayer] Loading new music (id = " << nextIndex+1 << ")" << std::endl;

        // Получаем ссылку на sf::Music из ResourceManager
        m_currentMusic = &m_resources.GetMusic(m_trackPaths[m_currentIndex]);

        m_currentMusic->setLooping(true);
        m_currentMusic->play();

        // Сбрасываем таймер и флаг ожидания
        m_timer.restart();
        m_waitingForTrackEnd = false;
    }

    std::size_t pickRandomIndex() {
        if (m_trackPaths.size() == 1) {
            return 0;
        }

        // Выбираем случайный индекс, отличный от текущего, чтобы трек не повторялся дважды подряд
        std::uniform_int_distribution<std::size_t> dist(0, m_trackPaths.size() - 2);
        std::size_t idx = dist(m_rng);
        if (idx >= m_currentIndex) {
            ++idx;
        }
        return idx;
    }

private:
    std::vector<std::string> m_trackPaths;
    ResourceManager& m_resources;
    sf::Time m_switchInterval;

    sf::Music* m_currentMusic = nullptr;
    std::size_t m_currentIndex = static_cast<std::size_t>(-1);

    sf::Clock m_timer;
    bool m_waitingForTrackEnd = false;

    std::mt19937 m_rng;
};
