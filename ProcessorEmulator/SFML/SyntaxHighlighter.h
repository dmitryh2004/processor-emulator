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
        : regex(pattern, std::regex_constants::optimize | std::regex_constants::icase), color(col) {
    }
};

class SyntaxHighlighter {
public:
    SyntaxHighlighter() = default;

    // Загрузка цветовой схемы из файла конфигурации .conf (остается без изменений, так как файлы в UTF-8)
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

            line.erase(line.begin(), std::find_if(line.begin(), line.end(), [](unsigned char ch) {
                return !std::isspace(ch);
            }));

            if (line.empty() || (line.size() >= 2 && line.compare(0, 2, "//") == 0)) {
                continue;
            }

            size_t spacePos = line.find(' ');
            if (spacePos == std::string::npos || spacePos == 0) {
                std::cerr << "[SyntaxHighlighter] Line " << lineNumber << " missing separator space." << std::endl;
                continue;
            }

            std::string hexStr = line.substr(0, spacePos);
            std::string regexPattern = line.substr(spacePos + 1);

            sf::Color color;
            if (!parseHexColor(hexStr, color)) {
                std::cerr << "[SyntaxHighlighter] Line " << lineNumber << " has invalid HEX color: " << hexStr << std::endl;
                continue;
            }

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

    // НОВАЯ ФУНКЦИЯ: Принимает UTF-32 строку и возвращает вектор цветов под её размер
    std::vector<sf::Color> highlight(const std::u32string& u32text, sf::Color defaultColor = sf::Color(220, 220, 220)) const {
        std::vector<sf::Color> colors(u32text.size(), defaultColor);
        if (u32text.empty()) return colors;

        // 1. Конвертируем UTF-32 в UTF-8 строку для поиска через std::regex
        std::string utf8Text;
        utf8Text.reserve(u32text.size()); // Примерное выделение памяти

        // Массив, связывающий каждый байт UTF-8 строки с индексом символа в UTF-32
        std::vector<size_t> utf8ByteToU32Index;
        utf8ByteToU32Index.reserve(u32text.size() * 2);

        for (size_t i = 0; i < u32text.size(); ++i) {
            char32_t cp = u32text[i];
            size_t bytesCount = 0;

            if (cp <= 0x7F) {
                utf8Text.push_back(static_cast<char>(cp));
                bytesCount = 1;
            }
            else if (cp <= 0x7FF) {
                utf8Text.push_back(static_cast<char>(0xC0 | ((cp >> 6) & 0x1F)));
                utf8Text.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
                bytesCount = 2;
            }
            else if (cp <= 0xFFFF) {
                utf8Text.push_back(static_cast<char>(0xE0 | ((cp >> 12) & 0x0F)));
                utf8Text.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
                utf8Text.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
                bytesCount = 3;
            }
            else {
                utf8Text.push_back(static_cast<char>(0xF0 | ((cp >> 18) & 0x07)));
                utf8Text.push_back(static_cast<char>(0x80 | ((cp >> 12) & 0x3F)));
                utf8Text.push_back(static_cast<char>(0x80 | ((cp >> 6) & 0x3F)));
                utf8Text.push_back(static_cast<char>(0x80 | (cp & 0x3F)));
                bytesCount = 4;
            }

            // Для каждого байта в UTF-8 запоминаем, какому UTF-32 индексу он принадлежит
            for (size_t b = 0; b < bytesCount; ++b) {
                utf8ByteToU32Index.push_back(i);
            }
        }

        // 2. Поиск по регулярным выражениям в UTF-8 строке
        std::vector<bool> colored(u32text.size(), false);

        for (const auto& rule : m_rules) {
            auto words_begin = std::sregex_iterator(utf8Text.begin(), utf8Text.end(), rule.regex);
            auto words_end = std::sregex_iterator();

            for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
                std::smatch match = *i;
                size_t startBytePos = match.position();
                size_t byteLength = match.length();

                if (byteLength == 0) continue;

                // Переводим байтовые позиции UTF-8 в символьные позиции UTF-32
                size_t startU32Idx = utf8ByteToU32Index[startBytePos];
                // Индекс конца — это индекс символа, которому принадлежит последний байт совпадения
                size_t endU32Idx = utf8ByteToU32Index[startBytePos + byteLength - 1];

                for (size_t charIdx = startU32Idx; charIdx <= endU32Idx; ++charIdx) {
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

    bool parseHexColor(std::string hex, sf::Color& outColor) {
        if (hex.empty()) return false;
        if (hex[0] == '#') hex.erase(0, 1);
        if (hex.size() != 6 && hex.size() != 8) return false;

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
            outColor.a = 255;
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
