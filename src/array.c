/**
 * @file array.c
 * @brief Dynamic array implementation for PCC compiler
 */

#include "array.h"

/* Create a new dynamic array */
PCCArray* pcc_array_create(size_t initial_capacity, size_t element_size) {
    if (initial_capacity == 0) {
        initial_capacity = INITIAL_CAPACITY;
    }
    if (element_size == 0) {
        return NULL;
    }

    PCCArray* array = (PCCArray*)malloc(sizeof(PCCArray));
    if (array == NULL) {
        return NULL;
    }

    array->data = (void**)malloc(initial_capacity * sizeof(void*));
    if (array->data == NULL) {
        free(array);
        return NULL;
    }

    array->size = 0;
    array->capacity = initial_capacity;
    array->element_size = element_size;

    return array;
}

/* Free array and its contents */
void pcc_array_free(PCCArray* array, void (*free_func)(void*)) {
    if (array == NULL) {
        return;
    }

    if (free_func != NULL) {
        for (size_t i = 0; i < array->size; i++) {
            if (array->data[i] != NULL) {
                free_func(array->data[i]);
            }
        }
    }

    free(array->data);
    free(array);
}

/* Add an element to the end of the array */
PCCError pcc_array_push(PCCArray* array, void* element) {
    if (array == NULL || element == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    /* Resize if needed */
    if (array->size >= array->capacity) {
        size_t new_capacity = array->capacity * 2;
        void** new_data = (void**)realloc(array->data, new_capacity * sizeof(void*));
        if (new_data == NULL) {
            return PCC_ERROR_MEMORY;
        }
        array->data = new_data;
        array->capacity = new_capacity;
    }

    array->data[array->size] = element;
    array->size++;

    return PCC_SUCCESS;
}

/* Remove and return the last element */
PCCError pcc_array_pop(PCCArray* array, void** out) {
    if (array == NULL || array->size == 0) {
        return PCC_ERROR_RUNTIME;
    }

    if (out != NULL) {
        *out = array->data[array->size - 1];
    }

    array->size--;

    /* Shrink if capacity is much larger than size */
    if (array->size > 0 && array->capacity > array->size * 4) {
        size_t new_capacity = array->capacity / 2;
        void** new_data = (void**)realloc(array->data, new_capacity * sizeof(void*));
        if (new_data != NULL) {
            array->data = new_data;
            array->capacity = new_capacity;
        }
    }

    return PCC_SUCCESS;
}

/* Get element at index */
void* pcc_array_get(PCCArray* array, size_t index) {
    if (array == NULL || index >= array->size) {
        return NULL;
    }
    return array->data[index];
}

/* Set element at index */
PCCError pcc_array_set(PCCArray* array, size_t index, void* element) {
    if (array == NULL || element == NULL || index >= array->size) {
        return PCC_ERROR_RUNTIME;
    }

    array->data[index] = element;
    return PCC_SUCCESS;
}

/* Insert element at index */
PCCError pcc_array_insert(PCCArray* array, size_t index, void* element) {
    if (array == NULL || element == NULL || index > array->size) {
        return PCC_ERROR_RUNTIME;
    }

    /* Resize if needed */
    if (array->size >= array->capacity) {
        size_t new_capacity = array->capacity * 2;
        void** new_data = (void**)realloc(array->data, new_capacity * sizeof(void*));
        if (new_data == NULL) {
            return PCC_ERROR_MEMORY;
        }
        array->data = new_data;
        array->capacity = new_capacity;
    }

    /* Shift elements to the right */
    for (size_t i = array->size; i > index; i--) {
        array->data[i] = array->data[i - 1];
    }

    array->data[index] = element;
    array->size++;

    return PCC_SUCCESS;
}

/* Remove element at index */
PCCError pcc_array_remove(PCCArray* array, size_t index, void** out) {
    if (array == NULL || index >= array->size) {
        return PCC_ERROR_RUNTIME;
    }

    if (out != NULL) {
        *out = array->data[index];
    }

    /* Shift elements to the left */
    for (size_t i = index; i < array->size - 1; i++) {
        array->data[i] = array->data[i + 1];
    }

    array->size--;

    return PCC_SUCCESS;
}

/* Find index of element */
int pcc_array_find(PCCArray* array, void* element, int (*compare_func)(const void*, const void*)) {
    if (array == NULL || element == NULL || compare_func == NULL) {
        return -1;
    }

    for (size_t i = 0; i < array->size; i++) {
        if (compare_func(array->data[i], element) == 0) {
            return (int)i;
        }
    }

    return -1;
}

/* Check if array contains element */
PCCBool pcc_array_contains(PCCArray* array, void* element, int (*compare_func)(const void*, const void*)) {
    return (PCCBool)(pcc_array_find(array, element, compare_func) != -1);
}

/* Clear all elements from array */
void pcc_array_clear(PCCArray* array, void (*free_func)(void*)) {
    if (array == NULL) {
        return;
    }

    if (free_func != NULL) {
        for (size_t i = 0; i < array->size; i++) {
            if (array->data[i] != NULL) {
                free_func(array->data[i]);
            }
        }
    }

    array->size = 0;
}

/* Resize array to new capacity */
PCCError pcc_array_resize(PCCArray* array, size_t new_capacity) {
    if (array == NULL || new_capacity < array->size) {
        return PCC_ERROR_RUNTIME;
    }

    void** new_data = (void**)realloc(array->data, new_capacity * sizeof(void*));
    if (new_data == NULL) {
        return PCC_ERROR_MEMORY;
    }

    array->data = new_data;
    array->capacity = new_capacity;

    return PCC_SUCCESS;
}

/* Sort array using comparison function */
void pcc_array_sort(PCCArray* array, int (*compare_func)(const void*, const void*)) {
    if (array == NULL || compare_func == NULL || array->size < 2) {
        return;
    }

    /* Simple bubble sort - can be replaced with qsort for better performance */
    for (size_t i = 0; i < array->size - 1; i++) {
        for (size_t j = 0; j < array->size - i - 1; j++) {
            if (compare_func(array->data[j], array->data[j + 1]) > 0) {
                void* temp = array->data[j];
                array->data[j] = array->data[j + 1];
                array->data[j + 1] = temp;
            }
        }
    }
}

/* Reverse array in place */
void pcc_array_reverse(PCCArray* array) {
    if (array == NULL || array->size < 2) {
        return;
    }

    for (size_t i = 0; i < array->size / 2; i++) {
        void* temp = array->data[i];
        array->data[i] = array->data[array->size - 1 - i];
        array->data[array->size - 1 - i] = temp;
    }
}
