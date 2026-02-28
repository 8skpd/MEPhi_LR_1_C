#include "matrix.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

Matrix* create_matrix(int rows, int cols, const TypeHandler *handler) {
    // Проверка входных данных
    if (rows <= 0 || cols <= 0) {
        fprintf(stderr, "Неверные размеры матрицы\n");
        return NULL;
    }

    if (handler == NULL) {
        fprintf(stderr, "Передан невалидный обработчик типа\n");
        return NULL;
    }

    // Выделение памяти под структуру
    Matrix *m = (Matrix*)malloc(sizeof(Matrix));
    if (m == NULL) {
        fprintf(stderr, "Ошибка выделения памяти под метаданные матрицы\n");
        return NULL;
    }

    // Выделение памяти под данные
    size_t data_size = rows * cols * handler->element_size;
    m->data = malloc(data_size);
    if (m->data == NULL) {
        fprintf(stderr, "Ошибка выделения памяти под данные матрицы\n");
        free(m);
        return NULL;
    }

    // Инициализация
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
    if (row < 0 || row >= m->rows || col < 0 || col >= m->cols) return;
    const TypeHandler *h = m->handler;
    h->set_element(m->data, row * m->cols + col, value);
}

void matrix_get(Matrix *m, int row, int col, void *out_value) {
    if (row < 0 || row >= m->rows || col < 0 || col >= m->cols) return;
    const TypeHandler *h = m->handler;
    h->get_element(m->data, row * m->cols + col, out_value);
}

Matrix* matrix_add(const Matrix *A, const Matrix *B) {
    if (A->rows != B->rows || A->cols != B->cols || A->handler != B->handler) {
        fprintf(stderr, "Разный размер или тип данных матриц\n");
        return NULL;
    }

    Matrix *C = create_matrix(A->rows, A->cols, A->handler);
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
    if (A->cols != B->rows || A->handler != B->handler) {
        fprintf(stderr, "Количество столбцов первой матрицы не совпадает с количеством строк второй или разный тип данных матриц\n");
        return NULL;
    }

    Matrix *C = create_matrix(A->rows, B->cols, A->handler);
    const TypeHandler *h = A->handler;

    // Указатели
    const void *ptrA;
    const void *ptrB;
    void *ptrC;
    // Буферы
    char tempA[h->element_size];
    char tempB[h->element_size];
    char tempProd[h->element_size];
    char tempResult[h->element_size];

    int i, j, k;

    for (i = 0; i < A->rows; i++) {
        for (j = 0; j < B->cols; j++) {

            h->set_zero(C->data, i * C->cols + j);
            
            // Получаем указатель на результат один раз для внутреннего цикла
            ptrC = (char*)C->data + (i * C->cols + j) * h->element_size;
            memcpy(tempProd, ptrC, h->element_size); // Считываем текущий ноль

            for (k = 0; k < A->cols; k++) {
                ptrA = (const char*)A->data + (i * A->cols + k) * h->element_size;
                ptrB = (const char*)B->data + (k * B->cols + j) * h->element_size;
                
                // Копируем данные в буферы
                memcpy(tempA, ptrA, h->element_size);
                memcpy(tempB, ptrB, h->element_size);
                
                h->multiply_elements(tempResult, tempA, tempB);
                h->add_elements(tempProd, tempProd, tempResult);
            }
            
            // Записываем финальный результат обратно в матрицу C
            memcpy(ptrC, tempProd, h->element_size);
        }
    }
    return C;
}

Matrix* matrix_transpose(const Matrix *A) {
    Matrix *T = create_matrix(A->cols, A->rows, A->handler);
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
    const TypeHandler *h = m->handler;
    
    char temp[h->element_size];
    char scaled[h->element_size];
    
    char *target_ptr = (char*)m->data + target_row * m->cols * h->element_size;

    const void *src;
    void *current;

    for (int col = 0; col < m->cols; col++) {
        current = (char*)target_ptr + col * h->element_size;
        
        // Копируем текущее значение целевой ячейки в буфер
        memcpy(temp, current, h->element_size);

        for (int k = 0; k < count; k++) {
            src = (const char*)m->data + 
                             (source_rows[k] * m->cols + col) * h->element_size;
            
            // Копируем значение исходной строки в буфер
            memcpy(scaled, src, h->element_size);
            
            // Масштабируем: scaled = scaled * coeffs[k]
            h->scale_element(scaled, scaled, coeffs[k]);
            
            // Прибавляем: temp = temp + scaled
            h->add_elements(temp, temp, scaled);
        }
        
        // Записываем результат обратно в матрицу
        memcpy(current, temp, h->element_size);
    }
}

void matrix_print(const Matrix *m) {
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
    const TypeHandler *h = m->handler;
    int total = m->rows * m->cols;
    for (int i = 0; i < total; i++) {
        h->set_element(m->data, i, value);
    }
}