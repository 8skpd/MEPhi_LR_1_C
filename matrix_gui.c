#include "matrix_gui.h"
#include "matrix_tests.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

extern const TypeHandler* get_type_handler_int(void);
extern const TypeHandler* get_type_handler_double(void);
extern const TypeHandler** get_all_type_handlers(int *count);

static GuiContext context = {NULL, NULL, NULL, NULL, NULL, NULL, NULL, 0, 0, -1};

// Таймер выполнения
static double get_time_ms(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0;
}

// Список тестов
static const char *test_names[] = {
    "1: Создание и вывод (DOUBLE)",
    "2: Умножение матриц (INT)",
    "3: Сложение матриц (DOUBLE)",
    "4: Транспонирование (INT)",
    "5: Линейная комбинация (DOUBLE)",
    "6: Заполнение и операции (INT)",
    "7: Большая матрица (100x100)",
    "8: Обработка ошибок",
    "9: Все операции последовательно"
};

extern void test_create_double_matrices(void);
extern void test_multiply_int_matrices(void);
extern void test_add_double_matrices(void);
extern void test_transpose_int_matrix(void);
extern void test_linear_combination_double(void);
extern void test_fill_and_operations(void);
extern void test_large_matrix(void);
extern void test_error_handling(void);
extern void test_all_operations_sequence(void);

static void (*test_funcs[])(void) = {
    test_create_double_matrices,
    test_multiply_int_matrices,
    test_add_double_matrices,
    test_transpose_int_matrix,
    test_linear_combination_double,
    test_fill_and_operations,
    test_large_matrix,
    test_error_handling,
    test_all_operations_sequence
};

static const int TEST_COUNT = 9;

GuiContext* gui_get_context(void) {
    return &context;
}

void gui_init(int *argc, char ***argv) {
    gtk_init(argc, argv);
    context.results = calloc(TEST_COUNT, sizeof(TestResult));
    context.test_count = TEST_COUNT;
    context.selected_test = -1;
    
    for (int i = 0; i < TEST_COUNT; i++) {
        context.results[i].name = test_names[i];
        context.results[i].test_func = test_funcs[i];
        context.results[i].passed = FALSE;
        context.results[i].execution_time_ms = 0.0;
        context.results[i].output = NULL;
    }
}

// Обработчик выбора строки
static void on_test_selected(GtkTreeSelection *selection, gpointer data) {
    (void)data;
    
    GtkTreeIter iter;
    GtkTreeModel *model;
    char *num_str;
    
    if (gtk_tree_selection_get_selected(selection, &model, &iter)) {
        gtk_tree_model_get(model, &iter, 0, &num_str, -1);
        context.selected_test = atoi(num_str) - 1;
        g_free(num_str);
        
        char msg[256];
        snprintf(msg, sizeof(msg), "Выбран тест %d: %s", 
                 context.selected_test + 1, 
                 test_names[context.selected_test]);
        gtk_label_set_text(GTK_LABEL(context.status_bar), msg);
    }
}

// Обработчики кнопок:
// Запустить все
static void on_btn_run_all_clicked(GtkWidget *btn, gpointer data) {
    (void)btn;
    (void)data;
    GuiContext *ctx = gui_get_context();
    
    // Замер общего времени
    double total_start = get_time_ms();
    
    // Запуск всех тестов
    for (int i = 0; i < ctx->test_count; i++) {
        gui_run_test(i);
    }
    
    double total_end = get_time_ms();
    double total_time = total_end - total_start;
    
    // Обновление метки суммарного времени
    char total_msg[256];
    snprintf(total_msg, sizeof(total_msg), 
             "⏱ Суммарное время всех тестов: %.2f мс", total_time);
    gtk_label_set_text(GTK_LABEL(ctx->total_time_label), total_msg);
    
    // Также выводим в консоль
    printf("\n========================================\n");
    printf("ВСЕ ТЕСТЫ ЗАВЕРШЕНЫ\n");
    printf("Суммарное время: %.2f мс\n", total_time);
    printf("========================================\n");
}

static void on_btn_run_selected_clicked(GtkWidget *btn, gpointer data) {
    (void)btn;
    (void)data;
    GuiContext *ctx = gui_get_context();
    
    if (ctx->selected_test >= 0 && ctx->selected_test < ctx->test_count) {
        gui_run_test(ctx->selected_test);  // Запускаем выбранный тест
    } else {
        gtk_label_set_text(GTK_LABEL(ctx->status_bar), 
                          "⚠️ Выберите тест из списка!");
    }
}

