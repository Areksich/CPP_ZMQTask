#include <csignal>
#include <iostream>

#include "Extractor.h"
#include "GeometryAnalyzer.h"
#include "HttpServer.h"
#include "JsonSerializer.h"
#include "Structs.h"

// Глобальная переменная для управления сервером
HttpServer* globalServer = nullptr;

// Обработчик сигнала для корректного завершения
void signalHandler(int signum) {
  std::cout << "\nShutting down server...\n";
  if (globalServer) {
    globalServer->stop();
  }
  exit(signum);
}

// Валидация входных данных
bool validateInput(const std::string& text, std::string& error) {
  if (text.empty()) {
    error = "Text is empty";
    return false;
  }

  if (text.length() > 1000000) {  // 1MB лимит
    error = "Text is too large (max 1MB)";
    return false;
  }

  return true;
}

// Обработчик HTTP запросов
HttpServer::HttpResponse handleRequest(const HttpRequest& request) {
  // Отладочный вывод
  std::cout << "=== Received request ===\n";
  std::cout << "Method: " << request.method << "\n";
  std::cout << "Path: " << request.path << "\n";
  std::cout << "Body length: " << request.body.length() << "\n";
  std::cout << "Body preview (first 100 chars): "
            << request.body.substr(0,
                                   std::min(size_t(100), request.body.length()))
            << "\n";
  std::cout << "========================\n\n";

  // Поддерживаем только POST
  if (request.method != "POST") {
    if (request.method == "GET" && request.path == "/") {
      // Простая HTML страница для тестирования
      std::string html = R"HTML(<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <title>Coordinate Extractor</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        textarea { width: 100%; height: 200px; }
        button { padding: 10px 20px; margin: 10px 0; }
        pre { background: #f4f4f4; padding: 10px; overflow: auto; }
    </style>
</head>
<body>
    <h1>Coordinate Extractor API</h1>
    <p>POST /extract with text in body to extract coordinates</p>
    <h2>Test Form</h2>
    <textarea id="input" placeholder="Enter text with coordinates..."></textarea><br>
    <button onclick="extract()">Extract Coordinates</button>
    <h3>Result:</h3>
    <pre id="output"></pre>
    <script>
        function extract() {
            const text = document.getElementById('input').value;
            fetch('/extract', {
                method: 'POST',
                headers: {
                    'Content-Type': 'text/plain; charset=utf-8'
                },
                body: text
            })
            .then(r => r.json())
            .then(data => {
                document.getElementById('output').textContent = JSON.stringify(data, null, 2);
            })
            .catch(e => {
                document.getElementById('output').textContent = 'Error: ' + e;
            });
        }
    </script>
</body>
</html>)HTML";
      return HttpServer::HttpResponse(html, "text/html; charset=utf-8");
    }
    return HttpServer::HttpResponse(
        JsonSerializer::serializeError("Method not allowed. Use POST /extract"),
        "application/json");
  }

  // Проверяем путь
  if (request.path != "/extract" && request.path != "/") {
    return HttpServer::HttpResponse(
        JsonSerializer::serializeError("Invalid path. Use /extract"),
        "application/json");
  }

  // Валидация входных данных
  std::string validationError;
  if (!validateInput(request.body, validationError)) {
    return HttpServer::HttpResponse(
        JsonSerializer::serializeError(validationError), "application/json");
  }

  // Извлекаем координаты
  std::vector<ExtractedCoordinate> coordinates =
      CoordinateExtractor::extractCoordinates(request.body);

  // Анализируем геометрию
  GeometryType geometryType = GeometryAnalyzer::analyzeGeometry(coordinates);

  // Формируем результат
  ExtractionResult result;
  result.coordinates = coordinates;
  result.geometry_type = geometryType;

  if (coordinates.empty()) {
    result.message = "No valid coordinates found in text";
  } else {
    result.message = "Successfully extracted " +
                     std::to_string(coordinates.size()) + " coordinate(s)";
  }

  // Сериализуем в JSON
  return HttpServer::HttpResponse(JsonSerializer::serializeResult(result),
                                  "application/json");
}

int main(int argc, char* argv[]) {
  // Порт по умолчанию
  int port = 8080;

  // Парсинг аргументов командной строки
  if (argc > 1) {
    try {
      port = std::stoi(argv[1]);
      if (port < 1 || port > 65535) {
        std::cerr << "Invalid port number. Using default port 8080\n";
        port = 8080;
      }
    } catch (...) {
      std::cerr << "Invalid port argument. Using default port 8080\n";
      port = 8080;
    }
  }

  // Создаем сервер
  HttpServer server(port);
  globalServer = &server;

  // Устанавливаем обработчик сигналов
  signal(SIGINT, signalHandler);
  signal(SIGTERM, signalHandler);

  // Запускаем сервер
  if (!server.start()) {
    std::cerr << "Failed to start server\n";
    return 1;
  }

  std::cout << "Coordinate Extractor Service is running\n";
  std::cout << "Listening on port " << port << "\n";
  std::cout << "Send POST requests to /extract with text in body\n";
  std::cout << "Press Ctrl+C to stop\n\n";

  // Обрабатываем запросы
  server.handleRequests(handleRequest);

  return 0;
}