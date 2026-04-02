#include "matrix.h"
#include "matrix_linalg.h"
#include "utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <time.h>
#include <sys/time.h>

extern const TypeHandler* get_type_handler_by_name(const char *name);

static double get_elem_double(const Matrix *m, int row, int col);
static void set_elem_double(Matrix *m, int row, int col, double value);
static void fill_matrix_random(Matrix *m, double min, double max);
static Matrix* create_hilbert_matrix(int n);
static double vector_norm(const double *v, int n);
static double compute_residual(const Matrix *A, const double *x, const double *b, int n);
static double relative_error(const double *x_approx, const double *x_exact, int n);


// Таймер (микросекунды)
static double get_time_us(void) {
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1e6 + tv.tv_usec;
}

// Случайное число из [-1, 1]
static double rand_uniform(void) {
    return 2.0 * ((double)rand() / RAND_MAX) - 1.0;
}

// Получить элемент как double
static double get_elem_double(const Matrix *m, int row, int col) {
    if (!m || row < 0 || row >= m->rows || col < 0 || col >= m->cols) return 0.0;
    const TypeHandler *h = m->handler;
    const void *ptr = (const char*)m->data + (row * m->cols + col) * h->element_size;
    if (h->element_size == sizeof(double)) return *(const double*)ptr;
    if (h->element_size == sizeof(float)) return (double)*(const float*)ptr;
    if (h->element_size == sizeof(int)) return (double)*(const int*)ptr;
    return 0.0;
}

// Установить элемент из double
static void set_elem_double(Matrix *m, int row, int col, double value) {
    if (!m || row < 0 || row >= m->rows || col < 0 || col >= m->cols) return;
    const TypeHandler *h = m->handler;
    void *ptr = (char*)m->data + (row * m->cols + col) * h->element_size;
    if (h->element_size == sizeof(double)) *(double*)ptr = value;
    else if (h->element_size == sizeof(float)) *(float*)ptr = (float)value;
    else if (h->element_size == sizeof(int)) *(int*)ptr = (int)value;
}

// Заполнение матрицы случайными значениями
static void fill_matrix_random(Matrix *m, double min, double max) {
    if (!m) return;
    for (int i = 0; i < m->rows; i++) {
        for (int j = 0; j < m->cols; j++) {
            double val = min + (max - min) * ((double)rand() / RAND_MAX);
            set_elem_double(m, i, j, val);
        }
    }
}

// Создание матрицы Гильберта: H[i][j] = 1/(i+j+1)
static Matrix* create_hilbert_matrix(int n) {
    Matrix *H = create_matrix(n, n, get_type_handler_by_name("DOUBLE"));
    if (!H) return NULL;
    
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            set_elem_double(H, i, j, 1.0 / (i + j + 1));
        }
    }
    return H;
}

// Евклидова норма вектора
static double vector_norm(const double *v, int n) {
    double sum = 0.0;
    for (int i = 0; i < n; i++) sum += v[i] * v[i];
    return sqrt(sum);
}

// Вычисление невязки: ||A*x - b||
static double compute_residual(const Matrix *A, const double *x, const double *b, int n) {
    double *res = calloc(n, sizeof(double));
    if (!res) return -1.0;
    
    for (int i = 0; i < n; i++) {
        double sum = 0.0;
        for (int j = 0; j < n; j++) {
            sum += get_elem_double(A, i, j) * x[j];
        }
        res[i] = sum - b[i];
    }
    
    double norm = vector_norm(res, n);
    free(res);
    return norm;
}

// Относительная погрешность: ||x_approx - x_exact|| / ||x_exact||
static double relative_error(const double *x_approx, const double *x_exact, int n) {
    double *diff = calloc(n, sizeof(double));
    if (!diff) return -1.0;
    
    for (int i = 0; i < n; i++) diff[i] = x_approx[i] - x_exact[i];
    
    double err_norm = vector_norm(diff, n);
    double exact_norm = vector_norm(x_exact, n);
    free(diff);
    
    return (exact_norm > 1e-12) ? err_norm / exact_norm : err_norm;
}

// Печать заголовка эксперимента
static void print_section_header(const char *title) {
    printf(COLOR_CYAN "\n╔════════════════════════════════════════╗\n" COLOR_RESET);
    printf(COLOR_CYAN "║  %-36s ║\n" COLOR_RESET, title);
    printf(COLOR_CYAN "╚════════════════════════════════════════╝\n" COLOR_RESET);
}

