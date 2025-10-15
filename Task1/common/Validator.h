#pragma once

#include <iostream>
#include <regex>

#include "Student.h"

class Validator {
 public:
  // Валидация даты
  static bool isValidDate(int day, int month, int year) {
    if (year < 1900 || year > 2010) {
      std::cerr << "⚠️  Ошибка: Год должен быть между 1900 и 2010 (получено: "
                << year << ")" << std::endl;
      return false;
    }

    if (month < 1 || month > 12) {
      std::cerr << "⚠️  Ошибка: Месяц должен быть между 1 и 12 (получено: "
                << month << ")" << std::endl;
      return false;
    }

    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};

    // Проверка високосного года
    if (month == 2 && isLeapYear(year)) {
      daysInMonth[1] = 29;
    }

    if (day < 1 || day > daysInMonth[month - 1]) {
      std::cerr << "⚠️  Ошибка: День " << day << " некорректен для месяца "
                << month << std::endl;
      return false;
    }

    return true;
  }

  // Валидация имени (только буквы)
  static bool isValidName(const std::string& name) {
    if (name.empty()) {
      std::cerr << "⚠️  Ошибка: Имя не может быть пустым" << std::endl;
      return false;
    }

    // Проверка на наличие только букв (латиница и кириллица)
    std::regex namePattern("^[A-Za-zА-Яа-яЁё]+$");
    if (!std::regex_match(name, namePattern)) {
      std::cerr << "⚠️  Ошибка: Имя содержит недопустимые символы: " << name
                << std::endl;
      return false;
    }

    return true;
  }

  // Валидация ID
  static bool isValidId(int id) {
    if (id <= 0) {
      std::cerr << "⚠️  Ошибка: ID должен быть положительным числом (получено: "
                << id << ")" << std::endl;
      return false;
    }
    return true;
  }

  // Парсинг даты из строки формата DD.MM.YYYY
  static bool parseDate(const std::string& dateStr, Date& date) {
    std::regex datePattern(R"((\d{1,2})\.(\d{1,2})\.(\d{4}))");
    std::smatch match;

    if (!std::regex_match(dateStr, match, datePattern)) {
      std::cerr << "⚠️  Ошибка: Неверный формат даты: " << dateStr
                << " (ожидается DD.MM.YYYY)" << std::endl;
      return false;
    }

    date.day = std::stoi(match[1]);
    date.month = std::stoi(match[2]);
    date.year = std::stoi(match[3]);

    return isValidDate(date.day, date.month, date.year);
  }

  // Валидация всего студента
  static bool validateStudent(const Student& student) {
    if (!isValidId(student.ids[0])) return false;
    if (!isValidName(student.firstName)) return false;
    if (!isValidName(student.middleName)) return false;
    if (!isValidName(student.lastName)) return false;

    return isValidDate(student.birthDate.day, student.birthDate.month,
                       student.birthDate.year);
  }

 private:
  static bool isLeapYear(int year) {
    return (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
  }
};