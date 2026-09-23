#include "ResourceManager.h"

const sf::Font& ResourceManager::GetFont(std::string_view path) {
    std::string key(path);

    // Ищем, не загружали ли мы этот шрифт ранее
    auto it = m_fonts.find(key);
    if (it != m_fonts.end()) {
        return it->second; // Нашли! Возвращаем уже существующий
    }

    // Если не нашли, загружаем с диска
    sf::Font font;
    try {
        font.openFromFile(key);
        std::cout << "[Success] Loaded font: " << path << std::endl;
    }
    catch (const sf::Exception& e) {
        std::cerr << "[Error] Failed to load font: " << path << ". Exception: " << e.what() << std::endl;
        // В случае критической ошибки SFML 3 вернет пустой объект,
        // но программа не упадет жестко во время игры
    }

    // Сохраняем в карту и возвращаем ссылку на созданный элемент
    return m_fonts[key] = std::move(font);
}

const sf::Texture& ResourceManager::GetTexture(std::string_view path) {
    std::string key(path);

    auto it = m_textures.find(key);
    if (it != m_textures.end()) {
        return it->second;
    }

    sf::Texture texture;
    try {
        texture.loadFromFile(key);
        std::cout << "[Success] Loaded texture: " << path << std::endl;
    }
    catch (const sf::Exception& e) {
        std::cerr << "[Error] Failed to load texture: " << path << ". Exception: " << e.what() << std::endl;
    }

    return m_textures[key] = std::move(texture);
}

sf::Music& ResourceManager::GetMusic(std::string_view path) {
    std::string key(path);

    // Ищем, не загружали ли трек ранее
    auto it = m_music.find(key);
    if (it != m_music.end()) {
        return it->second;
    }

    // Создаем пустой объект sf::Music прямо внутри карты (адрес фиксируется)
    auto [insertedIt, success] = m_music.emplace(key, sf::Music{});
    sf::Music& music = insertedIt->second;

    // Настраиваем уже созданный по фиксированному адресу объект
    try {
        music.openFromFile(key);
        std::cout << "[Success] Loaded music file: " << path << std::endl;
    }
    catch (const sf::Exception& e) {
        std::cerr << "[Error] Failed to load music file: " << path << ". Exception: " << e.what() << std::endl;
    }

    return music;
}

const sf::SoundBuffer& ResourceManager::GetSoundBuffer(std::string_view path) {
    std::string key(path);

    auto it = m_soundBuffers.find(key);
    if (it != m_soundBuffers.end()) {
        return it->second;
    }

    sf::SoundBuffer buffer;
    try {
        buffer.loadFromFile(key);
        std::cout << "[Success] Loaded sound buffer: " << path << std::endl;
    }
    catch (const sf::Exception& e) {
        std::cerr << "[Error] Failed to load sound buffer: " << path << ". Exception: " << e.what() << std::endl;
    }

    return m_soundBuffers[key] = std::move(buffer);
}

sf::Shader& ResourceManager::GetShader(std::string_view path, sf::Shader::Type shaderType)
{
    std::string key(path);

    auto it = m_shaders.find(key);
    if (it != m_shaders.end()) {
        return it->second;
    }

    sf::Shader shader;
    try {
        shader.loadFromFile(key, shaderType);
        std::cout << "[Success] Loaded shader: " << path << std::endl;
    }
    catch (const sf::Exception& e) {
        std::cerr << "[Error] Failed to load shader: " << path << ". Exception: " << e.what() << std::endl;
    }

    return m_shaders[key] = std::move(shader);
}

