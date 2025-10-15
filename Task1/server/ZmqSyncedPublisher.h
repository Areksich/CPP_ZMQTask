#pragma once

#include <atomic>
#include <chrono>
#include <iostream>
#include <thread>
#include <zmq.hpp>

#include "../common/Student.h"
#include "Serializer.h"

class ZmqSyncedPublisher {
 public:
  ZmqSyncedPublisher(const std::string& pubEndpoint,
                     const std::string& syncEndpoint)
      : pubEndpoint_(pubEndpoint),
        syncEndpoint_(syncEndpoint),
        running_(false) {}

  ~ZmqSyncedPublisher() { stop(); }

  // Запуск публикации в отдельном потоке
  void start(const std::vector<Student>& students,
             int expectedSubscribers = 1) {
    if (running_) {
      std::cerr << "⚠️  Publisher уже запущен" << std::endl;
      return;
    }

    running_ = true;
    studentsData_ = students;
    expectedSubscribers_ = expectedSubscribers;
    publishThread_ = std::thread(&ZmqSyncedPublisher::publishLoop, this);

    std::cout << "📡 Synced Publisher запущен" << std::endl;
    std::cout << "   PUB: " << pubEndpoint_ << std::endl;
    std::cout << "   SYNC: " << syncEndpoint_ << std::endl;
  }

  // Остановка публикации
  void stop() {
    if (!running_) return;

    running_ = false;
    if (publishThread_.joinable()) {
      publishThread_.join();
    }

    std::cout << "📡 Synced Publisher остановлен" << std::endl;
  }

  bool isRunning() const { return running_; }

 private:
  void publishLoop() {
    try {
      zmq::context_t context(1);

      // PUB сокет для отправки данных
      zmq::socket_t publisher(context, zmq::socket_type::pub);
      publisher.bind(pubEndpoint_);

      // REP сокет для синхронизации с подписчиками
      zmq::socket_t syncService(context, zmq::socket_type::rep);
      syncService.bind(syncEndpoint_);

      std::cout << "⏳ Ожидание подключения " << expectedSubscribers_
                << " подписчиков..." << std::endl;

      // Ждем сигналы готовности от всех подписчиков
      int subscribersReady = 0;
      while (subscribersReady < expectedSubscribers_ && running_) {
        zmq::message_t message;
        auto result = syncService.recv(message, zmq::recv_flags::dontwait);

        if (result) {
          subscribersReady++;
          std::cout << "   Подписчик #" << subscribersReady << " готов"
                    << std::endl;

          // Отправляем подтверждение
          zmq::message_t reply(2);
          memcpy(reply.data(), "OK", 2);
          syncService.send(reply, zmq::send_flags::none);
        }

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }

      if (subscribersReady < expectedSubscribers_) {
        std::cerr << "⚠️  Не все подписчики подключились" << std::endl;
        running_ = false;
        return;
      }

      std::cout << "✅ Все подписчики готовы, начинаем отправку" << std::endl;

      // Небольшая пауза для стабильности
      std::this_thread::sleep_for(std::chrono::milliseconds(200));

      // Сериализуем данные
      std::string data = Serializer::serialize(studentsData_);

      std::cout << "📤 Отправка данных (" << studentsData_.size()
                << " студентов)..." << std::endl;

      int messagesSent = 0;
      const int MAX_MESSAGES = 5;

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

        std::this_thread::sleep_for(std::chrono::milliseconds(100));
      }

      std::cout << "✅ Отправка завершена" << std::endl;

      // Даем время доставить последнее сообщение
      std::this_thread::sleep_for(std::chrono::milliseconds(500));

    } catch (const zmq::error_t& e) {
      std::cerr << "❌ ZMQ Synced Publisher ошибка: " << e.what() << std::endl;
    }

    running_ = false;
  }

  std::string pubEndpoint_;
  std::string syncEndpoint_;
  std::atomic<bool> running_;
  std::thread publishThread_;
  std::vector<Student> studentsData_;
  int expectedSubscribers_;
};