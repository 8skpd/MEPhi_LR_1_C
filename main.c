#include "matrix.h"
#include "matrix_tests.h"
#include <stdio.h>
#include <stdlib.h>
#include "utils.h"

extern const TypeHandler* get_type_handler_int(void);
extern const TypeHandler* get_type_handler_double(void);

void print_menu(void) {
    printf("\n========================================\n");
    printf("       MATRIX LIBRARY TEST MENU\n");
    printf("========================================\n");
    printf("1: Создание и вывод матриц (DOUBLE)\n");
    printf("2: Умножение матриц (INT)\n");
    printf("3: Сложение матриц (DOUBLE)\n");
    printf("4: Транспонирование матрицы (INT)\n");
    printf("5: Линейная комбинация строк (DOUBLE)\n");
    printf("6: Заполнение и операции (INT)\n");
    printf("7: Тест большой матрицы (100x100)\n");
    printf("8: Тест обработки ошибок\n");
    printf("9: Последовательность всех операций\n");
    printf("0: Выход\n");
    printf("========================================\n");
    printf("Выберите тест (0-9): ");
}

int main(void) {
    int choice;
    
    printf("╔════════════════════════════════════════╗\n");
    printf("║   СДЕЛАЛ ВЕЛИКАНОВ МИХАИЛ Б25-507      ║\n");
    printf("╚════════════════════════════════════════╝\n");
    
    while (1) {
        print_menu();
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("Ошибка ввода! Введите число от 0 до 9.\n");
            continue;
        }

        if (choice >= 1 && choice <= 9) {
            clear_console();
        }

        switch (choice) {
            case 1: test_create_double_matrices(); break;
            case 2: test_multiply_int_matrices(); break;
            case 3: test_add_double_matrices(); break;
            case 4: test_transpose_int_matrix(); break;
            case 5: test_linear_combination_double(); break;
            case 6: test_fill_and_operations(); break;
            case 7: test_large_matrix(); break;
            case 8: test_error_handling(); break;
            case 9: test_all_operations_sequence(); break;
            case 0:
                printf("\nЗавершение работы программы.\n");
                return 0;
            default:
                printf("Неверный выбор! Попробуйте снова.\n");
        }
        
        printf("\nНажмите Enter для продолжения...");
        while (getchar() != '\n');
        getchar();

        clear_console();

    }
}