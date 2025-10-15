#pragma once

#include "Structs.h"

class CoordinateValidator {
 public:
  // Проверка валидности широты (-90 до 90)
  static bool isValidLatitude(double lat) {
    return lat >= -90.0 && lat <= 90.0;
  }

  // Проверка валидности долготы (-180 до 180)
  static bool isValidLongitude(double lon) {
    return lon >= -180.0 && lon <= 180.0;
  }

  // Проверка координаты
  static bool validateCoordinate(const DecimalCoordinate& coord) {
    return coord.valid && isValidLatitude(coord.latitude) &&
           isValidLongitude(coord.longitude);
  }

  // Проверка координаты с выводом ошибки
  static bool validateCoordinate(const DecimalCoordinate& coord,
                                 std::string& error) {
    if (!coord.valid) {
      error = "Invalid coordinate format";
      return false;
    }
    if (!isValidLatitude(coord.latitude)) {
      error =
          "Latitude out of range [-90, 90]: " + std::to_string(coord.latitude);
      return false;
    }
    if (!isValidLongitude(coord.longitude)) {
      error = "Longitude out of range [-180, 180]: " +
              std::to_string(coord.longitude);
      return false;
    }
    return true;
  }
};