CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g $(shell pkg-config --cflags gtk+-3.0)
LDFLAGS = $(shell pkg-config --libs gtk+-3.0) -lm

DIR_BIN = bin

# Цели
TARGET_CONSOLE = $(DIR_BIN)/matrix_app
TARGET_GUI = $(DIR_BIN)/matrix_gui_app
TARGET_EDGE = $(DIR_BIN)/matrix_edge_tests
TARGET_BENCH = $(DIR_BIN)/matrix_benchmark

# Исходные файлы
SRCS_CONSOLE = main.c matrix.c matrix_ops.c matrix_tests.c matrix_linalg.c utils.c
SRCS_GUI = main_gui.c matrix.c matrix_ops.c matrix_gui.c matrix_tests.c matrix_linalg.c utils.c
SRCS_EDGE = main_edge_tests.c matrix.c matrix_ops.c matrix_edge_tests.c matrix_linalg.c utils.c
SRCS_BENCH = main_benchmark.c matrix.c matrix_ops.c matrix_linalg.c matrix_benchmark.c utils.c 

# Сборка
all: $(DIR_BIN) $(TARGET_CONSOLE) $(TARGET_GUI) $(TARGET_EDGE) $(TARGET_BENCH)

$(DIR_BIN):
	mkdir -p $(DIR_BIN)

$(TARGET_CONSOLE): $(patsubst %.c,$(DIR_BIN)/%.o,$(SRCS_CONSOLE)) | $(DIR_BIN)
	$(CC) $(CFLAGS) -o $@ $^ -lm

$(TARGET_GUI): $(patsubst %.c,$(DIR_BIN)/%.o,$(SRCS_GUI)) | $(DIR_BIN)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

$(TARGET_EDGE): $(patsubst %.c,$(DIR_BIN)/%.o,$(SRCS_EDGE)) | $(DIR_BIN)
	$(CC) $(CFLAGS) -o $@ $^ -lm

$(TARGET_BENCH): $(patsubst %.c,$(DIR_BIN)/%.o,$(SRCS_BENCH)) | $(DIR_BIN)
	$(CC) $(CFLAGS) -o $@ $^ -lm

# Компиляция объектных файлов
$(DIR_BIN)/%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

# Цели
gui: $(TARGET_GUI)
console: $(TARGET_CONSOLE)
edge-tests: $(TARGET_EDGE)
benchmark: $(TARGET_BENCH)

clean:
	rm -rf $(DIR_BIN)

run: $(TARGET_CONSOLE)
	./$(TARGET_CONSOLE)

run-gui: $(TARGET_GUI)
	./$(TARGET_GUI)

run-edge: $(TARGET_EDGE)
	./$(TARGET_EDGE)

run-bench: $(TARGET_BENCH)
	./$(TARGET_BENCH)

.PHONY: all gui console edge-tests benchmark clean run run-gui run-edge run-bench
