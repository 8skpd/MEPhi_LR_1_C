#ifndef MATRIX_H
#define MATRIX_H

#include "matrix_types.h"

typedef struct {
    int rows;
    int cols;
    const TypeHandler *handler;
    void *data;
} Matrix;

// Управление памятью
Matrix* create_matrix(int rows, int cols, const TypeHandler *handler);
void free_matrix(Matrix *m);

// Доступ к элементам
void matrix_set(Matrix *m, int row, int col, const void *value);
void matrix_get(Matrix *m, int row, int col, void *out_value);

// Матричные операции
Matrix* matrix_add(const Matrix *A, const Matrix *B);
Matrix* matrix_multiply(const Matrix *A, const Matrix *B);
Matrix* matrix_transpose(const Matrix *A);

// Операции со строками
void matrix_add_linear_combination(Matrix *m, int target_row,
                                   const int *source_rows,
                                   const double *coeffs,
                                   int count);

// Утилиты
void matrix_print(const Matrix *m);
void matrix_fill(Matrix *m, const void *value);

#endif // MATRIX_H