#pragma once

#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>
#include <zmq.hpp>

#include "../common/Student.h"
#include "Serializer.h"

class ZmqSubscriber {
 public:
  ZmqSubscriber(const std::string& endpoint)
      : endpoint_(endpoint), running_(false), dataReceived_(false) {}

  ~ZmqSubscriber() { stop(); }

  // Запуск подписки в отдельном потоке
  void start() {
    if (running_) {
      std::cerr << "⚠️  Subscriber уже запущен" << std::endl;
      return;
    }

    running_ = true;
    dataReceived_ = false;
    subscribeThread_ = std::thread(&ZmqSubscriber::subscribeLoop, this);

    std::cout << "📡 Subscriber запущен, подключение к " << endpoint_
              << std::endl;
  }

  // Остановка подписки
  void stop() {
    if (!running_) return;

    running_ = false;
    if (subscribeThread_.joinable()) {
      subscribeThread_.join();
    }

    std::cout << "📡 Subscriber остановлен" << std::endl;
  }

  // Получение принятых данных
  std::vector<Student> getReceivedData() {
    std::lock_guard<std::mutex> lock(dataMutex_);
    return receivedStudents_;
  }

  bool isDataReceived() const { return dataReceived_; }

  bool isRunning() const { return running_; }

 private:
  void subscribeLoop() {
    try {
      // Создаем ZMQ контекст и сокет
      zmq::context_t context(1);
      zmq::socket_t subscriber(context, zmq::socket_type::sub);

      // Подключаемся к publisher
      subscriber.connect(endpoint_);

      // Подписываемся на топик "students"
      subscriber.set(zmq::sockopt::subscribe, "students");

      // Устанавливаем таймаут получения
      subscriber.set(zmq::sockopt::rcvtimeo, 1000);  // 1 секунда

      std::cout << "⏳ Ожидание данных..." << std::endl;

      int attempts = 0;
      const int MAX_ATTEMPTS = 30;  // 30 секунд ожидания

      while (running_ && attempts < MAX_ATTEMPTS) {
        try {
          // Получаем топик
          zmq::message_t topic;
          auto result = subscriber.recv(topic, zmq::recv_flags::none);

          if (!result) {
            attempts++;
            continue;
          }

          // Получаем данные
          zmq::message_t message;
          result = subscriber.recv(message, zmq::recv_flags::none);

          if (!result) {
            attempts++;
            continue;
          }

          std::cout << "📥 Данные получены (" << message.size() << " байт)"
                    << std::endl;

          // Десериализуем данные
          std::string data(static_cast<char*>(message.data()), message.size());

          std::vector<Student> students = Serializer::deserialize(data);

          std::cout << "✅ Десериализовано " << students.size() << " студентов"
                    << std::endl;

          // Сохраняем данные
          {
            std::lock_guard<std::mutex> lock(dataMutex_);
            receivedStudents_ = students;
          }

          dataReceived_ = true;
          break;

        } catch (const zmq::error_t& e) {
          if (e.num() != EAGAIN) {  // Игнорируем таймауты
            throw;
          }
          attempts++;
        }
      }

      if (!dataReceived_) {
        std::cerr << "⚠️  Данные не получены за отведенное время" << std::endl;
      }

    } catch (const zmq::error_t& e) {
      std::cerr << "❌ ZMQ Subscriber ошибка: " << e.what() << std::endl;
    }

    running_ = false;
  }

  std::string endpoint_;
  std::atomic<bool> running_;
  std::atomic<bool> dataReceived_;
  std::thread subscribeThread_;
  std::vector<Student> receivedStudents_;
  std::mutex dataMutex_;
};