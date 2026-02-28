#ifndef MATRIX_GUI_H
#define MATRIX_GUI_H

#include "matrix.h"
#include <gtk/gtk.h>
#include <time.h>

// Структура результата теста
typedef struct {
    const char *name;
    void (*test_func)(void);
    gboolean passed;
    double execution_time_ms;
    char *output;
} TestResult;

// Контекст приложения GUI
typedef struct {
    GtkWidget *window;
    GtkWidget *notebook;
    GtkWidget *status_bar;
    GtkWidget *time_label;
    GtkWidget *tree_view;
    GtkWidget *total_time_label;
    TestResult *results;
    int test_count;
    int current_test;
    int selected_test;
} GuiContext;

// Инициализация и запуск GUI
void gui_init(int *argc, char ***argv);
void gui_run(void);
GuiContext* gui_get_context(void);

// Создание главного окна
GtkWidget* gui_create_main_window(void);

// Добавление теста в интерфейс
void gui_add_test(const char *name, void (*test_func)(void));

// Запуск выбранного теста с измерением времени
void gui_run_test(int test_index);

// Обновление статуса в GUI
void gui_update_status(const char *message, double time_ms, gboolean success);

// Получить все тесты для GUI
void gui_get_test_list(const char ***names, void (***funcs)(void), int *count);

#endif // MATRIX_GUI_H