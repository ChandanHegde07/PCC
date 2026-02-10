/**
 * @file array.h
 * @brief Dynamic array implementation for PCC compiler
 * @details Implements a generic dynamic array using void pointers
 *          DSA Concept: Array with dynamic resizing
 */

#ifndef PCC_ARRAY_H
#define PCC_ARRAY_H

#include "common.h"

/* Dynamic array structure */
typedef struct {
    void** data;           /* Array of void pointers */
    size_t size;           /* Current number of elements */
    size_t capacity;       /* Current capacity */
    size_t element_size;   /* Size of each element */
} PCCArray;

/* Function prototypes */

/**
 * @brief Create a new dynamic array
 * @param initial_capacity Initial capacity of the array
 * @param element_size Size of each element in bytes
 * @return Pointer to new array, or NULL on failure
 */
PCCArray* pcc_array_create(size_t initial_capacity, size_t element_size);

/**
 * @brief Free array and its contents
 * @param array Array to free
 * @param free_func Optional function to free each element (can be NULL)
 */
void pcc_array_free(PCCArray* array, void (*free_func)(void*));

/**
 * @brief Add an element to the end of the array
 * @param array Array to add to
 * @param element Element to add
 * @return PCC_SUCCESS on success, error code on failure
 */
PCCError pcc_array_push(PCCArray* array, void* element);

/**
 * @brief Remove and return the last element
 * @param array Array to pop from
 * @param out Pointer to store the popped element
 * @return PCC_SUCCESS on success, error code on failure
 */
PCCError pcc_array_pop(PCCArray* array, void** out);

/**
 * @brief Get element at index
 * @param array Array to get from
 * @param index Index of element
 * @return Pointer to element, or NULL if index out of bounds
 */
void* pcc_array_get(PCCArray* array, size_t index);

/**
 * @brief Set element at index
 * @param array Array to set in
 * @param index Index to set
 * @param element Element to set
 * @return PCC_SUCCESS on success, error code on failure
 */
PCCError pcc_array_set(PCCArray* array, size_t index, void* element);

/**
 * @brief Insert element at index
 * @param array Array to insert into
 * @param index Index to insert at
 * @param element Element to insert
 * @return PCC_SUCCESS on success, error code on failure
 */
PCCError pcc_array_insert(PCCArray* array, size_t index, void* element);

/**
 * @brief Remove element at index
 * @param array Array to remove from
 * @param index Index to remove
 * @param out Pointer to store removed element (can be NULL)
 * @return PCC_SUCCESS on success, error code on failure
 */
PCCError pcc_array_remove(PCCArray* array, size_t index, void** out);

/**
 * @brief Find index of element
 * @param array Array to search
 * @param element Element to find
 * @param compare_func Comparison function (returns 0 if equal)
 * @return Index of element, or -1 if not found
 */
int pcc_array_find(PCCArray* array, void* element, int (*compare_func)(const void*, const void*));

/**
 * @brief Check if array contains element
 * @param array Array to search
 * @param element Element to find
 * @param compare_func Comparison function
 * @return PCC_TRUE if found, PCC_FALSE otherwise
 */
PCCBool pcc_array_contains(PCCArray* array, void* element, int (*compare_func)(const void*, const void*));

/**
 * @brief Clear all elements from array
 * @param array Array to clear
 * @param free_func Optional function to free each element
 */
void pcc_array_clear(PCCArray* array, void (*free_func)(void*));

/**
 * @brief Resize array to new capacity
 * @param array Array to resize
 * @param new_capacity New capacity
 * @return PCC_SUCCESS on success, error code on failure
 */
PCCError pcc_array_resize(PCCArray* array, size_t new_capacity);

/**
 * @brief Sort array using comparison function
 * @param array Array to sort
 * @param compare_func Comparison function
 */
void pcc_array_sort(PCCArray* array, int (*compare_func)(const void*, const void*));

/**
 * @brief Reverse array in place
 * @param array Array to reverse
 */
void pcc_array_reverse(PCCArray* array);

/**
 * @brief Get current size of array
 * @param array Array to query
 * @return Number of elements in array
 */
static inline size_t pcc_array_size(PCCArray* array) {
    return array ? array->size : 0;
}

/**
 * @brief Check if array is empty
 * @param array Array to query
 * @return PCC_TRUE if empty, PCC_FALSE otherwise
 */
static inline PCCBool pcc_array_empty(PCCArray* array) {
    return (PCCBool)(array ? (array->size == 0) : PCC_TRUE);
}

/**
 * @brief Get current capacity of array
 * @param array Array to query
 * @return Current capacity
 */
static inline size_t pcc_array_capacity(PCCArray* array) {
    return array ? array->capacity : 0;
}

#endif /* PCC_ARRAY_H */
