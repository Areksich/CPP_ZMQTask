#pragma once

#include <optional>
#include <string>
#include <vector>

// Структура для десятичных координат
struct DecimalCoordinate {
  double latitude;
  double longitude;
  bool valid;

  DecimalCoordinate() : latitude(0), longitude(0), valid(false) {}
  DecimalCoordinate(double lat, double lon)
      : latitude(lat), longitude(lon), valid(true) {}
};

// Структура для извлеченной координаты
struct ExtractedCoordinate {
  DecimalCoordinate coord;
  std::string original_format;
  std::string context;
  std::string name;
  size_t position;

  ExtractedCoordinate() : position(0) {}
};

// Типы геометрии
enum class GeometryType {
  POINTS,   // Одиночные точки
  LINE,     // Линия
  POLYGON,  // Замкнутый полигон
  UNKNOWN
};

// Результат анализа
struct ExtractionResult {
  std::vector<ExtractedCoordinate> coordinates;
  GeometryType geometry_type;
  std::string message;

  ExtractionResult() : geometry_type(GeometryType::UNKNOWN) {}
};