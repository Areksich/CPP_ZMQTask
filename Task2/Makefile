CXX = g++

CXXFLAGS = -std=c++17 -Wall -Wextra -O2 -pthread

# Для Windows нужна библиотека ws2_32
ifeq ($(OS),Windows_NT)
    LDFLAGS = -lws2_32
    EXECUTABLE = coord_extractor.exe
else
    LDFLAGS = -pthread
    EXECUTABLE = coord_extractor
endif

# Исходные файлы
SOURCES = main.cpp

# Заголовочные файлы
HEADERS = Structs.h \
          Validator.h \
          Parser.h \
          Extractor.h \
          GeometryAnalyzer.h \
          JsonSerializer.h \
          HttpServer.h

all: $(EXECUTABLE)

# Сборка исполняемого файла
$(EXECUTABLE): $(SOURCES) $(HEADERS)
	$(CXX) $(CXXFLAGS) $(SOURCES) -o $(EXECUTABLE) $(LDFLAGS)
	@echo "Build complete: $(EXECUTABLE)"

# Запуск сервера
run: $(EXECUTABLE)
	./$(EXECUTABLE)

# Запуск на указанном порту
run-port: $(EXECUTABLE)
	./$(EXECUTABLE) $(PORT)

# Очистка
clean:
	rm -f $(EXECUTABLE) test_runner
	@echo "Clean complete"