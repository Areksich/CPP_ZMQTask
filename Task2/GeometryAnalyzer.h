#pragma once

#include <algorithm>
#include <cmath>

#include "Structs.h"

class GeometryAnalyzer {
 private:
  // Вычисление расстояния между двумя координатами (приблизительно в градусах)
  static double distance(const DecimalCoordinate& c1,
                         const DecimalCoordinate& c2) {
    double dlat = c1.latitude - c2.latitude;
    double dlon = c1.longitude - c2.longitude;
    return std::sqrt(dlat * dlat + dlon * dlon);
  }

  // Проверка, являются ли две координаты практически одинаковыми
  static bool areCoordinatesEqual(const DecimalCoordinate& c1,
                                  const DecimalCoordinate& c2,
                                  double epsilon = 0.0001) {
    return distance(c1, c2) < epsilon;
  }

  // Проверка, лежат ли точки примерно на одной линии
  static bool areCollinear(const std::vector<ExtractedCoordinate>& coords,
                           double tolerance = 0.01) {
    if (coords.size() < 3) return true;

    // Используем первые две точки для определения направления
    double dx = coords[1].coord.longitude - coords[0].coord.longitude;
    double dy = coords[1].coord.latitude - coords[0].coord.latitude;
    double length = std::sqrt(dx * dx + dy * dy);

    if (length < 0.0001) return false;

    // Нормализуем вектор направления
    dx /= length;
    dy /= length;

    // Проверяем остальные точки
    for (size_t i = 2; i < coords.size(); i++) {
      double vx = coords[i].coord.longitude - coords[0].coord.longitude;
      double vy = coords[i].coord.latitude - coords[0].coord.latitude;

      // Вычисляем перпендикулярное расстояние от точки до линии
      double perpDist = std::abs(vx * dy - vy * dx);

      if (perpDist > tolerance) {
        return false;
      }
    }

    return true;
  }

  // Проверка, образуют ли координаты замкнутый полигон
  static bool isClosedPolygon(const std::vector<ExtractedCoordinate>& coords) {
    if (coords.size() < 3) return false;

    // Проверяем, совпадает ли первая и последняя точка
    if (areCoordinatesEqual(coords.front().coord, coords.back().coord)) {
      return true;
    }

    // Проверяем, образуют ли точки замкнутую фигуру (не на одной линии)
    // и расстояние от первой до последней точки мало
    if (coords.size() >= 3 && !areCollinear(coords)) {
      double dist = distance(coords.front().coord, coords.back().coord);
      // Если расстояние от первой до последней точки меньше 10% от общего
      // периметра
      double totalDist = 0;
      for (size_t i = 1; i < coords.size(); i++) {
        totalDist += distance(coords[i - 1].coord, coords[i].coord);
      }

      if (dist < totalDist * 0.2) {
        return true;
      }
    }

    return false;
  }

 public:
  // Определение типа геометрии
  static GeometryType analyzeGeometry(
      const std::vector<ExtractedCoordinate>& coords) {
    if (coords.empty()) {
      return GeometryType::UNKNOWN;
    }

    if (coords.size() == 1) {
      return GeometryType::POINTS;
    }

    if (coords.size() == 2) {
      // Две точки - это всегда линия
      return GeometryType::LINE;
    }

    // Проверяем на полигон
    if (isClosedPolygon(coords)) {
      return GeometryType::POLYGON;
    }

    // Проверяем на линию
    if (areCollinear(coords)) {
      return GeometryType::LINE;
    }

    // Если точки не на одной линии и не образуют замкнутый полигон,
    // проверяем последовательность точек
    // Если точки идут последовательно и образуют паттерн, это может быть
    // полигон или линия
    bool sequential = true;
    for (size_t i = 1; i < coords.size(); i++) {
      if (coords[i].position < coords[i - 1].position) {
        sequential = false;
        break;
      }
    }

    if (sequential && coords.size() >= 3) {
      // Для последовательных точек проверяем контекст
      // Если в контексте есть слова "полигон", "периметр", "замкнутый" - это
      // полигон
      for (const auto& coord : coords) {
        std::string ctx = coord.context;
        std::transform(ctx.begin(), ctx.end(), ctx.begin(), ::tolower);
        if (ctx.find("полигон") != std::string::npos ||
            ctx.find("polygon") != std::string::npos ||
            ctx.find("периметр") != std::string::npos ||
            ctx.find("замкнут") != std::string::npos ||
            ctx.find("угол") != std::string::npos ||
            ctx.find("вершин") != std::string::npos) {
          return GeometryType::POLYGON;
        }
        if (ctx.find("линия") != std::string::npos ||
            ctx.find("line") != std::string::npos ||
            ctx.find("маршрут") != std::string::npos ||
            ctx.find("route") != std::string::npos) {
          return GeometryType::LINE;
        }
      }

      // По умолчанию для 3+ точек - полигон, если не линия
      return GeometryType::POLYGON;
    }

    // Несколько разрозненных точек
    return GeometryType::POINTS;
  }

  // Получение строкового представления типа геометрии
  static std::string geometryTypeToString(GeometryType type) {
    switch (type) {
      case GeometryType::POINTS:
        return "points";
      case GeometryType::LINE:
        return "line";
      case GeometryType::POLYGON:
        return "polygon";
      default:
        return "unknown";
    }
  }
};