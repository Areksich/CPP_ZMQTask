# Компилятор и флаги
CXX = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -pthread
INCLUDES = -I./common -I/usr/local/include -I/usr/include
LIBS = -lzmq -pthread

# Директории
SERVER_DIR = server
CLIENT_DIR = client
COMMON_DIR = common

# Исходные файлы
SERVER_SRC = $(SERVER_DIR)/main.cpp
CLIENT_SRC = $(CLIENT_DIR)/main.cpp

# Заголовочные файлы
COMMON_HEADERS = $(COMMON_DIR)/Student.h \
                 $(COMMON_DIR)/Validator.h \
                 $(COMMON_DIR)/Serializer.h

SERVER_HEADERS = $(SERVER_DIR)/FileParser.h \
                 $(SERVER_DIR)/StudentMerger.h \
                 $(SERVER_DIR)/ZmqSyncedPublisher.h \
                 $(COMMON_HEADERS)

CLIENT_HEADERS = $(CLIENT_DIR)/ZmqSyncedSubscriber.h \
                 $(CLIENT_DIR)/StudentSorter.h \
                 $(COMMON_HEADERS)

# Целевые файлы
SERVER_TARGET = server_app
CLIENT_TARGET = client_app

all: $(SERVER_TARGET) $(CLIENT_TARGET)
	@echo "✅ Сборка завершена успешно!"
	@echo "   Используйте 'make run-server' для запуска сервера"
	@echo "   Используйте 'make run-client' для запуска клиента"

# Сборка сервера
$(SERVER_TARGET): $(SERVER_SRC) $(SERVER_HEADERS)
	@echo "🔨 Компиляция сервера..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I$(SERVER_DIR) $(SERVER_SRC) -o $(SERVER_TARGET) $(LIBS)
	@echo "✅ Сервер скомпилирован: $(SERVER_TARGET)"

# Сборка клиента
$(CLIENT_TARGET): $(CLIENT_SRC) $(CLIENT_HEADERS)
	@echo "🔨 Компиляция клиента..."
	$(CXX) $(CXXFLAGS) $(INCLUDES) -I$(CLIENT_DIR) $(CLIENT_SRC) -o $(CLIENT_TARGET) $(LIBS)
	@echo "✅ Клиент скомпилирован: $(CLIENT_TARGET)"

# Запуск сервера
run-server: $(SERVER_TARGET)
	@echo "🚀 Запуск сервера..."
	./$(SERVER_TARGET)

# Запуск клиента
run-client: $(CLIENT_TARGET)
	@echo "🚀 Запуск клиента..."
	./$(CLIENT_TARGET)

# Создание тестовых файлов
test-files:
	@echo "📝 Создание тестовых файлов..."
	@echo "1 Ivan Ivanovich Ivanov 01.01.1988" > student_file_1.txt
	@echo "2 Petr Petrovich Petrov 04.07.1988" >> student_file_1.txt
	@echo "3 Denis Denisovich Denisov 43.01.1988" >> student_file_1.txt
	@echo "4 Vladimir Vladimirovich Jukov 04.5.1987" >> student_file_1.txt
	@echo "5 Vladimir Vladimirovich Kochkin 11.10.1989" >> student_file_1.txt
	@echo "" >> student_file_1.txt
	@echo "34 Ivan Ivanovich Ivanov 01.01.1988" > student_file_2.txt
	@echo "54 Petr Petrovich Kazakov 05.07.1986" >> student_file_2.txt
	@echo "23 Denis Denisovich Denisov 04.03.1988" >> student_file_2.txt
	@echo "12 Vladimir Vladimirovich Jukov 04.5.1987" >> student_file_2.txt
	@echo "43 Vladimir Vladimirovich Kochkin 11.11.1989" >> student_file_2.txt
	@echo "" >> student_file_2.txt
	@echo "✅ Тестовые файлы созданы"

# Проверка зависимостей
check-deps:
	@echo "🔍 Проверка зависимостей..."
	@which $(CXX) > /dev/null || (echo "❌ $(CXX) не найден" && exit 1)
	@echo "✅ Компилятор $(CXX) найден"
	@pkg-config --exists libzmq || (echo "❌ libzmq не найден. Установите: sudo apt-get install libzmq3-dev" && exit 1)
	@echo "✅ ZeroMQ найден"
	@echo "✅ Все зависимости установлены"

# Очистка
clean:
	@echo "🧹 Очистка..."
	rm -f $(SERVER_TARGET) $(CLIENT_TARGET)
	rm -f *.o
	@echo "✅ Очистка завершена"

# Полная очистка (включая тестовые файлы)
distclean: clean
	rm -f student_file_1.txt student_file_2.txt
	@echo "✅ Полная очистка завершена"
