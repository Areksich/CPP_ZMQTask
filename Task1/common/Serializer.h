#pragma once

#include <iostream>
#include <sstream>
#include <vector>

#include "Student.h"
#include "Validator.h"

class Serializer {
 public:
  // Сериализация списка студентов в строку
  // Формат: каждый студент на отдельной строке
  // ID1,ID2,ID3|FirstName|MiddleName|LastName|DD.MM.YYYY
  static std::string serialize(const std::vector<Student>& students) {
    std::ostringstream oss;

    for (const auto& student : students) {
      // Сериализуем ID
      for (size_t i = 0; i < student.ids.size(); ++i) {
        oss << student.ids[i];
        if (i < student.ids.size() - 1) {
          oss << ",";
        }
      }
      oss << "|";

      // Сериализуем имя
      oss << student.firstName << "|" << student.middleName << "|"
          << student.lastName << "|";

      // Сериализуем дату
      oss << student.birthDate.toString();

      oss << "\n";
    }

    return oss.str();
  }

  // Десериализация строки в список студентов
  static std::vector<Student> deserialize(const std::string& data) {
    std::vector<Student> students;
    std::istringstream iss(data);
    std::string line;
    int lineNumber = 0;

    while (std::getline(iss, line)) {
      lineNumber++;
      if (line.empty()) continue;

      Student student;
      if (deserializeLine(line, student)) {
        if (Validator::validateStudent(student)) {
          students.push_back(student);
        } else {
          std::cerr << "⚠️  Десериализация: строка " << lineNumber
                    << " не прошла валидацию" << std::endl;
        }
      } else {
        std::cerr << "⚠️  Десериализация: ошибка парсинга строки " << lineNumber
                  << std::endl;
      }
    }

    return students;
  }

 private:
  static bool deserializeLine(const std::string& line, Student& student) {
    std::istringstream iss(line);
    std::string token;
    std::vector<std::string> tokens;

    // Разделяем по '|'
    while (std::getline(iss, token, '|')) {
      tokens.push_back(token);
    }

    if (tokens.size() != 5) {
      return false;
    }

    // Парсим ID (формат: ID1,ID2,ID3)
    std::istringstream idStream(tokens[0]);
    std::string idStr;
    while (std::getline(idStream, idStr, ',')) {
      try {
        int id = std::stoi(idStr);
        student.ids.push_back(id);
      } catch (...) {
        return false;
      }
    }

    if (student.ids.empty()) {
      return false;
    }

    // Парсим имя
    student.firstName = tokens[1];
    student.middleName = tokens[2];
    student.lastName = tokens[3];

    // Парсим дату
    return Validator::parseDate(tokens[4], student.birthDate);
  }
};