// Печать строки таблицы результатов
static void print_result_row(const char *method, double time_ms, double residual) {
    printf("  %-30s %10.3f ms   %12.6f\n", method, time_ms, residual);
}

// ЭКСПЕРИМЕНТ 4.1: Сравнение времени решения одной системы

int benchmark_single_system(void) {
    print_section_header("4.1: Время решения одной СЛАУ");
    
    const int sizes[] = {100, 200, 500, 1000};
    const int num_sizes = 4;
    
    printf("\n%-8s %-30s %-15s %-15s %-15s\n", 
           "n", "Метод", "Время (мс)", "Невязка", "Статус");
    printf("─────────────────────────────────────────────────────────\n");
    
    srand(42);  // Фиксированный seed для воспроизводимости
    
    for (int s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        
        // Генерация тестовых данных
        Matrix *A = create_matrix(n, n, get_type_handler_by_name("DOUBLE"));
        double *b = calloc(n, sizeof(double));
        double *x_gauss = calloc(n, sizeof(double));
        double *x_gauss_pivot = calloc(n, sizeof(double));
        double *x_lu = calloc(n, sizeof(double));
        
        if (!A || !b || !x_gauss || !x_gauss_pivot || !x_lu) {
            fprintf(stderr, "Ошибка выделения памяти для n=%d\n", n);
            continue;
        }
        
        fill_matrix_random(A, -1.0, 1.0);
        for (int i = 0; i < n; i++) b[i] = rand_uniform();
        
        // 1. Гаусс без выбора
        double t1 = get_time_us();
        int status1 = matrix_solve_gauss(A, b, x_gauss);
        double time1 = (get_time_us() - t1) / 1000.0;
        double resid1 = (status1 == 0) ? compute_residual(A, x_gauss, b, n) : -1.0;
        print_result_row("Гаусс (без выбора)", time1, resid1);
        
        // 2. Гаусс с выбором
        double t2 = get_time_us();
        int status2 = matrix_solve_gauss_pivot(A, b, x_gauss_pivot);
        double time2 = (get_time_us() - t2) / 1000.0;
        double resid2 = (status2 == 0) ? compute_residual(A, x_gauss_pivot, b, n) : -1.0;
        print_result_row("Гаусс (с выбором)", time2, resid2);
        
        // 3. LU-разложение + решение
        Matrix *L = create_matrix(n, n, get_type_handler_by_name("DOUBLE"));
        Matrix *U = create_matrix(n, n, get_type_handler_by_name("DOUBLE"));
        
        double t_lu = get_time_us();
        int status_lu_decomp = matrix_lu_decompose(A, L, U);
        double time_lu_decomp = (get_time_us() - t_lu) / 1000.0;
        
        double t_solve = get_time_us();
        int status_lu_solve = (status_lu_decomp == 0) ? matrix_solve_lu(L, U, b, x_lu) : -1;
        double time_lu_solve = (get_time_us() - t_solve) / 1000.0;
        
        double time_lu_total = time_lu_decomp + time_lu_solve;
        double resid3 = (status_lu_solve == 0) ? compute_residual(A, x_lu, b, n) : -1.0;
        
        printf("  %-30s %10.3f ms   %12.6f  [LU: %.3f + %.3f]\n", 
               "LU-разложение", time_lu_total, resid3, time_lu_decomp, time_lu_solve);
        
        // Очистка
        free_matrix(A);
        free_matrix(L);
        free_matrix(U);
        free(b);
        free(x_gauss);
        free(x_gauss_pivot);
        free(x_lu);
        
        printf("\n");
    }
    
    return 0;
}

// ЭКСПЕРИМЕНТ 4.2: Экономия при множественных правых частях

