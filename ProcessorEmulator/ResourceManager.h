#pragma once

#include <SFML/Graphics.hpp>
#include <SFML/Audio.hpp>
#include <unordered_map>
#include <string>
#include <string_view>
#include <iostream>

class ResourceManager {
public:
    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    ResourceManager() = default;

    const sf::Font& GetFont(std::string_view path);
    const sf::Texture& GetTexture(std::string_view path);
    sf::Music& GetMusic(std::string_view path);

    // Получить буфер звука (для sf::Sound)
    const sf::SoundBuffer& GetSoundBuffer(std::string_view path);

private:
    std::unordered_map<std::string, sf::Font> m_fonts;
    std::unordered_map<std::string, sf::Texture> m_textures;
    std::unordered_map<std::string, sf::Music> m_music;
    std::unordered_map<std::string, sf::SoundBuffer> m_soundBuffers; // Новый контейнер
};
