#include "matrix_types.h"
#include <stdio.h>
#include <string.h>

// Функции для инта
static void int_set(void *data, int index, const void *value) {
    ((int*)data)[index] = *(const int*)value;
}
static void int_get(void *data, int index, void *out_value) {
    *(int*)out_value = ((int*)data)[index];
}
static void int_set_zero(void *data, int index) {
    ((int*)data)[index] = 0;
}
static void int_add(void *result, const void *a, const void *b) {
    *(int*)result = *(const int*)a + *(const int*)b;
}
static void int_multiply(void *result, const void *a, const void *b) {
    *(int*)result = *(const int*)a * *(const int*)b;
}
static void int_scale(void *result, const void *a, double coeff) {
    *(int*)result = (int)(*(const int*)a * coeff);
}
static void int_print(const void *value) {
    printf("%4d", *(const int*)value);
}

// Функции для дабла
static void double_set(void *data, int index, const void *value) {
    ((double*)data)[index] = *(const double*)value;
}
static void double_get(void *data, int index, void *out_value) {
    *(double*)out_value = ((double*)data)[index];
}
static void double_set_zero(void *data, int index) {
    ((double*)data)[index] = 0.0;
}
static void double_add(void *result, const void *a, const void *b) {
    *(double*)result = *(const double*)a + *(const double*)b;
}
static void double_multiply(void *result, const void *a, const void *b) {
    *(double*)result = *(const double*)a * *(const double*)b;
}
static void double_scale(void *result, const void *a, double coeff) {
    *(double*)result = *(const double*)a * coeff;
}
static void double_print(const void *value) {
    printf("%6.2f", *(const double*)value);
}

// Таблицы обработчиков
static const TypeHandler handler_int = {
    .type_name = "INT",
    .element_size = sizeof(int),
    .set_element = int_set,
    .get_element = int_get,
    .set_zero = int_set_zero,
    .add_elements = int_add,
    .multiply_elements = int_multiply,
    .scale_element = int_scale,
    .print_element = int_print
};

static const TypeHandler handler_double = {
    .type_name = "DOUBLE",
    .element_size = sizeof(double),
    .set_element = double_set,
    .get_element = double_get,
    .set_zero = double_set_zero,
    .add_elements = double_add,
    .multiply_elements = double_multiply,
    .scale_element = double_scale,
    .print_element = double_print
};

// Массив всех доступных обработчиков
static const TypeHandler *handlers[] = {
    &handler_int,
    &handler_double,
    NULL  // Терминатор
};

// Получить обработчик по имени
const TypeHandler* get_type_handler_by_name(const char *name) {
    for (int i = 0; handlers[i] != NULL; i++) {
        if (strcmp(handlers[i]->type_name, name) == 0) {
            return handlers[i];
        }
    }
    return NULL;
}

// Получить все обработчики (для GUI)
const TypeHandler** get_all_type_handlers(int *count) {
    int c = 0;
    while (handlers[c] != NULL) c++;
    if (count) *count = c;
    return handlers;
}

// Для обратной совместимости
const TypeHandler* get_type_handler_int(void) { return &handler_int; }
const TypeHandler* get_type_handler_double(void) { return &handler_double; }