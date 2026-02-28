#include "matrix.h"
#include "matrix_tests.h"
#include <stdio.h>
#include <stdlib.h>


// Внешние функции для получения обработчиков
extern const TypeHandler* get_type_handler_int(void);
extern const TypeHandler* get_type_handler_double(void);

// === ТЕСТ 1: Создание и вывод дабловых матриц ===
void test_create_double_matrices(void) {
    printf("\n--- ТЕСТ 1: Создание и вывод матриц (DOUBLE) ---\n");
    
    Matrix *m1 = create_matrix(3, 3, get_type_handler_double());
    if (m1 == NULL) { printf("❌ Ошибка выделения памяти\n"); return; }
    
    double values[3][3] = {
        {1.5, 2.5, 3.5},
        {4.5, 5.5, 6.5},
        {7.5, 8.5, 9.5}
    };
    
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            matrix_set(m1, i, j, &values[i][j]);
        }
    }
    
    printf("Матрица 3x3 (DOUBLE):\n");
    matrix_print(m1);
    
    double val = 0.0;
    matrix_get(m1, 1, 1, &val);
    printf("Элемент [1][1] = %.2f (ожидается 5.50)\n", val);
    
    free_matrix(m1);
    printf("✅ Тест 1 завершен\n");
}

// === ТЕСТ 2: Умножение интовых матриц ===
void test_multiply_int_matrices(void) {
    printf("\n--- ТЕСТ 2: Умножение матриц (INT) ---\n");
    
    Matrix *a = create_matrix(2, 3, get_type_handler_int());
    Matrix *b = create_matrix(3, 2, get_type_handler_int());
    if (!a || !b) { printf("❌ Ошибка выделения памяти\n"); return; }
    
    int vals_a[2][3] = {{1, 2, 3}, {4, 5, 6}};
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 3; j++)
            matrix_set(a, i, j, &vals_a[i][j]);
    
    int vals_b[3][2] = {{7, 8}, {9, 10}, {11, 12}};
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 2; j++)
            matrix_set(b, i, j, &vals_b[i][j]);
    
    printf("Матрица A (2x3):\n");
    matrix_print(a);
    
    printf("Матрица B (3x2):\n");
    matrix_print(b);
    
    Matrix *c = matrix_multiply(a, b);
    if (c == NULL) { printf("❌ Ошибка умножения\n"); free_matrix(a); free_matrix(b); return; }
    
    printf("Результат A * B (2x2):\n");
    matrix_print(c);
    printf("Ожидаемый результат: [58, 64], [139, 154]\n");
    
    free_matrix(a);
    free_matrix(b);
    free_matrix(c);
    printf("✅ Тест 2 завершен\n");
}

// === ТЕСТ 3: Сложение матриц (DOUBLE) ===
void test_add_double_matrices(void) {
    printf("\n--- ТЕСТ 3: Сложение матриц (DOUBLE) ---\n");
    
    Matrix *a = create_matrix(2, 2, get_type_handler_double());
    Matrix *b = create_matrix(2, 2, get_type_handler_double());
    if (!a || !b) { printf("❌ Ошибка выделения памяти\n"); return; }
    
    double vals_a[2][2] = {{1.0, 2.0}, {3.0, 4.0}};
    double vals_b[2][2] = {{0.5, 0.5}, {0.5, 0.5}};
    
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            matrix_set(a, i, j, &vals_a[i][j]);
            matrix_set(b, i, j, &vals_b[i][j]);
        }
    }
    
    printf("Матрица A:\n");
    matrix_print(a);
    printf("Матрица B:\n");
    matrix_print(b);
    
    Matrix *c = matrix_add(a, b);
    if (c == NULL) { printf("❌ Ошибка сложения\n"); free_matrix(a); free_matrix(b); return; }
    
    printf("Результат A + B:\n");
    matrix_print(c);
    
    free_matrix(a);
    free_matrix(b);
    free_matrix(c);
    printf("✅ Тест 3 завершен\n");
}

// === ТЕСТ 4: Транспонирование матрицы (INT) ===
void test_transpose_int_matrix(void) {
    printf("\n--- ТЕСТ 4: Транспонирование матрицы (INT) ---\n");
    
    Matrix *m = create_matrix(2, 3, get_type_handler_int());
    if (m == NULL) { printf("❌ Ошибка выделения памяти\n"); return; }
    
    int vals[2][3] = {{1, 2, 3}, {4, 5, 6}};
    for (int i = 0; i < 2; i++)
        for (int j = 0; j < 3; j++)
            matrix_set(m, i, j, &vals[i][j]);
    
    printf("Исходная матрица (2x3):\n");
    matrix_print(m);
    
    Matrix *t = matrix_transpose(m);
    if (t == NULL) { printf("❌ Ошибка транспонирования\n"); free_matrix(m); return; }
    
    printf("Транспонированная матрица (3x2):\n");
    matrix_print(t);
    
    free_matrix(m);
    free_matrix(t);
    printf("✅ Тест 4 завершен\n");
}

