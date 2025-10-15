#pragma once

#include <algorithm>
#include <iomanip>
#include <iostream>

#include "Student.h"

class StudentSorter {
 public:
  // Сортировка студентов по ФИО
  static void sortByFullName(std::vector<Student>& students) {
    std::cout << "\n🔤 Сортировка студентов по ФИО..." << std::endl;

    std::sort(students.begin(), students.end(),
              [](const Student& a, const Student& b) {
                return a.getFullName() < b.getFullName();
              });

    std::cout << "✅ Сортировка завершена" << std::endl;
  }

  // Вывод списка студентов на экран
  static void displayStudents(const std::vector<Student>& students) {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "📋 СПИСОК СТУДЕНТОВ (" << students.size() << ")" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    if (students.empty()) {
      std::cout << "Список пуст" << std::endl;
    } else {
      std::cout << std::left;
      std::cout << std::setw(15) << "ID" << std::setw(35) << "ФИО"
                << "Дата рождения" << std::endl;
      std::cout << std::string(70, '-') << std::endl;

      for (const auto& student : students) {
        // Форматируем ID
        std::string idsStr;
        for (size_t i = 0; i < student.ids.size(); ++i) {
          idsStr += std::to_string(student.ids[i]);
          if (i < student.ids.size() - 1) {
            idsStr += ", ";
          }
        }

        std::cout << std::setw(15) << idsStr << std::setw(35)
                  << student.getFullName() << student.birthDate.toString()
                  << std::endl;
      }
    }

    std::cout << std::string(70, '=') << std::endl;
  }
};