int benchmark_multiple_rhs(void) {
    print_section_header("4.2: Множественные правые части (n=500)");
    
    const int n = 500;
    const int k_values[] = {1, 10, 100};
    const int num_k = 3;
    
    printf("\n%-6s %-35s %-15s %-15s\n", 
           "k", "Метод", "Общее время (мс)", "На одну систему");
    printf("─────────────────────────────────────────────────────\n");
    
    srand(123);
    
    // Генерируем одну матрицу A
    Matrix *A = create_matrix(n, n, get_type_handler_by_name("DOUBLE"));
    fill_matrix_random(A, -1.0, 1.0);
    
    // Предварительное LU-разложение
    Matrix *L = create_matrix(n, n, get_type_handler_by_name("DOUBLE"));
    Matrix *U = create_matrix(n, n, get_type_handler_by_name("DOUBLE"));
    matrix_lu_decompose(A, L, U);
    
    for (int ki = 0; ki < num_k; ki++) {
        int k = k_values[ki];
        
        // Генерация k правых частей
        double **b_list = malloc(k * sizeof(double*));
        double **x_gauss_list = malloc(k * sizeof(double*));
        double **x_lu_list = malloc(k * sizeof(double*));
        
        for (int i = 0; i < k; i++) {
            b_list[i] = calloc(n, sizeof(double));
            x_gauss_list[i] = calloc(n, sizeof(double));
            x_lu_list[i] = calloc(n, sizeof(double));
            for (int j = 0; j < n; j++) b_list[i][j] = rand_uniform();
        }
        
        // Метод Гаусса: решаем каждую систему заново
        double t_gauss_start = get_time_us();
        for (int i = 0; i < k; i++) {
            matrix_solve_gauss_pivot(A, b_list[i], x_gauss_list[i]);
        }
        double time_gauss_total = (get_time_us() - t_gauss_start) / 1000.0;
        
        // LU-метод: одно разложение + k подстановок
        double t_lu_start = get_time_us();
        for (int i = 0; i < k; i++) {
            matrix_solve_lu(L, U, b_list[i], x_lu_list[i]);
        }
        double time_lu_total = (get_time_us() - t_lu_start) / 1000.0;
        
        printf("  %-4d %-35s %15.3f   %15.3f\n", 
               k, "Гаусс (с выбором)", time_gauss_total, time_gauss_total / k);
        printf("  %-4d %-35s %15.3f   %15.3f  [экономия: %.2fx]\n", 
               k, "LU-разложение", time_lu_total, time_lu_total / k,
               (time_lu_total > 0) ? time_gauss_total / time_lu_total : 0);
        printf("\n");
        
        // Очистка
        for (int i = 0; i < k; i++) {
            free(b_list[i]);
            free(x_gauss_list[i]);
            free(x_lu_list[i]);
        }
        free(b_list);
        free(x_gauss_list);
        free(x_lu_list);
    }
    
    free_matrix(A);
    free_matrix(L);
    free_matrix(U);
    
    return 0;
}

// ЭКСПЕРИМЕНТ 4.3: Точность на матрице Гильберта

int benchmark_hilbert_accuracy(void) {
    print_section_header("4.3: Точность на матрице Гильберта");
    
    const int sizes[] = {5, 10, 15};
    const int num_sizes = 3;
    
    printf("\n%-6s %-25s %-15s %-15s %-15s\n", 
           "n", "Метод", "Отн. погрешность", "Невязка", "Статус");
    printf("─────────────────────────────────────────────────────────\n");
    
    for (int s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        
        // Создаём матрицу Гильберта
        Matrix *H = create_hilbert_matrix(n);
        if (!H) continue;
        
        // Точное решение: x = [1, 1, ..., 1]
        double *x_exact = calloc(n, sizeof(double));
        double *b = calloc(n, sizeof(double));
        for (int i = 0; i < n; i++) x_exact[i] = 1.0;
        
        // Вычисляем b = H * x_exact
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                b[i] += get_elem_double(H, i, j) * x_exact[j];
            }
        }
        
        // Массивы для решений
        double *x_gauss = calloc(n, sizeof(double));
        double *x_gauss_pivot = calloc(n, sizeof(double));
        double *x_lu = calloc(n, sizeof(double));
        
        // 1. Гаусс без выбора
        int status1 = matrix_solve_gauss(H, b, x_gauss);
        double err1 = (status1 == 0) ? relative_error(x_gauss, x_exact, n) : -1.0;
        double resid1 = (status1 == 0) ? compute_residual(H, x_gauss, b, n) : -1.0;
        printf("  %-4d %-25s %15.6e   %15.6e   %s\n", 
               n, "Гаусс (без выбора)", err1, resid1, 
               (status1 == 0) ? "OK" : "FAIL");
        
        // 2. Гаусс с выбором
        int status2 = matrix_solve_gauss_pivot(H, b, x_gauss_pivot);
        double err2 = (status2 == 0) ? relative_error(x_gauss_pivot, x_exact, n) : -1.0;
        double resid2 = (status2 == 0) ? compute_residual(H, x_gauss_pivot, b, n) : -1.0;
        printf("  %-4d %-25s %15.6e   %15.6e   %s\n", 
               n, "Гаусс (с выбором)", err2, resid2,
               (status2 == 0) ? "OK" : "FAIL");
        
        // 3. LU-разложение
        Matrix *L = create_matrix(n, n, get_type_handler_by_name("DOUBLE"));
        Matrix *U = create_matrix(n, n, get_type_handler_by_name("DOUBLE"));
        int status_lu = matrix_lu_decompose(H, L, U);
        int status3 = (status_lu == 0) ? matrix_solve_lu(L, U, b, x_lu) : -1;
        double err3 = (status3 == 0) ? relative_error(x_lu, x_exact, n) : -1.0;
        double resid3 = (status3 == 0) ? compute_residual(H, x_lu, b, n) : -1.0;
        printf("  %-4d %-25s %15.6e   %15.6e   %s\n", 
               n, "LU-разложение", err3, resid3,
               (status3 == 0) ? "OK" : "FAIL");
        
        printf("\n");
        
        // Очистка
        free_matrix(H);
        free_matrix(L);
        free_matrix(U);
        free(x_exact);
        free(b);
        free(x_gauss);
        free(x_gauss_pivot);
        free(x_lu);
    }
    
    return 0;
}

// ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ ДЛЯ QR

// Проверка ортогональности Q: ||Qᵀ×Q - I||
static double check_orthogonality(const Matrix *Q) {
    if (!Q || Q->rows != Q->cols) return -1.0;
    
    int n = Q->rows;
    double *I = calloc(n * n, sizeof(double));  // Единичная матрица
    double *QtQ = calloc(n * n, sizeof(double)); // Qᵀ×Q
    
    // I[i][i] = 1
    for (int i = 0; i < n; i++) I[i * n + i] = 1.0;
    
    // QtQ = Qᵀ × Q
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            double sum = 0.0;
            for (int k = 0; k < n; k++) {
                sum += get_elem_double(Q, k, i) * get_elem_double(Q, k, j);
            }
            QtQ[i * n + j] = sum;
        }
    }
    
    // Норма разности ||QtQ - I||
    double norm = 0.0;
    for (int i = 0; i < n * n; i++) {
        double diff = QtQ[i] - I[i];
        norm += diff * diff;
    }
    
    free(I);
    free(QtQ);
    return sqrt(norm);
}

// Проверка верхней треугольности R
static int check_upper_triangular(const Matrix *R, double tol) {
    if (!R || R->rows != R->cols) return 0;
    
    int n = R->rows;
    for (int i = 1; i < n; i++) {
        for (int j = 0; j < i; j++) {
            if (fabs(get_elem_double(R, i, j)) > tol) return 0;
        }
    }
    return 1;
}

// Проверка разложения: ||A - Q×R||
static double check_decomposition(const Matrix *A, const Matrix *Q, const Matrix *R) {
    if (!A || !Q || !R) return -1.0;
    if (A->rows != Q->rows || A->cols != R->cols) return -1.0;
    
    int m = A->rows;
    int n = A->cols;
    double norm = 0.0;
    
    for (int i = 0; i < m; i++) {
        for (int j = 0; j < n; j++) {
            double a_ij = get_elem_double(A, i, j);
            double qr_ij = 0.0;
            for (int k = 0; k < n; k++) {
                qr_ij += get_elem_double(Q, i, k) * get_elem_double(R, k, j);
            }
            double diff = a_ij - qr_ij;
            norm += diff * diff;
        }
    }
    
    return sqrt(norm);
}

// ЭКСПЕРИМЕНТ 4.4: Проверка QR-разложения

int benchmark_qr_decomposition(void) {
    print_section_header("4.4: Проверка QR-разложения");
    
    const int sizes[] = {50, 100, 200};
    const int num_sizes = 3;
    
    printf("\n%-6s %-15s %-15s %-15s %-15s %-10s\n", 
           "n", "Время (мс)", "||A-QR||", "Ортогональность", "R треугольная", "Статус");
    printf("────────────────────────────────────────────────────────────────────\n");
    
    srand(456);
    
    for (int s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        
        // Генерация случайной матрицы
        Matrix *A = create_matrix(n, n, get_type_handler_by_name("DOUBLE"));
        Matrix *Q = create_matrix(n, n, get_type_handler_by_name("DOUBLE"));
        Matrix *R = create_matrix(n, n, get_type_handler_by_name("DOUBLE"));
        
        if (!A || !Q || !R) {
            fprintf(stderr, "Ошибка выделения памяти для n=%d\n", n);
            continue;
        }
        
        fill_matrix_random(A, -1.0, 1.0);
        
        // QR-разложение
        double t_start = get_time_us();
        int status = matrix_qr_decompose(A, Q, R);
        double time_ms = (get_time_us() - t_start) / 1000.0;
        
        // Проверки
        double decomp_err = check_decomposition(A, Q, R);
        double ortho_err = check_orthogonality(Q);
        int is_upper = check_upper_triangular(R, 1e-10);
        
        printf("  %-4d %-15.3f %-15.6e %-15.6e %-15s %-10s\n",
               n, time_ms, decomp_err, ortho_err,
               is_upper ? "Да" : "Нет",
               (status == 0) ? "OK" : "FAIL");
        
        free_matrix(A);
        free_matrix(Q);
        free_matrix(R);
    }
    
    return 0;
}

