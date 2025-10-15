#pragma once

#include <algorithm>
#include <iostream>
#include <map>

#include "../common/Student.h"

class StudentMerger {
 public:
  // Объединяет списки студентов, объединяя дубликаты по ФИО и дате рождения
  static std::vector<Student> mergeStudents(const std::vector<Student>& list1,
                                            const std::vector<Student>& list2) {
    std::cout << "\n🔄 Объединение списков студентов..." << std::endl;
    std::cout << "   Список 1: " << list1.size() << " записей" << std::endl;
    std::cout << "   Список 2: " << list2.size() << " записей" << std::endl;

    // Используем map для группировки по уникальному ключу
    std::map<std::string, Student> studentMap;

    // Добавляем студентов из первого списка
    for (const auto& student : list1) {
      addStudentToMap(studentMap, student);
    }

    // Добавляем студентов из второго списка
    for (const auto& student : list2) {
      addStudentToMap(studentMap, student);
    }

    // Преобразуем map обратно в vector
    std::vector<Student> mergedList;
    mergedList.reserve(studentMap.size());

    for (auto& pair : studentMap) {
      mergedList.push_back(std::move(pair.second));
    }

    int duplicatesFound = (list1.size() + list2.size()) - mergedList.size();

    std::cout << "✅ Объединение завершено:" << std::endl;
    std::cout << "   Уникальных студентов: " << mergedList.size() << std::endl;
    if (duplicatesFound > 0) {
      std::cout << "   Найдено дубликатов: " << duplicatesFound << std::endl;
    }

    return mergedList;
  }

 private:
  static void addStudentToMap(std::map<std::string, Student>& studentMap,
                              const Student& student) {
    std::string key = student.getUniqueKey();

    auto it = studentMap.find(key);
    if (it != studentMap.end()) {
      // Студент уже существует - добавляем ID
      for (int id : student.ids) {
        // Проверяем, что ID еще не добавлен
        if (std::find(it->second.ids.begin(), it->second.ids.end(), id) ==
            it->second.ids.end()) {
          it->second.ids.push_back(id);
        }
      }
      std::cout << "   🔗 Дубликат найден: " << student.getFullName()
                << " (ID объединены)" << std::endl;
    } else {
      // Новый студент
      studentMap[key] = student;
    }
  }
};