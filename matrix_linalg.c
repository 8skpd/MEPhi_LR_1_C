#include "matrix_linalg.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

// Вспомогательные функции

int is_floating_point_type(const TypeHandler *handler) {
    // Проверяем по имени типа или размеру
    return (handler->element_size == sizeof(double) || 
            handler->element_size == sizeof(float));
}

// Вспомогательная: получить значение как double для вычислений
static double get_as_double(const TypeHandler *h, const void *data) {
    if (h->element_size == sizeof(double)) {
        return *(const double*)data;
    } else if (h->element_size == sizeof(float)) {
        return (double)*(const float*)data;
    } else if (h->element_size == sizeof(int)) {
        return (double)*(const int*)data;
    }
    return 0.0;
}

// Вспомогательная: установить значение из double
static void set_from_double(const TypeHandler *h, void *data, double value) {
    if (h->element_size == sizeof(double)) {
        *(double*)data = value;
    } else if (h->element_size == sizeof(float)) {
        *(float*)data = (float)value;
    } else if (h->element_size == sizeof(int)) {
        *(int*)data = (int)value;
    }
}

// Вспомогательная: получить элемент матрицы как double
static double get_elem_double(const Matrix *m, int row, int col) {
    if (row < 0 || row >= m->rows || col < 0 || col >= m->cols) return 0.0;
    const TypeHandler *h = m->handler;
    const void *ptr = (const char*)m->data + (row * m->cols + col) * h->element_size;
    return get_as_double(h, ptr);
}

// Вспомогательная: установить элемент матрицы из double
static void set_elem_double(Matrix *m, int row, int col, double value) {
    if (row < 0 || row >= m->rows || col < 0 || col >= m->cols) return;
    const TypeHandler *h = m->handler;
    void *ptr = (char*)m->data + (row * m->cols + col) * h->element_size;
    set_from_double(h, ptr, value);
}

// СЛАУ методом Гаусса (без выбора ведущего элемента)

int matrix_solve_gauss(const Matrix *A, const void *b, void *x) {
    if (A == NULL || b == NULL || x == NULL) return -1;
    if (A->rows != A->cols) return -1;  // Только квадратные матрицы
    if (!is_floating_point_type(A->handler)) {
        fprintf(stderr, "Gaussian elimination requires floating-point type\n");
        return -1;
    }
    
    int n = A->rows;
    const TypeHandler *h = A->handler;
    
    // Создаём расширенную матрицу [A|b] для прямого хода
    double **aug = malloc(n * sizeof(double*));
    for (int i = 0; i < n; i++) {
        aug[i] = calloc(n + 1, sizeof(double));
        for (int j = 0; j < n; j++) {
            aug[i][j] = get_elem_double(A, i, j);
        }
        // Копируем b в последний столбец
        const void *b_ptr = (const char*)b + i * h->element_size;
        aug[i][n] = get_as_double(h, b_ptr);
    }
    
    // Прямой ход Гаусса
    for (int k = 0; k < n - 1; k++) {
        // Проверка на нулевой ведущий элемент
        if (fabs(aug[k][k]) < 1e-12) {
            fprintf(stderr, "Zero pivot at row %d\n", k);
            for (int i = 0; i < n; i++) free(aug[i]);
            free(aug);
            return -1;
        }
        
        for (int i = k + 1; i < n; i++) {
            double factor = aug[i][k] / aug[k][k];
            for (int j = k; j <= n; j++) {
                aug[i][j] -= factor * aug[k][j];
            }
        }
    }
    
    // Обратный ход
    for (int i = n - 1; i >= 0; i--) {
        double sum = aug[i][n];
        for (int j = i + 1; j < n; j++) {
            sum -= aug[i][j] * get_as_double(h, (char*)x + j * h->element_size);
        }
        double val = sum / aug[i][i];
        set_from_double(h, (char*)x + i * h->element_size, val);
    }
    
    // Очистка
    for (int i = 0; i < n; i++) free(aug[i]);
    free(aug);
    
    return 0;
}

// Гаусс с выбором ведущего элемента

