#pragma once

#include <ctime>
#include <string>
#include <vector>

struct Date {
  int day;
  int month;
  int year;

  bool operator==(const Date& other) const {
    return day == other.day && month == other.month && year == other.year;
  }

  bool operator<(const Date& other) const {
    if (year != other.year) return year < other.year;
    if (month != other.month) return month < other.month;
    return day < other.day;
  }

  std::string toString() const {
    char buffer[11];
    snprintf(buffer, sizeof(buffer), "%02d.%02d.%04d", day, month, year);
    return std::string(buffer);
  }
};

struct Student {
  std::vector<int> ids;  // Может быть несколько ID для одного студента
  std::string firstName;
  std::string middleName;
  std::string lastName;
  Date birthDate;

  // Полное имя для сравнения и сортировки
  std::string getFullName() const {
    return firstName + " " + middleName + " " + lastName;
  }

  // Уникальный ключ для объединения
  std::string getUniqueKey() const {
    return getFullName() + "_" + birthDate.toString();
  }

  bool operator<(const Student& other) const {
    return getFullName() < other.getFullName();
  }

  std::string toString() const {
    std::string idsStr = "[";
    for (size_t i = 0; i < ids.size(); ++i) {
      idsStr += std::to_string(ids[i]);
      if (i < ids.size() - 1) idsStr += ",";
    }
    idsStr += "]";

    return idsStr + " " + getFullName() + " " + birthDate.toString();
  }
};