static void on_btn_clear_clicked(GtkWidget *btn, gpointer data) {
    (void)btn;
    (void)data;
    GuiContext *ctx = gui_get_context();
    
    // 1. Очищаем статус-бары
    gtk_label_set_text(GTK_LABEL(ctx->status_bar), "Результаты очищены");
    gtk_label_set_text(GTK_LABEL(ctx->time_label), "Время выполнения: -- мс");
    gtk_label_set_text(GTK_LABEL(ctx->total_time_label), "Суммарное время: -- мс");
    
    // 2. Очищаем таблицу результатов
    if (ctx->tree_view) {
        GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(ctx->tree_view));
        GtkTreeIter iter;
        
        // Проверяем, есть ли данные в модели
        if (gtk_tree_model_get_iter_first(model, &iter)) {
            do {
                // Сбрасываем колонки: время (2) и статус (3)
                gtk_list_store_set(GTK_LIST_STORE(model), &iter,
                                  2, "--",           // Время
                                  3, "Ожидание",    // Статус
                                  -1);
            } while (gtk_tree_model_iter_next(model, &iter));
        }
    }
    
    // 3. Сбрасываем внутренний счётчик выбранного теста
    ctx->selected_test = -1;
    
    printf("🗑 Результаты тестов очищены\n");
}

static void on_btn_exit_clicked(GtkWidget *btn, gpointer data) {
    (void)btn;
    (void)data;
    gtk_main_quit();
}

