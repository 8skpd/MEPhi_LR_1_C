#ifndef MATRIX_EDGE_TESTS_H
#define MATRIX_EDGE_TESTS_H

// Тесты граничных значений

// Тесты создания матриц
int test_create_zero_rows(void);
int test_create_zero_cols(void);
int test_create_negative_dimensions(void);
int test_create_very_large_matrix(void);
int test_create_1x1_matrix(void);

// Тесты доступа к элементам
int test_set_out_of_bounds(void);
int test_get_out_of_bounds(void);
int test_set_get_null_value(void);

// Тесты арифметических операций
int test_add_dimension_mismatch(void);
int test_add_type_mismatch(void);
int test_add_null_matrices(void);
int test_multiply_dimension_mismatch(void);
int test_multiply_type_mismatch(void);
int test_multiply_null_matrices(void);
int test_transpose_null_matrix(void);

// Тесты линейной комбинации
int test_linear_combination_invalid_target_row(void);
int test_linear_combination_invalid_source_row(void);
int test_linear_combination_zero_count(void);
int test_linear_combination_null_params(void);

// Тесты заполнения и печати
int test_fill_null_matrix(void);
int test_fill_null_value(void);
int test_print_null_matrix(void);

// Тесты освобождения памяти
int test_free_null_matrix(void);
int test_free_already_freed(void);  // Проверка на double-free

// Тесты переполнения и точности
int test_int_overflow_addition(void);
int test_double_precision_loss(void);

// Вспомогательные функции
void print_test_header(const char *name);
void print_test_result(const char *name, int passed, const char *message);

#endif // MATRIX_EDGE_TESTS_H