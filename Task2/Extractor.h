#pragma once

#include <algorithm>
#include <regex>
#include <set>

#include "Parser.h"
#include "Structs.h"
#include "Validator.h"

class CoordinateExtractor {
 private:
  // Извлечение контекста (до 200 символов)
  static std::string extractContext(const std::string& text, size_t pos,
                                    size_t length) {
    // Найти начало предложения
    size_t start = pos;
    while (start > 0 && text[start] != '.' && text[start] != '\n' &&
           text[start] != '!' && text[start] != '?') {
      start--;
    }
    if (start > 0) start++;  // Пропустить сам разделитель

    // Найти конец предложения
    size_t end = pos + length;
    while (end < text.length() && text[end] != '.' && text[end] != '\n' &&
           text[end] != '!' && text[end] != '?') {
      end++;
    }
    if (end < text.length()) end++;  // Включить разделитель

    // Ограничить до 200 символов
    if (end - start > 200) {
      end = start + 200;
    }

    std::string context = text.substr(start, end - start);
    // Убрать лишние пробелы
    context.erase(0, context.find_first_not_of(" \t\n\r"));
    context.erase(context.find_last_not_of(" \t\n\r") + 1);

    return context;
  }

  // Извлечение названия координаты
  static std::string extractName(const std::string& text, size_t pos) {
    // Ищем паттерны типа "точка X:", "Point Alpha:", "Угол №1"
    std::regex namePatterns[] = {
        std::regex(R"((точка|Point|Угол|Цель)[^\n:]{0,30}:)",
                   std::regex::icase),
        std::regex(R"(([А-Яа-яA-Za-z\s]+мыс)[^\n:]{0,30}:)",
                   std::regex::icase)};

    // Ищем в предыдущих 100 символах
    size_t searchStart = (pos > 100) ? pos - 100 : 0;
    std::string searchText = text.substr(searchStart, pos - searchStart + 50);

    for (const auto& pattern : namePatterns) {
      std::smatch match;
      if (std::regex_search(searchText, match, pattern)) {
        std::string name = match[0].str();
        // Убираем двоеточие
        if (!name.empty() && name.back() == ':') {
          name.pop_back();
        }
        // Убрать лишние пробелы
        name.erase(0, name.find_first_not_of(" \t\n\r"));
        name.erase(name.find_last_not_of(" \t\n\r") + 1);
        return name;
      }
    }

    return "";
  }

 public:
  // Извлечение всех координат из текста
  static std::vector<ExtractedCoordinate> extractCoordinates(
      const std::string& text) {
    std::vector<ExtractedCoordinate> results;
    std::set<size_t> usedPositions;

    size_t pos = 0;
    while (pos < text.length()) {
      // Пробуем все форматы и выбираем ближайшее совпадение
      std::vector<std::function<std::optional<CoordinateParser::ParseResult>(
          const std::string&, size_t)>>
          parsers = {
              CoordinateParser::parseFormat7,  // Самый специфичный
              CoordinateParser::parseFormat6,
              CoordinateParser::parseFormatRussian,
              CoordinateParser::parseFormat5,
              CoordinateParser::parseFormat4,
              CoordinateParser::parseFormat3,
              CoordinateParser::parseFormat13,
              CoordinateParser::parseFormat12,
              CoordinateParser::parseFormat2,
              CoordinateParser::parseFormat1  // Самый общий
          };

      // Находим все возможные совпадения
      std::optional<CoordinateParser::ParseResult> bestResult;
      size_t bestStartPos = text.length();

      for (auto& parser : parsers) {
        auto result = parser(text, pos);
        if (result.has_value()) {
          const auto& parseResult = result.value();

          // Проверяем, что не пересекается с уже найденными
          bool overlaps = false;
          for (size_t checkPos = parseResult.startPos;
               checkPos < parseResult.endPos && checkPos < text.length();
               checkPos++) {
            if (usedPositions.count(checkPos) > 0) {
              overlaps = true;
              break;
            }
          }

          // Выбираем ближайшее к текущей позиции совпадение
          if (!overlaps && parseResult.startPos < bestStartPos) {
            bestResult = parseResult;
            bestStartPos = parseResult.startPos;
          }
        }
      }

      // Если нашли координату, добавляем её
      if (bestResult.has_value()) {
        const auto& parseResult = bestResult.value();

        ExtractedCoordinate extracted;
        extracted.coord = parseResult.coord;
        extracted.position = parseResult.startPos;
        extracted.original_format = text.substr(
            parseResult.startPos, parseResult.endPos - parseResult.startPos);
        extracted.context =
            extractContext(text, parseResult.startPos,
                           parseResult.endPos - parseResult.startPos);
        extracted.name = extractName(text, parseResult.startPos);

        results.push_back(extracted);

        // Отмечаем использованные позиции
        for (size_t i = parseResult.startPos;
             i < parseResult.endPos && i < text.length(); i++) {
          usedPositions.insert(i);
        }

        pos = parseResult.endPos;
      } else {
        pos++;
      }
    }

    return results;
  }
};