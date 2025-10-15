#pragma once

#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#include "../common/Validator.h"
#include "Student.h"

class FileParser {
 public:
  // Парсинг файла со студентами
  static std::vector<Student> parseFile(const std::string& filename) {
    std::vector<Student> students;
    std::ifstream file(filename);

    if (!file.is_open()) {
      std::cerr << "❌ Ошибка: Не удалось открыть файл: " << filename
                << std::endl;
      return students;
    }

    std::cout << "📂 Чтение файла: " << filename << std::endl;

    std::string line;
    int lineNumber = 0;
    int validCount = 0;
    int invalidCount = 0;

    while (std::getline(file, line)) {
      lineNumber++;

      // Пропускаем пустые строки
      if (line.empty() ||
          line.find_first_not_of(" \t\r\n") == std::string::npos) {
        continue;
      }

      Student student;
      if (parseLine(line, student)) {
        if (Validator::validateStudent(student)) {
          students.push_back(student);
          validCount++;
        } else {
          std::cerr << "   Строка " << lineNumber
                    << " отклонена из-за ошибок валидации" << std::endl;
          invalidCount++;
        }
      } else {
        std::cerr << "   Строка " << lineNumber << ": Ошибка парсинга: " << line
                  << std::endl;
        invalidCount++;
      }
    }

    file.close();

    std::cout << "✅ Обработано: " << validCount << " корректных записей"
              << std::endl;
    if (invalidCount > 0) {
      std::cout << "⚠️  Отклонено: " << invalidCount << " некорректных записей"
                << std::endl;
    }

    return students;
  }

 private:
  // Парсинг одной строки: ID FirstName MiddleName LastName DD.MM.YYYY
  static bool parseLine(const std::string& line, Student& student) {
    std::istringstream iss(line);
    int id;
    std::string firstName, middleName, lastName, dateStr;

    // Читаем данные
    if (!(iss >> id >> firstName >> middleName >> lastName >> dateStr)) {
      return false;
    }

    // Проверяем, что в строке нет лишних данных
    std::string extra;
    if (iss >> extra) {
      std::cerr << "⚠️  Предупреждение: Лишние данные в строке: " << extra
                << std::endl;
    }

    // Заполняем структуру
    student.ids.push_back(id);
    student.firstName = firstName;
    student.middleName = middleName;
    student.lastName = lastName;

    // Парсим дату
    return Validator::parseDate(dateStr, student.birthDate);
  }
};