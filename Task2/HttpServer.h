#pragma once

#include <functional>
#include <iostream>
#include <map>
#include <sstream>
#include <string>

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#else
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

struct HttpRequest {
  std::string method;
  std::string path;
  std::string body;
  std::map<std::string, std::string> headers;
};

class HttpServer {
 private:
  int port;
  SOCKET serverSocket;
  bool running;

#ifdef _WIN32
  WSADATA wsaData;
#endif

  // Парсинг HTTP запроса
  HttpRequest parseRequest(const std::string& rawRequest) {
    HttpRequest request;

    // Находим конец заголовков (\r\n\r\n или \n\n)
    size_t bodyStart = rawRequest.find("\r\n\r\n");
    if (bodyStart != std::string::npos) {
      bodyStart += 4;  // Пропускаем \r\n\r\n
    } else {
      bodyStart = rawRequest.find("\n\n");
      if (bodyStart != std::string::npos) {
        bodyStart += 2;  // Пропускаем \n\n
      }
    }

    // Извлекаем заголовки
    std::string headerSection = (bodyStart != std::string::npos)
                                    ? rawRequest.substr(0, bodyStart)
                                    : rawRequest;

    std::istringstream stream(headerSection);
    std::string line;

    // Первая строка: метод путь версия
    if (std::getline(stream, line)) {
      // Убираем \r если есть
      if (!line.empty() && line.back() == '\r') {
        line.pop_back();
      }
      std::istringstream firstLine(line);
      firstLine >> request.method >> request.path;
    }

    // Заголовки
    while (std::getline(stream, line)) {
      // Убираем \r если есть
      if (!line.empty() && line.back() == '\r') {
        line.pop_back();
      }

      // Пустая строка означает конец заголовков
      if (line.empty()) {
        break;
      }

      size_t colon = line.find(':');
      if (colon != std::string::npos) {
        std::string key = line.substr(0, colon);
        std::string value = line.substr(colon + 1);
        // Убираем пробелы
        value.erase(0, value.find_first_not_of(" \t\r\n"));
        value.erase(value.find_last_not_of(" \t\r\n") + 1);
        request.headers[key] = value;
      }
    }

    // Тело запроса - все что после разделителя заголовков
    if (bodyStart != std::string::npos && bodyStart < rawRequest.length()) {
      request.body = rawRequest.substr(bodyStart);
    }

    return request;
  }

  // Отправка HTTP ответа
  void sendResponse(SOCKET clientSocket, int statusCode,
                    const std::string& statusText, const std::string& body,
                    const std::string& contentType = "application/json") {
    std::ostringstream response;
    response << "HTTP/1.1 " << statusCode << " " << statusText << "\r\n";
    response << "Content-Type: " << contentType << "; charset=utf-8\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Access-Control-Allow-Origin: *\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;

    std::string responseStr = response.str();
    send(clientSocket, responseStr.c_str(),
         static_cast<int>(responseStr.length()), 0);
  }

 public:
  struct HttpResponse {
    std::string body;
    std::string contentType;

    HttpResponse(const std::string& b,
                 const std::string& ct = "application/json")
        : body(b), contentType(ct) {}
  };

  using RequestHandler = std::function<HttpResponse(const HttpRequest&)>;

  HttpServer(int port)
      : port(port), serverSocket(INVALID_SOCKET), running(false) {
#ifdef _WIN32
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
      throw std::runtime_error("WSAStartup failed");
    }
#endif
  }

  ~HttpServer() {
    stop();
#ifdef _WIN32
    WSACleanup();
#endif
  }

  // Запуск сервера
  bool start() {
    serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET) {
      std::cerr << "Failed to create socket\n";
      return false;
    }

    // Разрешаем переиспользование адреса
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt,
               sizeof(opt));

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(port);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) ==
        SOCKET_ERROR) {
      std::cerr << "Bind failed\n";
      closesocket(serverSocket);
      return false;
    }

    if (listen(serverSocket, 10) == SOCKET_ERROR) {
      std::cerr << "Listen failed\n";
      closesocket(serverSocket);
      return false;
    }

    running = true;
    std::cout << "Server started on port " << port << std::endl;
    return true;
  }

  // Остановка сервера
  void stop() {
    running = false;
    if (serverSocket != INVALID_SOCKET) {
      closesocket(serverSocket);
      serverSocket = INVALID_SOCKET;
    }
  }

  // Обработка запросов
  void handleRequests(RequestHandler handler) {
    while (running) {
      sockaddr_in clientAddr;
      socklen_t clientLen = sizeof(clientAddr);
      SOCKET clientSocket =
          accept(serverSocket, (sockaddr*)&clientAddr, &clientLen);

      if (clientSocket == INVALID_SOCKET) {
        if (running) {
          std::cerr << "Accept failed\n";
        }
        continue;
      }

      // Читаем запрос
      char buffer[65536] = {0};
      int bytesRead = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

      if (bytesRead > 0) {
        std::string rawRequest(buffer, bytesRead);
        HttpRequest request = parseRequest(rawRequest);

        try {
          // Обрабатываем OPTIONS для CORS
          if (request.method == "OPTIONS") {
            std::ostringstream response;
            response << "HTTP/1.1 204 No Content\r\n";
            response << "Access-Control-Allow-Origin: *\r\n";
            response << "Access-Control-Allow-Methods: POST, GET, OPTIONS\r\n";
            response << "Access-Control-Allow-Headers: Content-Type\r\n";
            response << "Connection: close\r\n\r\n";
            std::string resp = response.str();
            send(clientSocket, resp.c_str(), static_cast<int>(resp.length()),
                 0);
          }
          // Вызываем обработчик
          else {
            HttpResponse response = handler(request);
            sendResponse(clientSocket, 200, "OK", response.body,
                         response.contentType);
          }
        } catch (const std::exception& e) {
          std::string error = "{\"error\": \"" + std::string(e.what()) + "\"}";
          sendResponse(clientSocket, 500, "Internal Server Error", error);
        }
      }

      closesocket(clientSocket);
    }
  }
};