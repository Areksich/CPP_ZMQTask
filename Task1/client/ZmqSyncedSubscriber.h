#pragma once

#include <atomic>
#include <iostream>
#include <mutex>
#include <thread>
#include <zmq.hpp>

#include "../common/Student.h"
#include "Serializer.h"

class ZmqSyncedSubscriber {
 public:
  ZmqSyncedSubscriber(const std::string& subEndpoint,
                      const std::string& syncEndpoint)
      : subEndpoint_(subEndpoint),
        syncEndpoint_(syncEndpoint),
        running_(false),
        dataReceived_(false) {}

  ~ZmqSyncedSubscriber() { stop(); }

  // Запуск подписки в отдельном потоке
  void start() {
    if (running_) {
      std::cerr << "⚠️  Subscriber уже запущен" << std::endl;
      return;
    }

    running_ = true;
    dataReceived_ = false;
    subscribeThread_ = std::thread(&ZmqSyncedSubscriber::subscribeLoop, this);

    std::cout << "📡 Synced Subscriber запущен" << std::endl;
  }

  // Остановка подписки
  void stop() {
    if (!running_) return;

    running_ = false;
    if (subscribeThread_.joinable()) {
      subscribeThread_.join();
    }

    std::cout << "📡 Synced Subscriber остановлен" << std::endl;
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
      zmq::context_t context(1);

      // SUB сокет для получения данных
      zmq::socket_t subscriber(context, zmq::socket_type::sub);
      subscriber.connect(subEndpoint_);
      subscriber.set(zmq::sockopt::subscribe, "students");

      // REQ сокет для синхронизации с publisher
      zmq::socket_t syncClient(context, zmq::socket_type::req);
      syncClient.connect(syncEndpoint_);

      std::cout << "📡 Подключение к серверу:" << std::endl;
      std::cout << "   SUB: " << subEndpoint_ << std::endl;
      std::cout << "   SYNC: " << syncEndpoint_ << std::endl;

      // Отправляем сигнал готовности publisher'у
      std::cout << "📤 Отправка сигнала готовности..." << std::endl;
      zmq::message_t syncMsg(5);
      memcpy(syncMsg.data(), "READY", 5);
      syncClient.send(syncMsg, zmq::send_flags::none);

      // Ждем подтверждения
      zmq::message_t reply;
      auto result = syncClient.recv(reply, zmq::recv_flags::none);

      if (!result) {
        std::cerr << "❌ Не получено подтверждение от сервера" << std::endl;
        running_ = false;
        return;
      }

      std::cout << "✅ Получено подтверждение, готов к приему данных"
                << std::endl;

      // Устанавливаем таймаут получения
      subscriber.set(zmq::sockopt::rcvtimeo, 1000);

      std::cout << "⏳ Ожидание данных..." << std::endl;

      int attempts = 0;
      const int MAX_ATTEMPTS = 30;

      while (running_ && attempts < MAX_ATTEMPTS) {
        try {
          // Получаем топик
          zmq::message_t topic;
          result = subscriber.recv(topic, zmq::recv_flags::none);

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
          if (e.num() != EAGAIN) {
            throw;
          }
          attempts++;
        }
      }

      if (!dataReceived_) {
        std::cerr << "⚠️  Данные не получены за отведенное время" << std::endl;
      }

    } catch (const zmq::error_t& e) {
      std::cerr << "❌ ZMQ Synced Subscriber ошибка: " << e.what() << std::endl;
    }

    running_ = false;
  }

  std::string subEndpoint_;
  std::string syncEndpoint_;
  std::atomic<bool> running_;
  std::atomic<bool> dataReceived_;
  std::thread subscribeThread_;
  std::vector<Student> receivedStudents_;
  std::mutex dataMutex_;
};