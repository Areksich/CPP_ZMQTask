#pragma once

#include <algorithm>
#include <cctype>
#include <cmath>
#include <regex>

#include "Structs.h"
#include "Validator.h"

class CoordinateParser {
 public:
  // Структура для возврата результата парсинга
  struct ParseResult {
    DecimalCoordinate coord;
    size_t startPos;  // Начало совпадения в оригинальном тексте
    size_t endPos;  // Конец совпадения в оригинальном тексте
  };

  // Конвертация DMS в десятичные градусы
  static double dmsToDecimal(double degrees, double minutes, double seconds,
                             bool isNegative) {
    double decimal = degrees + minutes / 60.0 + seconds / 3600.0;
    return isNegative ? -decimal : decimal;
  }

  // Определение знака на основе направления
  static bool isNegativeDirection(const std::string& dir) {
    if (dir.empty()) return false;
    char c = std::toupper(dir[0]);
    // Проверка латинских символов
    if (c == 'S' || c == 'W') return true;
    // Проверка русских символов (многобайтовые)
    if (dir.find("Ю") == 0 || dir.find("ю") == 0 || dir.find("З") == 0 ||
        dir.find("з") == 0)
      return true;
    return false;
  }

  // Парсинг формата 1: 12.2112 -32.434 или 12.2112, -32.434
  static std::optional<ParseResult> parseFormat1(const std::string& text,
                                                 size_t pos) {
    // Поддержка как пробелов, так и запятых между координатами
    std::regex pattern(R"((-?\d+\.?\d*)[,\s]+(-?\d+\.?\d*))");
    std::smatch match;
    std::string substr = text.substr(pos);

    if (std::regex_search(substr, match, pattern)) {
      try {
        double lat = std::stod(match[1].str());
        double lon = std::stod(match[2].str());
        DecimalCoordinate coord(lat, lon);

        if (CoordinateValidator::validateCoordinate(coord)) {
          ParseResult result;
          result.coord = coord;
          result.startPos = pos + match.position();
          result.endPos = result.startPos + match.length();
          return result;
        }
      } catch (...) {
      }
    }
    return std::nullopt;
  }

  // Парсинг формата 2: N12.2112 W32.434
  static std::optional<ParseResult> parseFormat2(const std::string& text,
                                                 size_t pos) {
    std::regex pattern(R"([NSnsюсЮС](\d+\.?\d*)\s+[EWewвзВЗ](\d+\.?\d*))");
    std::smatch match;
    std::string substr = text.substr(pos);

    if (std::regex_search(substr, match, pattern)) {
      try {
        double lat = std::stod(match[1].str());
        double lon = std::stod(match[2].str());

        std::string latDir =
            match[0].str().substr(0, match[0].str().find(match[1].str()));
        std::string lonDir =
            match[0].str().substr(match[1].str().length() + latDir.length());
        lonDir = lonDir.substr(0, lonDir.find(match[2].str()));

        bool latNeg = isNegativeDirection(latDir);
        bool lonNeg = isNegativeDirection(lonDir);

        if (latNeg) lat = -lat;
        if (lonNeg) lon = -lon;

        DecimalCoordinate coord(lat, lon);
        if (CoordinateValidator::validateCoordinate(coord)) {
          ParseResult result;
          result.coord = coord;
          result.startPos = pos + match.position();
          result.endPos = result.startPos + match.length();
          return result;
        }
      } catch (...) {
      }
    }
    return std::nullopt;
  }

  // Парсинг формата 3: 34-24N 124-49W
  static std::optional<ParseResult> parseFormat3(const std::string& text,
                                                 size_t pos) {
    std::regex pattern(R"((\d+)-(\d+)([NSnsюсЮС])\s+(\d+)-(\d+)([EWewвзВЗ]))");
    std::smatch match;
    std::string substr = text.substr(pos);

    if (std::regex_search(substr, match, pattern)) {
      try {
        double latDeg = std::stod(match[1].str());
        double latMin = std::stod(match[2].str());
        double lonDeg = std::stod(match[4].str());
        double lonMin = std::stod(match[5].str());

        bool latNeg = isNegativeDirection(match[3].str());
        bool lonNeg = isNegativeDirection(match[6].str());

        double lat = dmsToDecimal(latDeg, latMin, 0, latNeg);
        double lon = dmsToDecimal(lonDeg, lonMin, 0, lonNeg);

        DecimalCoordinate coord(lat, lon);
        if (CoordinateValidator::validateCoordinate(coord)) {
          ParseResult result;
          result.coord = coord;
          result.startPos = pos + match.position();
          result.endPos = result.startPos + match.length();
          return result;
        }
      } catch (...) {
      }
    }
    return std::nullopt;
  }

