#include <chrono>
#include <iostream>
#include <thread>

#include "../common/Student.h"
#include "StudentSorter.h"
#include "ZmqSubscriber.h"

int main() {
  std::cout << "╔════════════════════════════════════════════════════╗"
            << std::endl;
  std::cout << "║          КЛИЕНТ УПРАВЛЕНИЯ СТУДЕНТАМИ              ║"
            << std::endl;
  std::cout << "╚════════════════════════════════════════════════════╝"
            << std::endl;
  std::cout << std::endl;

  // Шаг 1: Подключение к серверу
  std::cout << "📖 ШАГ 1: Подключение к серверу\n" << std::endl;

  const std::string endpoint = "tcp://localhost:5555";
  ZmqSubscriber subscriber(endpoint);

  // Запускаем подписку в отдельном потоке
  subscriber.start();

  // Ждем получения данных
  std::cout << "\n⏳ Ожидание данных от сервера..." << std::endl;

  int waitTime = 0;
  const int MAX_WAIT_TIME = 30;  // 30 секунд

  while (!subscriber.isDataReceived() && waitTime < MAX_WAIT_TIME) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    waitTime++;

    if (waitTime % 5 == 0) {
      std::cout << "   Ожидание... (" << waitTime << " сек)" << std::endl;
    }
  }

  // Останавливаем subscriber
  subscriber.stop();

  if (!subscriber.isDataReceived()) {
    std::cerr << "\n❌ Ошибка: Не удалось получить данные от сервера"
              << std::endl;
    std::cerr << "   Убедитесь, что сервер запущен и доступен" << std::endl;
    return 1;
  }

  // Шаг 2: Получение данных
  std::cout << "\n📖 ШАГ 2: Обработка полученных данных\n" << std::endl;

  auto students = subscriber.getReceivedData();

  if (students.empty()) {
    std::cerr << "⚠️  Предупреждение: Получен пустой список студентов"
              << std::endl;
    return 0;
  }

  std::cout << "✅ Получено студентов: " << students.size() << std::endl;

  // Шаг 3: Сортировка студентов
  std::cout << "\n📖 ШАГ 3: Сортировка студентов по ФИО\n" << std::endl;

  StudentSorter::sortByFullName(students);

  // Шаг 4: Вывод на экран
  std::cout << "\n📖 ШАГ 4: Отображение результатов\n" << std::endl;

  StudentSorter::displayStudents(students);

  std::cout << "\n✅ Клиент завершил работу" << std::endl;

  return 0;
}

/*
 * Компиляция:
 * g++ -std=c++17 client_main.cpp -o client -lzmq -pthread
 *
 * Запуск:
 * ./client
 */