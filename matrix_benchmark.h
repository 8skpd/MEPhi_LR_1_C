#ifndef MATRIX_BENCHMARK_H
#define MATRIX_BENCHMARK_H

// Тесты из ТЗ
int benchmark_single_system(void);
int benchmark_multiple_rhs(void);
int benchmark_hilbert_accuracy(void);

// QR-тесты
int benchmark_qr_decomposition(void);
int benchmark_qr_accuracy(void);
int benchmark_qr_time(void);

// Запуск всех бенчмарков
int run_all_benchmarks(void);

#endif // MATRIX_BENCHMARK_H