  // Парсинг формата 4: 5401N 15531W (DDMMH DDDMMH)
  static std::optional<ParseResult> parseFormat4(const std::string& text,
                                                 size_t pos) {
    std::regex pattern(R"((\d{2,4})([NSnsюсЮС])\s+(\d{3,5})([EWewвзВЗ]))");
    std::smatch match;
    std::string substr = text.substr(pos);

    if (std::regex_search(substr, match, pattern)) {
      try {
        std::string latStr = match[1].str();
        std::string lonStr = match[3].str();

        double latDeg, latMin;
        if (latStr.length() == 4) {
          latDeg = std::stod(latStr.substr(0, 2));
          latMin = std::stod(latStr.substr(2, 2));
        } else if (latStr.length() == 2) {
          latDeg = std::stod(latStr);
          latMin = 0;
        } else {
          return std::nullopt;
        }

        double lonDeg, lonMin;
        if (lonStr.length() == 5) {
          lonDeg = std::stod(lonStr.substr(0, 3));
          lonMin = std::stod(lonStr.substr(3, 2));
        } else if (lonStr.length() == 3 || lonStr.length() == 2) {
          lonDeg = std::stod(lonStr);
          lonMin = 0;
        } else {
          return std::nullopt;
        }

        bool latNeg = isNegativeDirection(match[2].str());
        bool lonNeg = isNegativeDirection(match[4].str());

        double lat = dmsToDecimal(latDeg, latMin, 0, latNeg);
        double lon = dmsToDecimal(lonDeg, lonMin, 0, lonNeg);

        DecimalCoordinate coord(lat, lon);
        if (CoordinateValidator::validateCoordinate(coord)) {
          ParseResult result;
          result.coord = coord;
          result.startPos = pos + match.position();
          result.endPos = result.startPos + match.length();
          return result;
        }
      } catch (...) {
      }
    }
    return std::nullopt;
  }

  // Парсинг формата 5: N405229 E087182 (HDDDMMSS)
  static std::optional<ParseResult> parseFormat5(const std::string& text,
                                                 size_t pos) {
    std::regex pattern(R"([NSnsюсЮС](\d{2,6})\s+[EWewвзВЗ](\d{3,7}))");
    std::smatch match;
    std::string substr = text.substr(pos);

    if (std::regex_search(substr, match, pattern)) {
      try {
        std::string latStr = match[1].str();
        std::string lonStr = match[2].str();

        double latDeg, latMin, latSec;
        if (latStr.length() == 6) {
          latDeg = std::stod(latStr.substr(0, 2));
          latMin = std::stod(latStr.substr(2, 2));
          latSec = std::stod(latStr.substr(4, 2));
        } else if (latStr.length() == 4) {
          latDeg = std::stod(latStr.substr(0, 2));
          latMin = std::stod(latStr.substr(2, 2));
          latSec = 0;
        } else if (latStr.length() == 2) {
          latDeg = std::stod(latStr);
          latMin = 0;
          latSec = 0;
        } else {
          return std::nullopt;
        }

        double lonDeg, lonMin, lonSec;
        if (lonStr.length() == 7) {
          lonDeg = std::stod(lonStr.substr(0, 3));
          lonMin = std::stod(lonStr.substr(3, 2));
          lonSec = std::stod(lonStr.substr(5, 2));
        } else if (lonStr.length() == 5) {
          lonDeg = std::stod(lonStr.substr(0, 3));
          lonMin = std::stod(lonStr.substr(3, 2));
          lonSec = 0;
        } else if (lonStr.length() == 3) {
          lonDeg = std::stod(lonStr);
          lonMin = 0;
          lonSec = 0;
        } else {
          return std::nullopt;
        }

        std::string fullMatch = match[0].str();
        std::string latDir = fullMatch.substr(0, fullMatch.find(latStr));
        std::string remaining =
            fullMatch.substr(latDir.length() + latStr.length());
        std::string lonDir = remaining.substr(0, remaining.find(lonStr));

        bool latNeg = isNegativeDirection(latDir);
        bool lonNeg = isNegativeDirection(lonDir);

        double lat = dmsToDecimal(latDeg, latMin, latSec, latNeg);
        double lon = dmsToDecimal(lonDeg, lonMin, lonSec, lonNeg);

        DecimalCoordinate coord(lat, lon);
        if (CoordinateValidator::validateCoordinate(coord)) {
          ParseResult result;
          result.coord = coord;
          result.startPos = pos + match.position();
          result.endPos = result.startPos + match.length();
          return result;
        }
      } catch (...) {
      }
    }
    return std::nullopt;
  }

