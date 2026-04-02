#include "matrix_benchmark.h"
#include "utils.h"
#include <stdio.h>

int main(void) {

    printf(COLOR_CYAN "╔════════════════════════════════════════╗\n" COLOR_RESET);
    printf(COLOR_CYAN "║  СРАВНЕНИЕ МЕТОДОВ РЕШЕНИЯ СЛАУ        ║\n" COLOR_RESET);
    printf(COLOR_CYAN "║  Гаусс ↔ LU ↔ QR                       ║\n" COLOR_RESET);
    printf(COLOR_CYAN "╚════════════════════════════════════════╝\n\n" COLOR_RESET);
    
    printf("   Эксперименты:\n");
    printf("   4.1: Время решения одной системы (n=100..1000)\n");
    printf("   4.2: Экономия при множественных правых частях\n");
    printf("   4.3: Точность на матрице Гильберта\n");
    printf("   4.4: Проверка QR-разложения [НОВЫЙ]\n");
    printf("   4.5: Точность QR vs Гаусс vs LU [НОВЫЙ]\n");
    printf("   4.6: Время QR vs других методов [НОВЫЙ]\n\n");
    
    return run_all_benchmarks();
}