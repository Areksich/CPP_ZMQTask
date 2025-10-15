#include <iomanip>
#include <iostream>

#include "../common/Student.h"
#include "FileParser.h"
#include "StudentMerger.h"
#include "ZmqSyncedPublisher.h"

int main() {
  std::cout << "╔════════════════════════════════════════════════════╗"
            << std::endl;
  std::cout << "║          СЕРВЕР УПРАВЛЕНИЯ СТУДЕНТАМИ              ║"
            << std::endl;
  std::cout << "╚════════════════════════════════════════════════════╝"
            << std::endl;
  std::cout << std::endl;

  // Шаг 1: Чтение файлов
  std::cout << "📖 ШАГ 1: Чтение файлов со студентами\n" << std::endl;

  auto students1 = FileParser::parseFile("student_file_1.txt");
  auto students2 = FileParser::parseFile("student_file_2.txt");

  if (students1.empty() && students2.empty()) {
    std::cerr << "\n❌ Ошибка: Не удалось загрузить данные студентов"
              << std::endl;
    return 1;
  }

  // Шаг 2: Объединение студентов
  std::cout << "\n📖 ШАГ 2: Объединение списков студентов" << std::endl;
  auto mergedStudents = StudentMerger::mergeStudents(students1, students2);

  if (mergedStudents.empty()) {
    std::cerr << "\n❌ Ошибка: Нет студентов для отправки" << std::endl;
    return 1;
  }

  // Вывод объединенного списка для проверки
  std::cout << "\n📋 Объединенный список студентов:" << std::endl;
  std::cout << std::string(70, '-') << std::endl;
  for (const auto& student : mergedStudents) {
    std::cout << student.toString() << std::endl;
  }
  std::cout << std::string(70, '-') << std::endl;

  // Шаг 3: Публикация через ZeroMQ
  std::cout << "\n📖 ШАГ 3: Публикация данных через ZeroMQ\n" << std::endl;

  ZmqSyncedPublisher publisher("tcp://*:5555", "tcp://*:5556");

  // Запускаем публикацию в отдельном потоке, ожидаем 2-х клиентов
  publisher.start(mergedStudents, 2);

  // Ждем завершения публикации
  std::cout << "\n⏳ Ожидание завершения публикации..." << std::endl;
  std::cout << "   (Нажмите Enter для завершения работы сервера)" << std::endl;

  std::cin.get();

  // Останавливаем publisher
  publisher.stop();

  std::cout << "\n✅ Сервер завершил работу" << std::endl;

  return 0;
}

/*
 * Компиляция:
 * g++ -std=c++17 server_main.cpp -o server -lzmq -pthread
 *
 * Запуск:
 * ./server
 */