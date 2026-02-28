CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -g $(shell pkg-config --cflags gtk+-3.0)
LDFLAGS = $(shell pkg-config --libs gtk+-3.0)

TARGET_CONSOLE = matrix_app
SRCS_CONSOLE = main.c matrix.c matrix_ops.c matrix_tests.c utils.c
OBJS_CONSOLE = $(SRCS_CONSOLE:.c=.o)

TARGET_GUI = matrix_gui_app
SRCS_GUI = main_gui.c matrix.c matrix_ops.c matrix_gui.c matrix_tests.c utils.c
OBJS_GUI = $(SRCS_GUI:.c=.o)

all: $(TARGET_CONSOLE) $(TARGET_GUI)

$(TARGET_CONSOLE): $(OBJS_CONSOLE)
	$(CC) $(CFLAGS) -o $@ $^

$(TARGET_GUI): $(OBJS_GUI)
	$(CC) $(CFLAGS) -o $@ $^ $(LDFLAGS)

gui: $(TARGET_GUI)
console: $(TARGET_CONSOLE)

%.o: %.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -f $(OBJS_CONSOLE) $(OBJS_GUI) $(TARGET_CONSOLE) $(TARGET_GUI)

run: $(TARGET_CONSOLE)
	./$(TARGET_CONSOLE)

run-gui: $(TARGET_GUI)
	./$(TARGET_GUI)

.PHONY: all gui console clean run run-gui