  // Парсинг формата 6: 51°12.32'S 32°34.43'E (градусы и десятичные минуты)
  static std::optional<ParseResult> parseFormat6(const std::string& text,
                                                 size_t pos) {
    std::regex pattern(
        R"((\d+)°(\d+\.?\d*)'([NSnsюсЮС])\s+(\d+)°(\d+\.?\d*)'([EWewвзВЗ]))");
    std::smatch match;
    std::string substr = text.substr(pos);

    if (std::regex_search(substr, match, pattern)) {
      try {
        double latDeg = std::stod(match[1].str());
        double latMin = std::stod(match[2].str());
        double lonDeg = std::stod(match[4].str());
        double lonMin = std::stod(match[5].str());

        bool latNeg = isNegativeDirection(match[3].str());
        bool lonNeg = isNegativeDirection(match[6].str());

        double lat = dmsToDecimal(latDeg, latMin, 0, latNeg);
        double lon = dmsToDecimal(lonDeg, lonMin, 0, lonNeg);

        DecimalCoordinate coord(lat, lon);
        if (CoordinateValidator::validateCoordinate(coord)) {
          ParseResult result;
          result.coord = coord;
          result.startPos = pos + match.position();
          result.endPos = result.startPos + match.length();
          return result;
        }
      } catch (...) {
      }
    }
    return std::nullopt;
  }

  // Парсинг формата 7: 51°12'32.212"S 32°34'23.1232"E (полный DMS)
  static std::optional<ParseResult> parseFormat7(const std::string& text,
                                                 size_t pos) {
    std::regex pattern(
        R"((\d+)°(\d+)'(\d+\.?\d*)''?([NSnsюсЮС])\s+(\d+)°(\d+)'(\d+\.?\d*)''?([EWewвзВЗ]))");
    std::smatch match;
    std::string substr = text.substr(pos);

    if (std::regex_search(substr, match, pattern)) {
      try {
        double latDeg = std::stod(match[1].str());
        double latMin = std::stod(match[2].str());
        double latSec = std::stod(match[3].str());
        double lonDeg = std::stod(match[5].str());
        double lonMin = std::stod(match[6].str());
        double lonSec = std::stod(match[7].str());

        bool latNeg = isNegativeDirection(match[4].str());
        bool lonNeg = isNegativeDirection(match[8].str());

        double lat = dmsToDecimal(latDeg, latMin, latSec, latNeg);
        double lon = dmsToDecimal(lonDeg, lonMin, lonSec, lonNeg);

        DecimalCoordinate coord(lat, lon);
        if (CoordinateValidator::validateCoordinate(coord)) {
          ParseResult result;
          result.coord = coord;
          result.startPos = pos + match.position();
          result.endPos = result.startPos + match.length();
          return result;
        }
      } catch (...) {
      }
    }
    return std::nullopt;
  }