// ЭКСПЕРИМЕНТ 4.5: Сравнение точности QR с другими методами

int benchmark_qr_accuracy(void) {
    print_section_header("4.5: Точность QR vs Гаусс vs LU (матрица Гильберта)");
    
    const int sizes[] = {5, 10, 15};
    const int num_sizes = 3;
    
    printf("\n%-6s %-12s %-18s %-18s %-18s\n", 
           "n", "Метод", "Отн. погрешность", "Невязка", "Статус");
    printf("─────────────────────────────────────────────────────────────────\n");
    
    for (int s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        
        // Матрица Гильберта
        Matrix *H = create_hilbert_matrix(n);
        if (!H) continue;
        
        // Точное решение x = [1, 1, ..., 1]
        double *x_exact = calloc(n, sizeof(double));
        double *b = calloc(n, sizeof(double));
        for (int i = 0; i < n; i++) x_exact[i] = 1.0;
        
        // b = H × x_exact
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                b[i] += get_elem_double(H, i, j) * x_exact[j];
            }
        }
        
        // Массивы для решений
        double *x_gauss = calloc(n, sizeof(double));
        double *x_lu = calloc(n, sizeof(double));
        double *x_qr = calloc(n, sizeof(double));
        
        // 1. Гаусс с выбором
        int status1 = matrix_solve_gauss_pivot(H, b, x_gauss);
        double err1 = (status1 == 0) ? relative_error(x_gauss, x_exact, n) : -1.0;
        double resid1 = (status1 == 0) ? compute_residual(H, x_gauss, b, n) : -1.0;
        printf("  %-4d %-12s %18.6e   %18.6e   %s\n",
               n, "Гаусс", err1, resid1, (status1 == 0) ? "OK" : "FAIL");
        
        // 2. LU-разложение
        Matrix *L = create_matrix(n, n, get_type_handler_by_name("DOUBLE"));
        Matrix *U = create_matrix(n, n, get_type_handler_by_name("DOUBLE"));
        int status_lu = matrix_lu_decompose(H, L, U);
        int status2 = (status_lu == 0) ? matrix_solve_lu(L, U, b, x_lu) : -1;
        double err2 = (status2 == 0) ? relative_error(x_lu, x_exact, n) : -1.0;
        double resid2 = (status2 == 0) ? compute_residual(H, x_lu, b, n) : -1.0;
        printf("  %-4d %-12s %18.6e   %18.6e   %s\n",
               n, "LU", err2, resid2, (status2 == 0) ? "OK" : "FAIL");
        
        // 3. QR-разложение
        Matrix *Q = create_matrix(n, n, get_type_handler_by_name("DOUBLE"));
        Matrix *R = create_matrix(n, n, get_type_handler_by_name("DOUBLE"));
        int status_qr = matrix_qr_decompose(H, Q, R);
        int status3 = (status_qr == 0) ? matrix_solve_qr(Q, R, b, x_qr) : -1;
        double err3 = (status3 == 0) ? relative_error(x_qr, x_exact, n) : -1.0;
        double resid3 = (status3 == 0) ? compute_residual(H, x_qr, b, n) : -1.0;
        printf("  %-4d %-12s %18.6e   %18.6e   %s\n",
               n, "QR", err3, resid3, (status3 == 0) ? "OK" : "FAIL");
        
        printf("\n");
        
        // Очистка
        free_matrix(H);
        free_matrix(L);
        free_matrix(U);
        free_matrix(Q);
        free_matrix(R);
        free(x_exact);
        free(b);
        free(x_gauss);
        free(x_lu);
        free(x_qr);
    }
    
    return 0;
}

// ЭКСПЕРИМЕНТ 4.6: Время QR vs других методов