// Создание главного окна
GtkWidget* gui_create_main_window(void) {
    context.window = gtk_window_new(GTK_WINDOW_TOPLEVEL);
    gtk_window_set_title(GTK_WINDOW(context.window), "Программа для теста операций с матрицами");
    gtk_window_set_default_size(GTK_WINDOW(context.window), 900, 700);
    gtk_container_set_border_width(GTK_CONTAINER(context.window), 10);
    
    GtkWidget *vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 10);
    
    // Заголовок
    GtkWidget *title = gtk_label_new(NULL);
    gtk_label_set_markup(GTK_LABEL(title), 
        "<span size='large' weight='bold'>Тесты матриц</span>");
    gtk_box_pack_start(GTK_BOX(vbox), title, FALSE, FALSE, 0);
    
    // Кнопки
    GtkWidget *button_box = gtk_box_new(GTK_ORIENTATION_HORIZONTAL, 5);
    
    GtkWidget *btn_run_all = gtk_button_new_with_label("▶ Запустить все тесты");
    GtkWidget *btn_run_selected = gtk_button_new_with_label("▶ Запустить выбранный");
    GtkWidget *btn_clear = gtk_button_new_with_label("🗑 Очистить результаты");
    GtkWidget *btn_exit = gtk_button_new_with_label("✖ Выход");
    
    gtk_box_pack_start(GTK_BOX(button_box), btn_run_all, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), btn_run_selected, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), btn_clear, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(button_box), btn_exit, FALSE, FALSE, 0);
    
    gtk_box_pack_start(GTK_BOX(vbox), button_box, FALSE, FALSE, 0);
    
    // Список тестов
    GtkWidget *scroll_window = gtk_scrolled_window_new(NULL, NULL);
    gtk_scrolled_window_set_policy(GTK_SCROLLED_WINDOW(scroll_window),
                                   GTK_POLICY_AUTOMATIC, GTK_POLICY_AUTOMATIC);
    gtk_widget_set_size_request(scroll_window, -1, 300);
    
    GtkListStore *store = gtk_list_store_new(4, G_TYPE_STRING, G_TYPE_STRING, 
                                                G_TYPE_STRING, G_TYPE_STRING);
    GtkWidget *tree_view = gtk_tree_view_new_with_model(GTK_TREE_MODEL(store));
    context.tree_view = tree_view;
    g_object_unref(store);
    
    // Колонки
    GtkTreeViewColumn *column;
    column = gtk_tree_view_column_new_with_attributes("№", 
        gtk_cell_renderer_text_new(), "text", 0, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
    
    column = gtk_tree_view_column_new_with_attributes("Тест", 
        gtk_cell_renderer_text_new(), "text", 1, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
    
    column = gtk_tree_view_column_new_with_attributes("Время (мс)", 
        gtk_cell_renderer_text_new(), "text", 2, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
    
    column = gtk_tree_view_column_new_with_attributes("Статус", 
        gtk_cell_renderer_text_new(), "text", 3, NULL);
    gtk_tree_view_append_column(GTK_TREE_VIEW(tree_view), column);
    
    // Заполнение списка
    GtkTreeIter iter;
    for (int i = 0; i < TEST_COUNT; i++) {
        gtk_list_store_append(store, &iter);
        char num[4];
        snprintf(num, sizeof(num), "%d", i + 1);
        gtk_list_store_set(store, &iter,
                          0, num,
                          1, test_names[i],
                          2, "--",
                          3, "Ожидание",
                          -1);
    }
    
    // Выбор строки
    GtkTreeSelection *select = gtk_tree_view_get_selection(GTK_TREE_VIEW(tree_view));
    gtk_tree_selection_set_mode(select, GTK_SELECTION_SINGLE);
    g_signal_connect(select, "changed", G_CALLBACK(on_test_selected), NULL);
    
    gtk_container_add(GTK_CONTAINER(scroll_window), tree_view);
    gtk_box_pack_start(GTK_BOX(vbox), scroll_window, TRUE, TRUE, 0);
    
    // Статус-бар
    GtkWidget *status_frame = gtk_frame_new("Статус выполнения");
    GtkWidget *status_vbox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 5);
    gtk_container_set_border_width(GTK_CONTAINER(status_vbox), 10);
    
    context.status_bar = gtk_label_new("Выберите тест из списка");
    gtk_label_set_selectable(GTK_LABEL(context.status_bar), TRUE);
    gtk_label_set_xalign(GTK_LABEL(context.status_bar), 0.0);
    gtk_label_set_yalign(GTK_LABEL(context.status_bar), 0.5);
    
    context.time_label = gtk_label_new("Время выполнения: -- мс");
    gtk_label_set_selectable(GTK_LABEL(context.time_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(context.time_label), 0.0);
    gtk_label_set_yalign(GTK_LABEL(context.time_label), 0.5);

    context.total_time_label = gtk_label_new("Суммарное время: -- мс");
    gtk_label_set_selectable(GTK_LABEL(context.total_time_label), TRUE);
    gtk_label_set_xalign(GTK_LABEL(context.total_time_label), 0.0);
    gtk_label_set_yalign(GTK_LABEL(context.total_time_label), 0.5);
    
    gtk_box_pack_start(GTK_BOX(status_vbox), context.status_bar, FALSE, FALSE, 0);
    gtk_box_pack_start(GTK_BOX(status_vbox), context.time_label, FALSE, FALSE, 0);
    gtk_container_add(GTK_CONTAINER(status_frame), status_vbox);
    gtk_box_pack_start(GTK_BOX(vbox), status_frame, FALSE, FALSE, 0);
    
    gtk_container_add(GTK_CONTAINER(context.window), vbox);
    
    // Сигналы
    g_signal_connect(context.window, "destroy", G_CALLBACK(gtk_main_quit), NULL);
    g_signal_connect(btn_exit, "clicked", G_CALLBACK(on_btn_exit_clicked), NULL);
    g_signal_connect(btn_run_all, "clicked", G_CALLBACK(on_btn_run_all_clicked), NULL);
    g_signal_connect(btn_run_selected, "clicked", G_CALLBACK(on_btn_run_selected_clicked), NULL);
    g_signal_connect(btn_clear, "clicked", G_CALLBACK(on_btn_clear_clicked), NULL);
    
    gtk_widget_show_all(context.window);
    
    return context.window;
}

// Запуск теста с таймингом
void gui_run_test(int test_index) {
    if (test_index < 0 || test_index >= context.test_count) return;
    
    TestResult *result = &context.results[test_index];
    
    clear_console();

    char msg[256];
    snprintf(msg, sizeof(msg), "⏳ Запуск теста %d: %s", test_index + 1, result->name);
    gtk_label_set_text(GTK_LABEL(context.status_bar), msg);
    gtk_widget_queue_draw(context.window);
    
    // Обновляем статус в списке
    GtkTreeModel *model = gtk_tree_view_get_model(GTK_TREE_VIEW(context.tree_view));
    GtkTreeIter iter;
    if (gtk_tree_model_get_iter_from_string(model, &iter, g_strdup_printf("%d", test_index))) {
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, 3, "⏳ Выполняется...", -1);
    }
    
    double start_time = get_time_ms();
    
    result->test_func();
    
    double end_time = get_time_ms();
    result->execution_time_ms = end_time - start_time;
    result->passed = TRUE;
    
    snprintf(msg, sizeof(msg), "Тест %d завершён: %s", test_index + 1, result->name);
    gtk_label_set_text(GTK_LABEL(context.status_bar), msg);
    
    char time_msg[128];
    snprintf(time_msg, sizeof(time_msg), "Время выполнения: %.2f мс", result->execution_time_ms);
    gtk_label_set_text(GTK_LABEL(context.time_label), time_msg);
    
    // Обновляем статус в списке
    if (gtk_tree_model_get_iter_from_string(model, &iter, g_strdup_printf("%d", test_index))) {
        char time_str[32];
        snprintf(time_str, sizeof(time_str), "%.2f", result->execution_time_ms);
        gtk_list_store_set(GTK_LIST_STORE(model), &iter, 
                          2, time_str, 
                          3, "✅ Успех", 
                          -1);
    }
    
    printf("Тест %d выполнен за %.2f мс\n", test_index + 1, result->execution_time_ms);
}

void gui_update_status(const char *message, double time_ms, gboolean success) {
    (void)success;
    if (context.status_bar) {
        gtk_label_set_text(GTK_LABEL(context.status_bar), message);
    }
    if (context.time_label) {
        char buf[128];
        snprintf(buf, sizeof(buf), "Время: %.2f мс", time_ms);
        gtk_label_set_text(GTK_LABEL(context.time_label), buf);
    }
}

void gui_get_test_list(const char ***names, void (***funcs)(void), int *count) {
    *names = test_names;
    *funcs = test_funcs;
    *count = TEST_COUNT;
}

void gui_run(void) {
    gtk_main();
}