// === ТЕСТ 5: Линейная комбинация строк (DOUBLE) ===
void test_linear_combination_double(void) {
    printf("\n--- ТЕСТ 5: Линейная комбинация строк (DOUBLE) ---\n");
    
    Matrix *m = create_matrix(3, 3, get_type_handler_double());
    if (m == NULL) { printf("❌ Ошибка выделения памяти\n"); return; }
    
    double val = 1.0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            matrix_set(m, i, j, &val);
            val++;
        }
    }
    
    printf("Исходная матрица:\n");
    matrix_print(m);
    
    int src_row = 0;
    double coeff = 2.0;
    printf("Операция: Row[2] = Row[2] + 2.0 * Row[0]\n");
    matrix_add_linear_combination(m, 2, &src_row, &coeff, 1);
    
    printf("Результат:\n");
    matrix_print(m);
    
    double check = 0.0;
    matrix_get(m, 2, 0, &check);
    printf("Элемент [2][0] = %.2f (ожидается 9.00: 7 + 2*1)\n", check);
    
    free_matrix(m);
    printf("✅ Тест 5 завершен\n");
}

// === ТЕСТ 6: Заполнение и операции (INT) ===
void test_fill_and_operations(void) {
    printf("\n--- ТЕСТ 6: Заполнение и операции (INT) ---\n");
    
    Matrix *m = create_matrix(3, 3, get_type_handler_int());
    if (m == NULL) { printf("❌ Ошибка выделения памяти\n"); return; }
    
    int fill_val = 5;
    matrix_fill(m, &fill_val);
    
    printf("Матрица после fill(5):\n");
    matrix_print(m);
    
    Matrix *sum = matrix_add(m, m);
    if (sum != NULL) {
        printf("Матрица + Матрица (должно быть 10):\n");
        matrix_print(sum);
        free_matrix(sum);
    }
    
    free_matrix(m);
    printf("✅ Тест 6 завершен\n");
}

// === ТЕСТ 7: Тест большой матрицы (100x100) ===
void test_large_matrix(void) {
    printf("\n--- ТЕСТ 7: Тест большой матрицы (100x100) ---\n");
    
    printf("Выделение памяти для матрицы 100x100 (DOUBLE)...\n");
    Matrix *m = create_matrix(100, 100, get_type_handler_double());
    if (m == NULL) {
        printf("❌ Не удалось выделить память\n");
        return;
    }
    
    printf("✅ Матрица успешно создана\n");
    printf("Размеры: %d x %d\n", m->rows, m->cols);
    
    double test_val = 3.14;
    matrix_set(m, 50, 50, &test_val);
    
    double retrieved = 0.0;
    matrix_get(m, 50, 50, &retrieved);
    printf("Элемент [50][50] = %.2f (ожидается 3.14)\n", retrieved);
    
    free_matrix(m);
    printf("✅ Память освобождена. Тест 7 завершен\n");
}

// === ТЕСТ 8: Тест обработки ошибок ===
void test_error_handling(void) {
    printf("\n--- ТЕСТ 8: Тест обработки ошибок ---\n");
    
    printf("1. Создание матрицы 0x5 (ошибка)...\n");
    Matrix *m1 = create_matrix(0, 5, get_type_handler_int());
    printf("   Результат: %s\n", (m1 == NULL) ? "✅ NULL (верно)" : "❌ Не NULL (ошибка)");
    
    printf("2. Сложение матриц 2x3 и 3x2 (ошибка)...\n");
    Matrix *a = create_matrix(2, 3, get_type_handler_int());
    Matrix *b = create_matrix(3, 2, get_type_handler_int());
    Matrix *sum = matrix_add(a, b);
    printf("   Результат: %s\n", (sum == NULL) ? "✅ NULL (верно)" : "❌ Не NULL (ошибка)");
    free_matrix(a);
    free_matrix(b);
    
    printf("3. Умножение матриц 2x2 и 3x3 (ошибка)...\n");
    Matrix *c = create_matrix(2, 2, get_type_handler_int());
    Matrix *d = create_matrix(3, 3, get_type_handler_int());
    Matrix *mul = matrix_multiply(c, d);
    printf("   Результат: %s\n", (mul == NULL) ? "✅ NULL (верно)" : "❌ Не NULL (ошибка)");
    free_matrix(c);
    free_matrix(d);
    
    printf("✅ Тест 8 завершен\n");
}

// === ТЕСТ 9: Последовательность всех операций ===
void test_all_operations_sequence(void) {
    printf("\n--- ТЕСТ 9: Последовательность всех операций ---\n");
    
    Matrix *m = create_matrix(2, 2, get_type_handler_int());
    if (m == NULL) { printf("❌ Ошибка выделения памяти\n"); return; }
    
    int val = 1;
    matrix_fill(m, &val);
    printf("1. После fill(1):\n");
    matrix_print(m);
    
    val = 10;
    matrix_set(m, 0, 0, &val);
    printf("2. После set[0][0] = 10:\n");
    matrix_print(m);
    
    Matrix *sum = matrix_add(m, m);
    printf("3. После M + M:\n");
    matrix_print(sum);
    
    Matrix *trans = matrix_transpose(sum);
    printf("4. После transpose:\n");
    matrix_print(trans);
    
    int src = 0;
    double coeff = 0.5;
    matrix_add_linear_combination(trans, 1, &src, &coeff, 1);
    printf("5. После Row[1] += 0.5 * Row[0]:\n");
    matrix_print(trans);
    
    free_matrix(m);
    free_matrix(sum);
    free_matrix(trans);
    printf("✅ Тест 9 завершен\n");
}