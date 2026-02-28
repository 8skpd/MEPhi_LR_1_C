#ifndef MATRIX_TYPES_H
#define MATRIX_TYPES_H

#include <stddef.h>


// Объявление структуры чтобы избежать циклической завсимости
struct TypeHandler;

// Структура обработчика операций для конкретного типа
typedef struct TypeHandler {
    const char *type_name;      // Имя типа для вывода
    size_t element_size;        // Размер элемента в байтах
    
    // Операции над элементами
    void (*set_element)(void *data, int index, const void *value);
    void (*get_element)(void *data, int index, void *out_value);
    void (*set_zero)(void *data, int index);
    
    // Арифметические операции
    void (*add_elements)(void *result, const void *a, const void *b);
    void (*multiply_elements)(void *result, const void *a, const void *b);
    void (*scale_element)(void *result, const void *a, double coeff);
    
    // Для вывода
    void (*print_element)(const void *value);
} TypeHandler;

// Получить обработчик по имени типа
const TypeHandler* get_type_handler_by_name(const char *name);

// Получить все доступные обработчики
const TypeHandler** get_all_type_handlers(int *count);

#endif // MATRIX_TYPES_H