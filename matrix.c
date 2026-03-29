#include "matrix.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Matrix* create_matrix(int rows, int cols, const TypeHandler *handler) {
    if (rows <= 0 || cols <= 0) {
        fprintf(stderr, "Неверные размеры матрицы\n");
        return NULL;
    }

    if (handler == NULL) {
        fprintf(stderr, "Передан невалидный обработчик типа\n");
        return NULL;
    }

    Matrix *m = (Matrix*)malloc(sizeof(Matrix));
    if (m == NULL) {
        fprintf(stderr, "Ошибка выделения памяти под метаданные матрицы\n");
        return NULL;
    }

    size_t data_size = rows * cols * handler->element_size;
    m->data = malloc(data_size);
    if (m->data == NULL) {
        fprintf(stderr, "Ошибка выделения памяти под данные матрицы\n");
        free(m);
        return NULL;
    }

    m->rows = rows;
    m->cols = cols;
    m->handler = handler;
    memset(m->data, 0, data_size);

    return m;
}

void free_matrix(Matrix *m) {
    if (m != NULL) {
        if (m->data != NULL) {
            free(m->data);
        }
        free(m);
    }
}

void matrix_set(Matrix *m, int row, int col, const void *value) {
    if (m == NULL || value == NULL) return;  // ← Добавлена проверка
    if (row < 0 || row >= m->rows || col < 0 || col >= m->cols) return;
    m->handler->set_element(m->data, row * m->cols + col, value);
}

void matrix_get(Matrix *m, int row, int col, void *out_value) {
    if (m == NULL || out_value == NULL) return;  // ← Добавлена проверка
    if (row < 0 || row >= m->rows || col < 0 || col >= m->cols) return;
    m->handler->get_element(m->data, row * m->cols + col, out_value);
}

Matrix* matrix_add(const Matrix *A, const Matrix *B) {
    // ✅ ПРОВЕРКА NULL ПЕРЕД ДЕРЕФЕРЕНЦИЕЙ
    if (A == NULL || B == NULL) {
        fprintf(stderr, "Попытка сложения с NULL матрицей\n");
        return NULL;
    }
    
    if (A->rows != B->rows || A->cols != B->cols || A->handler != B->handler) {
        fprintf(stderr, "Разный размер или тип данных матриц\n");
        return NULL;
    }

    Matrix *C = create_matrix(A->rows, A->cols, A->handler);
    if (C == NULL) return NULL;
    
    const TypeHandler *h = A->handler;
    int total = A->rows * A->cols;

    for (int i = 0; i < total; i++) {
        void *ptrC = (char*)C->data + i * h->element_size;
        const void *ptrA = (const char*)A->data + i * h->element_size;
        const void *ptrB = (const char*)B->data + i * h->element_size;
        h->add_elements(ptrC, ptrA, ptrB);
    }
    return C;
}

Matrix* matrix_multiply(const Matrix *A, const Matrix *B) {
    if (A == NULL || B == NULL) {
        fprintf(stderr, "Попытка умножения с NULL матрицей\n");
        return NULL;
    }
    
    if (A->cols != B->rows || A->handler != B->handler) {
        fprintf(stderr, "Количество столбцов первой матрицы не совпадает с количеством строк второй или разный тип данных матриц\n");
        return NULL;
    }

    Matrix *C = create_matrix(A->rows, B->cols, A->handler);
    if (C == NULL) return NULL;
    
    const TypeHandler *h = A->handler;

    const void *ptrA;
    const void *ptrB;
    void *ptrC;
    char tempA[h->element_size];
    char tempB[h->element_size];
    char tempProd[h->element_size];
    char tempResult[h->element_size];

    int i, j, k;

    for (i = 0; i < A->rows; i++) {
        for (j = 0; j < B->cols; j++) {
            h->set_zero(C->data, i * C->cols + j);
            ptrC = (char*)C->data + (i * C->cols + j) * h->element_size;
            memcpy(tempProd, ptrC, h->element_size);

            for (k = 0; k < A->cols; k++) {
                ptrA = (const char*)A->data + (i * A->cols + k) * h->element_size;
                ptrB = (const char*)B->data + (k * B->cols + j) * h->element_size;
                
                memcpy(tempA, ptrA, h->element_size);
                memcpy(tempB, ptrB, h->element_size);
                
                h->multiply_elements(tempResult, tempA, tempB);
                h->add_elements(tempProd, tempProd, tempResult);
            }
            
            memcpy(ptrC, tempProd, h->element_size);
        }
    }
    return C;
}

Matrix* matrix_transpose(const Matrix *A) {
    if (A == NULL) {
        fprintf(stderr, "Попытка транспонирования NULL матрицы\n");
        return NULL;
    }
    
    Matrix *T = create_matrix(A->cols, A->rows, A->handler);
    if (T == NULL) return NULL;
    
    const TypeHandler *h = A->handler;

    const void *src;
    void *dst;

    for (int i = 0; i < A->rows; i++) {
        for (int j = 0; j < A->cols; j++) {
            src = (const char*)A->data + (i * A->cols + j) * h->element_size;
            dst = (char*)T->data + (j * T->cols + i) * h->element_size;
            memcpy(dst, src, h->element_size);
        }
    }
    return T;
}

void matrix_add_linear_combination(Matrix *m, int target_row,
                                   const int *source_rows,
                                   const double *coeffs,
                                   int count) {
    if (m == NULL || source_rows == NULL || coeffs == NULL) {
        fprintf(stderr, "Попытка линейной комбинации с NULL параметрами\n");
        return;
    }
    
    if (count <= 0) return;  // Нечего делать
    if (target_row < 0 || target_row >= m->rows) return;  // Неверная строка

    const TypeHandler *h = m->handler;
    char temp[h->element_size];
    char scaled[h->element_size];
    
    char *target_ptr = (char*)m->data + target_row * m->cols * h->element_size;

    const void *src;
    void *current;

    for (int col = 0; col < m->cols; col++) {
        current = (char*)target_ptr + col * h->element_size;
        memcpy(temp, current, h->element_size);

        for (int k = 0; k < count; k++) {
            // Проверка на валидность исходной строки
            if (source_rows[k] < 0 || source_rows[k] >= m->rows) continue;
            
            src = (const char*)m->data + 
                             (source_rows[k] * m->cols + col) * h->element_size;
            
            memcpy(scaled, src, h->element_size);
            h->scale_element(scaled, scaled, coeffs[k]);
            h->add_elements(temp, temp, scaled);
        }
        
        memcpy(current, temp, h->element_size);
    }
}

void matrix_print(const Matrix *m) {
    if (m == NULL) {
        fprintf(stderr, "Попытка печати NULL матрицы\n");
        return;
    }
    
    const TypeHandler *h = m->handler;
    printf("Матрица (%dx%d, Тип: %s):\n", m->rows, m->cols, h->type_name);
    
    const void *ptr;

    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            ptr = (const char*)m->data + (i * m->cols + j) * h->element_size;
            h->print_element(ptr);
            printf(" ");
        }
        printf("\n");
    }
    printf("\n");
}

void matrix_fill(Matrix *m, const void *value) {
    if (m == NULL || value == NULL) {
        fprintf(stderr, "Попытка заполнения NULL матрицы или значения\n");
        return;
    }
    
    const TypeHandler *h = m->handler;
    int total = m->rows * m->cols;
    for (int i = 0; i < total; i++) {
        h->set_element(m->data, i, value);
    }
}