#include "matrix_gui.h"
#include "utils.h"
#include <stdio.h>

int main(int argc, char *argv[]) {

    clear_console();
    
    printf("╔════════════════════════════════════════╗\n");
    printf("║     ПРОГРАММА ТЕСТОВ ДЛЯ МАТРИЦЫ       ║\n");
    printf("╚════════════════════════════════════════╝\n\n");
    
    // Инициализация GUI
    gui_init(&argc, &argv);
    
    // Создание главного окна
    gui_create_main_window();
    
    printf("🖥  Запуск графического интерфейса...\n");
    printf("   Закройте окно для завершения программы.\n\n");
    
    // Запуск главного цикла GTK
    gui_run();
    
    // Очистка памяти
    GuiContext *ctx = gui_get_context();
    if (ctx->results) {
        free(ctx->results);
    }
    
    printf("✅ Программа завершена.\n");
    return 0;
}