  // Парсинг формата 8 и 9: с русскими обозначениями (с.ш., в.д.)
  static std::optional<ParseResult> parseFormatRussian(const std::string& text,
                                                       size_t pos) {
    // Формат 8: 51°12.32' с.ш. 32°34.43' в.д.
    std::regex pattern8(
        R"((\d+)°(\d+\.?\d*)'\s*([сюСЮ]\.?ш\.?|[СЮ])\s+(\d+)°(\d+\.?\d*)'\s*([взВЗ]\.?д\.?|[ВЗ]))");
    // Формат 9: 51°12'32.212" с.ш 32°34'23.1232 в.д.
    std::regex pattern9(
        R"((\d+)°(\d+)'(\d+\.?\d*)''?\s*([сюСЮ]\.?ш\.?|[СЮ])\s+(\d+)°(\d+)'(\d+\.?\d*)''?\s*([взВЗ]\.?д\.?|[ВЗ]))");

    std::smatch match;
    std::string substr = text.substr(pos);

    if (std::regex_search(substr, match, pattern9)) {
      try {
        double latDeg = std::stod(match[1].str());
        double latMin = std::stod(match[2].str());
        double latSec = std::stod(match[3].str());
        double lonDeg = std::stod(match[5].str());
        double lonMin = std::stod(match[6].str());
        double lonSec = std::stod(match[7].str());

        bool latNeg = isNegativeDirection(match[4].str());
        bool lonNeg = isNegativeDirection(match[8].str());

        double lat = dmsToDecimal(latDeg, latMin, latSec, latNeg);
        double lon = dmsToDecimal(lonDeg, lonMin, lonSec, lonNeg);

        DecimalCoordinate coord(lat, lon);
        if (CoordinateValidator::validateCoordinate(coord)) {
          ParseResult result;
          result.coord = coord;
          result.startPos = pos + match.position();
          result.endPos = result.startPos + match.length();
          return result;
        }
      } catch (...) {
      }
    } else if (std::regex_search(substr, match, pattern8)) {
      try {
        double latDeg = std::stod(match[1].str());
        double latMin = std::stod(match[2].str());
        double lonDeg = std::stod(match[4].str());
        double lonMin = std::stod(match[5].str());

        bool latNeg = isNegativeDirection(match[3].str());
        bool lonNeg = isNegativeDirection(match[6].str());

        double lat = dmsToDecimal(latDeg, latMin, 0, latNeg);
        double lon = dmsToDecimal(lonDeg, lonMin, 0, lonNeg);

        DecimalCoordinate coord(lat, lon);
        if (CoordinateValidator::validateCoordinate(coord)) {
          ParseResult result;
          result.coord = coord;
          result.startPos = pos + match.position();
          result.endPos = result.startPos + match.length();
          return result;
        }
      } catch (...) {
      }
    }
    return std::nullopt;
  }

  // Парсинг формата 12: 55,755831°, 37,617673° или 55.755831°, 37.617673°
  static std::optional<ParseResult> parseFormat12(const std::string& text,
                                                  size_t pos) {
    // Требуем наличие символа градуса для этого формата
    std::regex pattern(R"((\d+[,\.]?\d*)°[,\s]+(\d+[,\.]?\d*)°)");
    std::smatch match;
    std::string substr = text.substr(pos);

    if (std::regex_search(substr, match, pattern)) {
      try {
        std::string latStr = match[1].str();
        std::string lonStr = match[2].str();
        std::replace(latStr.begin(), latStr.end(), ',', '.');
        std::replace(lonStr.begin(), lonStr.end(), ',', '.');

        double lat = std::stod(latStr);
        double lon = std::stod(lonStr);

        DecimalCoordinate coord(lat, lon);
        if (CoordinateValidator::validateCoordinate(coord)) {
          ParseResult result;
          result.coord = coord;
          result.startPos = pos + match.position();
          result.endPos = result.startPos + match.length();
          return result;
        }
      } catch (...) {
      }
    }
    return std::nullopt;
  }

  // Парсинг формата 13: N55.755831°, E37.617673°
  static std::optional<ParseResult> parseFormat13(const std::string& text,
                                                  size_t pos) {
    std::regex pattern(
        R"([NSnsюсЮС](\d+[,\.]?\d*)°?[,\s]+[EWewвзВЗ](\d+[,\.]?\d*)°?)");
    std::smatch match;
    std::string substr = text.substr(pos);

    if (std::regex_search(substr, match, pattern)) {
      try {
        std::string latStr = match[1].str();
        std::string lonStr = match[2].str();
        std::replace(latStr.begin(), latStr.end(), ',', '.');
        std::replace(lonStr.begin(), lonStr.end(), ',', '.');

        double lat = std::stod(latStr);
        double lon = std::stod(lonStr);

        std::string fullMatch = match[0].str();
        std::string latDir = fullMatch.substr(0, fullMatch.find(latStr));
        std::string remaining =
            fullMatch.substr(latDir.length() + latStr.length());
        // Находим следующую букву направления после запятой
        size_t lonDirPos = 0;
        for (size_t i = 0; i < remaining.length(); i++) {
          char c = remaining[i];
          if (std::isalpha(c) || (unsigned char)c > 127) {
            lonDirPos = i;
            break;
          }
        }
        std::string lonDir =
            remaining.substr(lonDirPos, remaining.find(lonStr) - lonDirPos);

        bool latNeg = isNegativeDirection(latDir);
        bool lonNeg = isNegativeDirection(lonDir);

        if (latNeg) lat = -lat;
        if (lonNeg) lon = -lon;

        DecimalCoordinate coord(lat, lon);
        if (CoordinateValidator::validateCoordinate(coord)) {
          ParseResult result;
          result.coord = coord;
          result.startPos = pos + match.position();
          result.endPos = result.startPos + match.length();
          return result;
        }
      } catch (...) {
      }
    }
    return std::nullopt;
  }
};