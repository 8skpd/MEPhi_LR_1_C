CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g $(shell pkg-config --cflags gtk+-3.0)
LDFLAGS = $(shell pkg-config --libs gtk+-3.0) -lm

# === Директория для исполняемых файлов ===
DIR_BIN = bin

# === Цели ===
TARGET_CONSOLE = $(DIR_BIN)/matrix_app
TARGET_GUI = $(DIR_BIN)/matrix_gui_app
TARGET_EDGE = $(DIR_BIN)/matrix_edge_tests

# === Исходные файлы ===
SRCS_CONSOLE = main.c matrix.c matrix_ops.c matrix_tests.c utils.c
SRCS_GUI = main_gui.c matrix.c matrix_ops.c matrix_gui.c matrix_tests.c utils.c
SRCS_EDGE = main_edge_tests.c matrix.c matrix_ops.c matrix_edge_tests.c utils.c

all: $(DIR_BIN) $(TARGET_CONSOLE) $(TARGET_GUI) $(TARGET_EDGE)

# === Создание директории bin ===
$(DIR_BIN):
	mkdir -p $(DIR_BIN)

# === Сборка исполняемых файлов ===
$(TARGET_CONSOLE): $(SRCS_CONSOLE) | $(DIR_BIN)
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET_GUI): $(SRCS_GUI) | $(DIR_BIN)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TARGET_EDGE): $(SRCS_EDGE) | $(DIR_BIN)
	$(CC) $(CFLAGS) -o $@ $^ -lm

# === Цели для отдельных сборок ===
gui: $(TARGET_GUI)
console: $(TARGET_CONSOLE)
edge-tests: $(TARGET_EDGE)

# === Очистка ===
clean:
	rm -rf $(DIR_BIN)

# === Запуск ===
run: $(TARGET_CONSOLE)
	./$(TARGET_CONSOLE)

run-gui: $(TARGET_GUI)
	./$(TARGET_GUI)

run-edge: $(TARGET_EDGE)
	./$(TARGET_EDGE)

.PHONY: all gui console edge-tests clean run run-gui run-edge