int matrix_solve_gauss_pivot(const Matrix *A, const void *b, void *x) {
    if (A == NULL || b == NULL || x == NULL) return -1;
    if (A->rows != A->cols) return -1;
    if (!is_floating_point_type(A->handler)) {
        fprintf(stderr, "Gaussian elimination requires floating-point type\n");
        return -1;
    }
    
    int n = A->rows;
    const TypeHandler *h = A->handler;
    
    // Расширенная матрица
    double **aug = malloc(n * sizeof(double*));
    int *perm = malloc(n * sizeof(int));  // Перестановки строк
    for (int i = 0; i < n; i++) {
        aug[i] = calloc(n + 1, sizeof(double));
        perm[i] = i;
        for (int j = 0; j < n; j++) {
            aug[i][j] = get_elem_double(A, i, j);
        }
        const void *b_ptr = (const char*)b + i * h->element_size;
        aug[i][n] = get_as_double(h, b_ptr);
    }
    
    // Прямой ход с выбором ведущего элемента по столбцу
    for (int k = 0; k < n - 1; k++) {
        // Поиск максимального по модулю элемента в столбце
        int max_row = k;
        double max_val = fabs(aug[k][k]);
        for (int i = k + 1; i < n; i++) {
            if (fabs(aug[i][k]) > max_val) {
                max_val = fabs(aug[i][k]);
                max_row = i;
            }
        }
        
        // Проверка на вырожденность
        if (max_val < 1e-12) {
            fprintf(stderr, "Matrix is singular at column %d\n", k);
            for (int i = 0; i < n; i++) free(aug[i]);
            free(aug);
            free(perm);
            return -1;
        }
        
        // Перестановка строк
        if (max_row != k) {
            double *tmp = aug[k];
            aug[k] = aug[max_row];
            aug[max_row] = tmp;
            int tmp_perm = perm[k];
            perm[k] = perm[max_row];
            perm[max_row] = tmp_perm;
        }
        
        // Исключение
        for (int i = k + 1; i < n; i++) {
            double factor = aug[i][k] / aug[k][k];
            for (int j = k; j <= n; j++) {
                aug[i][j] -= factor * aug[k][j];
            }
        }
    }
    
    // Обратный ход
    for (int i = n - 1; i >= 0; i--) {
        double sum = aug[i][n];
        for (int j = i + 1; j < n; j++) {
            sum -= aug[i][j] * get_as_double(h, (char*)x + j * h->element_size);
        }
        double val = sum / aug[i][i];
        set_from_double(h, (char*)x + i * h->element_size, val);
    }
    
    // Очистка
    for (int i = 0; i < n; i++) free(aug[i]);
    free(aug);
    free(perm);
    
    return 0;
}

// LU-РАЗЛОЖЕНИЕ (без перестановок, метод Дулиттла)

int matrix_lu_decompose(const Matrix *A, Matrix *L, Matrix *U) {
    if (A == NULL || L == NULL || U == NULL) return -1;
    if (A->rows != A->cols || L->rows != L->cols || U->rows != U->cols) return -1;
    if (A->rows != L->rows || A->cols != U->cols) return -1;
    if (!is_floating_point_type(A->handler)) return -1;
    
    int n = A->rows;
    const TypeHandler *h = A->handler;
    
    // Инициализация: L = 0, U = A (копия)
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double val = get_elem_double(A, i, j);
            set_elem_double(U, i, j, val);
            set_elem_double(L, i, j, (i == j) ? 1.0 : 0.0);
        }
    }
    
    // Разложение: для каждого столбца
    for (int k = 0; k < n - 1; k++) {
        if (fabs(get_elem_double(U, k, k)) < 1e-12) {
            fprintf(stderr, "Zero pivot in LU decomposition\n");
            return -1;
        }
        
        for (int i = k + 1; i < n; i++) {
            double factor = get_elem_double(U, i, k) / get_elem_double(U, k, k);
            set_elem_double(L, i, k, factor);
            
            for (int j = k; j < n; j++) {
                double u_val = get_elem_double(U, i, j) - factor * get_elem_double(U, k, j);
                set_elem_double(U, i, j, u_val);
            }
        }
    }
    
    return 0;
}

// ПРЯМАЯ ПОДСТАНОВКА (L×x = b, L — нижняя треугольная с единицами на диагонали)

int matrix_solve_lower_triangular(const Matrix *L, const void *b, void *x) {
    if (L == NULL || b == NULL || x == NULL) return -1;
    if (L->rows != L->cols) return -1;
    if (!is_floating_point_type(L->handler)) return -1;
    
    int n = L->rows;
    const TypeHandler *h = L->handler;
    
    for (int i = 0; i < n; i++) {
        double sum = get_as_double(h, (const char*)b + i * h->element_size);
        for (int j = 0; j < i; j++) {
            double l_val = get_elem_double(L, i, j);
            double x_val = get_as_double(h, (char*)x + j * h->element_size);
            sum -= l_val * x_val;
        }
        // Диагональ L равна 1, делить не нужно
        set_from_double(h, (char*)x + i * h->element_size, sum);
    }
    
    return 0;
}

// ОБРАТНАЯ ПОДСТАНОВКА (U×x = b, U — верхняя треугольная)

int matrix_solve_upper_triangular(const Matrix *U, const void *b, void *x) {
    if (U == NULL || b == NULL || x == NULL) return -1;
    if (U->rows != U->cols) return -1;
    if (!is_floating_point_type(U->handler)) return -1;
    
    int n = U->rows;
    const TypeHandler *h = U->handler;
    
    for (int i = n - 1; i >= 0; i--) {
        double sum = get_as_double(h, (const char*)b + i * h->element_size);
        for (int j = i + 1; j < n; j++) {
            double u_val = get_elem_double(U, i, j);
            double x_val = get_as_double(h, (char*)x + j * h->element_size);
            sum -= u_val * x_val;
        }
        double diag = get_elem_double(U, i, i);
        if (fabs(diag) < 1e-12) {
            fprintf(stderr, "Zero diagonal in upper triangular solve\n");
            return -1;
        }
        set_from_double(h, (char*)x + i * h->element_size, sum / diag);
    }
    
    return 0;
}

