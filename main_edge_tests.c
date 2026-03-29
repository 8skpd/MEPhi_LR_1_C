#include "matrix_edge_tests.h"
#include "utils.h"
#include <stdio.h>

int main(void) {
    clear_console();
    printf(COLOR_CYAN "╔════════════════════════════════════════╗\n" COLOR_RESET);
    printf(COLOR_CYAN "║   EDGE CASE & EXCEPTION TEST SUITE     ║\n" COLOR_RESET);
    printf(COLOR_CYAN "╚════════════════════════════════════════╝\n\n" COLOR_RESET);
    
    return run_all_edge_tests();
}