int benchmark_qr_time(void) {
    print_section_header("4.6: Сравнение времени (QR vs Гаусс vs LU)");
    
    const int sizes[] = {100, 200, 500};
    const int num_sizes = 3;
    
    printf("\n%-6s %-12s %-15s %-15s %-15s\n", 
           "n", "Метод", "Время (мс)", "Невязка", "Статус");
    printf("─────────────────────────────────────────────────────────────\n");
    
    srand(789);
    
    for (int s = 0; s < num_sizes; s++) {
        int n = sizes[s];
        
        Matrix *A = create_matrix(n, n, get_type_handler_by_name("DOUBLE"));
        double *b = calloc(n, sizeof(double));
        double *x_gauss = calloc(n, sizeof(double));
        double *x_lu = calloc(n, sizeof(double));
        double *x_qr = calloc(n, sizeof(double));
        
        fill_matrix_random(A, -1.0, 1.0);
        for (int i = 0; i < n; i++) b[i] = rand_uniform();
        
        // 1. Гаусс с выбором
        double t1 = get_time_us();
        int status1 = matrix_solve_gauss_pivot(A, b, x_gauss);
        double time1 = (get_time_us() - t1) / 1000.0;
        double resid1 = (status1 == 0) ? compute_residual(A, x_gauss, b, n) : -1.0;
        printf("  %-4d %-12s %15.3f   %15.6f   %s\n",
               n, "Гаусс", time1, resid1, (status1 == 0) ? "OK" : "FAIL");
        
        // 2. LU
        Matrix *L = create_matrix(n, n, get_type_handler_by_name("DOUBLE"));
        Matrix *U = create_matrix(n, n, get_type_handler_by_name("DOUBLE"));
        double t2 = get_time_us();
        int status_lu = matrix_lu_decompose(A, L, U);
        int status2 = (status_lu == 0) ? matrix_solve_lu(L, U, b, x_lu) : -1;
        double time2 = (get_time_us() - t2) / 1000.0;
        double resid2 = (status2 == 0) ? compute_residual(A, x_lu, b, n) : -1.0;
        printf("  %-4d %-12s %15.3f   %15.6f   %s\n",
               n, "LU", time2, resid2, (status2 == 0) ? "OK" : "FAIL");
        
        // 3. QR
        Matrix *Q = create_matrix(n, n, get_type_handler_by_name("DOUBLE"));
        Matrix *R = create_matrix(n, n, get_type_handler_by_name("DOUBLE"));
        double t3 = get_time_us();
        int status_qr = matrix_qr_decompose(A, Q, R);
        int status3 = (status_qr == 0) ? matrix_solve_qr(Q, R, b, x_qr) : -1;
        double time3 = (get_time_us() - t3) / 1000.0;
        double resid3 = (status3 == 0) ? compute_residual(A, x_qr, b, n) : -1.0;
        printf("  %-4d %-12s %15.3f   %15.6f   %s\n",
               n, "QR", time3, resid3, (status3 == 0) ? "OK" : "FAIL");
        
        printf("\n");
        
        // Очистка
        free_matrix(A);
        free_matrix(L);
        free_matrix(U);
        free_matrix(Q);
        free_matrix(R);
        free(b);
        free(x_gauss);
        free(x_lu);
        free(x_qr);
    }
    
    return 0;
}

// ОБНОВЛЁННЫЙ ЗАПУСК ВСЕХ БЕНЧМАРКОВ

int run_all_benchmarks(void) {
    printf(COLOR_GREEN "\n╔════════════════════════════════════════╗\n" COLOR_RESET);
    printf(COLOR_GREEN "║     MATRIX LIBRARY BENCHMARK SUITE     ║\n" COLOR_RESET);
    printf(COLOR_GREEN "║     Гаусс ↔ LU ↔ QR                    ║\n" COLOR_RESET);
    printf(COLOR_GREEN "╚════════════════════════════════════════╝\n" COLOR_RESET);
        
    // Оригинальные тесты из ТЗ
    benchmark_single_system();
    benchmark_multiple_rhs();
    benchmark_hilbert_accuracy();
    
    // Новые QR-тесты
    benchmark_qr_decomposition();
    benchmark_qr_accuracy();
    benchmark_qr_time();
    
    printf(COLOR_GREEN "\nВсе бенчмарки завершены!\n" COLOR_RESET);
    printf("Результаты можно скопировать в отчёт или сохранить в файл.\n");
    printf("\nПеренаправить вывод в файл для отчёта:\n");
    printf("   ./bin/matrix_benchmark > results.txt\n");
    
    return 0;
}