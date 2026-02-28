#include "utils.h"
#include <stdio.h>

void clear_console(void) {
    printf("\033[2J\033[H");
    fflush(stdout);
}