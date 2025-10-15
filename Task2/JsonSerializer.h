#pragma once

#include <iomanip>
#include <sstream>

#include "GeometryAnalyzer.h"
#include "Structs.h"

class JsonSerializer {
 private:
  // Экранирование строки для JSON
  static std::string escapeJson(const std::string& str) {
    std::ostringstream oss;
    for (size_t i = 0; i < str.length(); i++) {
      unsigned char c = static_cast<unsigned char>(str[i]);

      switch (c) {
        case '"':
          oss << "\\\"";
          break;
        case '\\':
          oss << "\\\\";
          break;
        case '\b':
          oss << "\\b";
          break;
        case '\f':
          oss << "\\f";
          break;
        case '\n':
          oss << "\\n";
          break;
        case '\r':
          oss << "\\r";
          break;
        case '\t':
          oss << "\\t";
          break;
        default:
          // Для UTF-8 многобайтовых символов (c >= 0x80) просто копируем как
          // есть
          if (c >= 0x80) {
            oss << static_cast<char>(c);
          }
          // Для управляющих символов ASCII экранируем
          else if (c < 0x20) {
            oss << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                << static_cast<int>(c);
          }
          // Обычные ASCII символы
          else {
            oss << static_cast<char>(c);
          }
      }
    }
    return oss.str();
  }

 public:
  // Сериализация координаты
  static std::string serializeCoordinate(const ExtractedCoordinate& coord) {
    std::ostringstream json;
    json << std::fixed << std::setprecision(6);

    json << "    {\n";
    json << "      \"latitude\": " << coord.coord.latitude << ",\n";
    json << "      \"longitude\": " << coord.coord.longitude << ",\n";
    json << "      \"original_format\": \"" << escapeJson(coord.original_format)
         << "\",\n";
    json << "      \"context\": \"" << escapeJson(coord.context) << "\",\n";
    json << "      \"name\": \"" << escapeJson(coord.name) << "\"\n";
    json << "    }";

    return json.str();
  }

  // Сериализация результата
  static std::string serializeResult(const ExtractionResult& result) {
    std::ostringstream json;

    json << "{\n";
    json << "  \"geometry_type\": \""
         << GeometryAnalyzer::geometryTypeToString(result.geometry_type)
         << "\",\n";
    json << "  \"coordinates_count\": " << result.coordinates.size() << ",\n";
    json << "  \"coordinates\": [\n";

    for (size_t i = 0; i < result.coordinates.size(); i++) {
      json << serializeCoordinate(result.coordinates[i]);
      if (i < result.coordinates.size() - 1) {
        json << ",";
      }
      json << "\n";
    }

    json << "  ]";

    if (!result.message.empty()) {
      json << ",\n  \"message\": \"" << escapeJson(result.message) << "\"";
    }

    json << "\n}\n";

    return json.str();
  }

  // Сериализация ошибки
  static std::string serializeError(const std::string& error) {
    std::ostringstream json;
    json << "{\n";
    json << "  \"error\": \"" << escapeJson(error) << "\"\n";
    json << "}\n";
    return json.str();
  }
};