// РЕШЕНИЕ СЛАУ ЧЕРЕЗ LU-РАЗЛОЖЕНИЕ

int matrix_solve_lu(const Matrix *L, const Matrix *U, const void *b, void *x) {
    if (L == NULL || U == NULL || b == NULL || x == NULL) return -1;
    
    // Временный вектор y для Ly = b
    const TypeHandler *h = L->handler;
    void *y = malloc(L->rows * h->element_size);
    if (y == NULL) return -1;
    
    // 1. Прямая подстановка: Ly = b
    if (matrix_solve_lower_triangular(L, b, y) != 0) {
        free(y);
        return -1;
    }
    
    // 2. Обратная подстановка: Ux = y
    int result = matrix_solve_upper_triangular(U, y, x);
    
    free(y);
    return result;
}

// QR-РАЗЛОЖЕНИЕ (метод Грама-Шмидта)

int matrix_qr_decompose(const Matrix *A, Matrix *Q, Matrix *R) {
    if (A == NULL || Q == NULL || R == NULL) return -1;
    if (A->rows < A->cols) return -1;  // m >= n
    if (Q->rows != A->rows || Q->cols != A->cols) return -1;
    if (R->rows != A->cols || R->cols != A->cols) return -1;
    if (!is_floating_point_type(A->handler)) return -1;
    
    int m = A->rows;
    int n = A->cols;
    const TypeHandler *h = A->handler;
    
    // Копируем A во временный массив для вычислений
    double **V = malloc(n * sizeof(double*));
    for (int j = 0; j < n; j++) {
        V[j] = malloc(m * sizeof(double));
        for (int i = 0; i < m; i++) {
            V[j][i] = get_elem_double(A, i, j);
        }
    }
    
    // Инициализация R нулями
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            set_elem_double(R, i, j, 0.0);
    
    // Ортогонализация Грама-Шмидта
    for (int j = 0; j < n; j++) {
        // Копируем j-й столбец A во временный вектор
        double *v = malloc(m * sizeof(double));
        for (int i = 0; i < m; i++) v[i] = V[j][i];
        
        // Вычитаем проекции на предыдущие ортогональные векторы
        for (int k = 0; k < j; k++) {
            // R[k][j] = Q[:,k]ᵀ × A[:,j]
            double r_kj = 0.0;
            for (int i = 0; i < m; i++) {
                double q_ik = get_elem_double(Q, i, k);
                r_kj += q_ik * V[j][i];
            }
            set_elem_double(R, k, j, r_kj);
            
            // v = v - R[k][j] × Q[:,k]
            for (int i = 0; i < m; i++) {
                double q_ik = get_elem_double(Q, i, k);
                v[i] -= r_kj * q_ik;
            }
        }
        
        // R[j][j] = ||v||
        double r_jj = 0.0;
        for (int i = 0; i < m; i++) r_jj += v[i] * v[i];
        r_jj = sqrt(r_jj);
        set_elem_double(R, j, j, r_jj);
        
        if (r_jj < 1e-12) {
            fprintf(stderr, "Linear dependence in QR decomposition\n");
            free(v);
            for (int j = 0; j < n; j++) free(V[j]);
            free(V);
            return -1;
        }
        
        // Q[:,j] = v / R[j][j]
        for (int i = 0; i < m; i++) {
            set_elem_double(Q, i, j, v[i] / r_jj);
        }
        
        free(v);
    }
    
    // Очистка
    for (int j = 0; j < n; j++) free(V[j]);
    free(V);
    
    return 0;
}

// РЕШЕНИЕ СЛАУ ЧЕРЕЗ QR-РАЗЛОЖЕНИЕ

int matrix_solve_qr(const Matrix *Q, const Matrix *R, const void *b, void *x) {
    if (Q == NULL || R == NULL || b == NULL || x == NULL) return -1;
    if (!is_floating_point_type(Q->handler)) return -1;
    
    int m = Q->rows;
    int n = Q->cols;
    const TypeHandler *h = Q->handler;
    
    // 1. Вычисляем c = Qᵀ × b
    double *c = calloc(n, sizeof(double));
    if (c == NULL) return -1;
    
    for (int j = 0; j < n; j++) {
        for (int i = 0; i < m; i++) {
            double q_ij = get_elem_double(Q, i, j);
            double b_i = get_as_double(h, (const char*)b + i * h->element_size);
            c[j] += q_ij * b_i;
        }
    }
    
    // 2. Решаем R × x = c (обратная подстановка)
    // Создаём временный вектор для c в нужном формате
    void *c_typed = malloc(n * h->element_size);
    if (c_typed == NULL) { free(c); return -1; }
    
    for (int i = 0; i < n; i++) {
        set_from_double(h, (char*)c_typed + i * h->element_size, c[i]);
    }
    
    int result = matrix_solve_upper_triangular(R, c_typed, x);
    
    free(c);
    free(c_typed);
    return result;
}