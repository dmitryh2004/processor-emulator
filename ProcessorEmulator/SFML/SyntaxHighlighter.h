#pragma once
#include <SFML/Graphics/Color.hpp>
#include <string>
#include <vector>
#include <regex>
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cctype>

struct SyntaxRule {
    std::regex regex;
    sf::Color color;

    SyntaxRule(const std::string& pattern, sf::Color col)
        : regex(pattern, std::regex_constants::optimize), color(col) {
    }
};

class SyntaxHighlighter {
public:
    SyntaxHighlighter() = default;

    // Загрузка цветовой схемы из файла конфигурации .conf
    bool loadFromConf(const std::string& filepath) {
        std::ifstream file(filepath);
        if (!file.is_open()) {
            std::cerr << "[SyntaxHighlighter] Failed to open file: " << filepath << std::endl;
            return false;
        }

        m_rules.clear();
        std::string line;
        size_t lineNumber = 0;

        while (std::getline(file, line)) {
            lineNumber++;

            // 1. Удаляем пробелы в самом начале строки для удобства выравнивания в файле
            line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));

            // 2. Игнорируем пустые строки и комментарии, начинающиеся с //
            if (line.empty() || (line.size() >= 2 && line.compare(0, 2, "//") == 0)) {
                continue;
            }

            // 3. Находим первый пробел, разделяющий [HEX] и [Регулярное выражение]
            size_t spacePos = line.find(' ');
            if (spacePos == std::string::npos || spacePos == 0) {
                std::cerr << "[SyntaxHighlighter] Line " << lineNumber << " missing separator space." << std::endl;
                continue;
            }

            std::string hexStr = line.substr(0, spacePos);
            std::string regexPattern = line.substr(spacePos + 1);

            // 4. Парсим HEX-код в sf::Color
            sf::Color color;
            if (!parseHexColor(hexStr, color)) {
                std::cerr << "[SyntaxHighlighter] Line " << lineNumber << " has invalid HEX color: " << hexStr << std::endl;
                continue;
            }

            // 5. Проверяем корректность регулярного выражения перед сохранением
            try {
                m_rules.emplace_back(regexPattern, color);
            }
            catch (const std::regex_error& e) {
                std::cerr << "[SyntaxHighlighter] Line " << lineNumber << " has invalid regex pattern: " << e.what() << std::endl;
            }
        }

        return true;
    }

    void addRule(const std::string& pattern, sf::Color color) {
        m_rules.emplace_back(pattern, color);
    }

    std::vector<sf::Color> highlight(const std::string& text, sf::Color defaultColor = sf::Color(220, 220, 220)) const {
        std::vector<sf::Color> colors(text.size(), defaultColor);
        if (text.empty()) return colors;

        std::vector<bool> colored(text.size(), false);

        for (const auto& rule : m_rules) {
            auto words_begin = std::sregex_iterator(text.begin(), text.end(), rule.regex);
            auto words_end = std::sregex_iterator();

            for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
                std::smatch match = *i;
                size_t startPos = match.position();
                size_t length = match.length();

                for (size_t j = 0; j < length; ++j) {
                    size_t charIdx = startPos + j;
                    if (!colored[charIdx]) {
                        colors[charIdx] = rule.color;
                        colored[charIdx] = true;
                    }
                }
            }
        }
        return colors;
    }

private:
    std::vector<SyntaxRule> m_rules;

    // Вспомогательный метод парсинга HEX-строк (поддерживает #RRGGBB, #RRGGBBAA, RRGGBB, RRGGBBAA)
    bool parseHexColor(std::string hex, sf::Color& outColor) {
        if (hex.empty()) return false;

        // Удаляем решетку, если она есть
        if (hex[0] == '#') {
            hex.erase(0, 1);
        }

        if (hex.size() != 6 && hex.size() != 8) {
            return false;
        }

        // Проверяем, что все символы являются валидными шестнадцатеричными цифрами
        for (char c : hex) {
            if (!std::isxdigit(static_cast<unsigned char>(c))) return false;
        }

        unsigned int hexValue = 0;
        std::stringstream ss;
        ss << std::hex << hex;
        ss >> hexValue;

        if (hex.size() == 6) {
            outColor.r = static_cast<uint8_t>((hexValue >> 16) & 0xFF);
            outColor.g = static_cast<uint8_t>((hexValue >> 8) & 0xFF);
            outColor.b = static_cast<uint8_t>(hexValue & 0xFF);
            outColor.a = 255; // Полноценная непрозрачность по умолчанию
        }
        else if (hex.size() == 8) {
            outColor.r = static_cast<uint8_t>((hexValue >> 24) & 0xFF);
            outColor.g = static_cast<uint8_t>((hexValue >> 16) & 0xFF);
            outColor.b = static_cast<uint8_t>((hexValue >> 8) & 0xFF);
            outColor.a = static_cast<uint8_t>(hexValue & 0xFF);
        }

        return true;
    }
};
