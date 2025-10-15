#pragma once

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <zmq.hpp>

#include "../common/Student.h"
#include "Serializer.h"

class ZmqPublisher {
 public:
  ZmqPublisher(const std::string& endpoint)
      : endpoint_(endpoint), running_(false) {}

  ~ZmqPublisher() { stop(); }

  // Запуск публикации в отдельном потоке
  void start(const std::vector<Student>& students) {
    if (running_) {
      std::cerr << "⚠️  Publisher уже запущен" << std::endl;
      return;
    }

    running_ = true;
    publishThread_ = std::thread(&ZmqPublisher::publishLoop, this, students);

    std::cout << "📡 Publisher запущен на " << endpoint_ << std::endl;
  }

  // Остановка публикации
  void stop() {
    if (!running_) return;

    running_ = false;
    if (publishThread_.joinable()) {
      publishThread_.join();
    }

    std::cout << "📡 Publisher остановлен" << std::endl;
  }

  bool isRunning() const { return running_; }

 private:
  void publishLoop(const std::vector<Student> students) {
    try {
      // Создаем ZMQ контекст и сокет
      zmq::context_t context(1);
      zmq::socket_t publisher(context, zmq::socket_type::pub);

      // Привязываем сокет
      publisher.bind(endpoint_);

      // Даем время подписчикам подключиться
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

      // Сериализуем данные
      std::string data = Serializer::serialize(students);

      std::cout << "📤 Отправка данных (" << students.size() << " студентов)..."
                << std::endl;

      int messagesSent = 0;
      const int MAX_MESSAGES = 10;  // Отправляем несколько раз для надежности

      while (running_ && messagesSent < MAX_MESSAGES) {
        // Отправляем топик и данные
        zmq::message_t topic(8);
        memcpy(topic.data(), "students", 8);
        publisher.send(topic, zmq::send_flags::sndmore);

        zmq::message_t message(data.size());
        memcpy(message.data(), data.c_str(), data.size());
        publisher.send(message, zmq::send_flags::none);

        messagesSent++;
        std::cout << "   Сообщение " << messagesSent << "/" << MAX_MESSAGES
                  << " отправлено" << std::endl;

        // Пауза между отправками
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }

      std::cout << "✅ Отправка завершена" << std::endl;

      // Даем время доставить последнее сообщение
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

    } catch (const zmq::error_t& e) {
      std::cerr << "❌ ZMQ Publisher ошибка: " << e.what() << std::endl;
    }

    running_ = false;
  }

  std::string endpoint_;
  std::atomic<bool> running_;
  std::thread publishThread_;
};