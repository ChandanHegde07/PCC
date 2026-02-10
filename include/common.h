/**
 * @file common.h
 * @brief Common definitions and types for PCC compiler
 */

#ifndef PCC_COMMON_H
#define PCC_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>

/* Compiler version */
#define PCC_VERSION "1.0.0"

/* Maximum sizes */
#define MAX_IDENTIFIER_LEN 256
#define MAX_STRING_LEN 4096
#define MAX_TOKEN_LEN 1024
#define MAX_LINE_LEN 8192
#define INITIAL_CAPACITY 16

/* Error codes */
typedef enum {
    PCC_SUCCESS = 0,
    PCC_ERROR_MEMORY,
    PCC_ERROR_SYNTAX,
    PCC_ERROR_SEMANTIC,
    PCC_ERROR_IO,
    PCC_ERROR_RUNTIME,
    PCC_ERROR_UNKNOWN
} PCCError;

/* Boolean type */
typedef enum {
    PCC_FALSE = 0,
    PCC_TRUE = 1
} PCCBool;

/* Value types */
typedef enum {
    PCC_TYPE_VOID,
    PCC_TYPE_STRING,
    PCC_TYPE_NUMBER,
    PCC_TYPE_BOOLEAN,
    PCC_TYPE_ARRAY,
    PCC_TYPE_OBJECT
} PCCType;

/* Generic value union */
typedef union {
    char* string_val;
    double number_val;
    int bool_val;
    void* ptr_val;
} PCCValue;

/* Generic value structure */
typedef struct {
    PCCType type;
    PCCValue value;
} PCCValueStruct;

/* Position information for error reporting */
typedef struct {
    int line;
    int column;
    const char* filename;
} PCCPosition;

/* Memory management macros */
#define PCC_MALLOC(ptr, size) \
    do { \
        (ptr) = malloc(size); \
        if ((ptr) == NULL) { \
            fprintf(stderr, "Memory allocation failed at %s:%d\n", __FILE__, __LINE__); \
            exit(PCC_ERROR_MEMORY); \
        } \
    } while(0)

#define PCC_CALLOC(ptr, count, size) \
    do { \
        (ptr) = calloc(count, size); \
        if ((ptr) == NULL) { \
            fprintf(stderr, "Memory allocation failed at %s:%d\n", __FILE__, __LINE__); \
            exit(PCC_ERROR_MEMORY); \
        } \
    } while(0)

#define PCC_REALLOC(ptr, new_size) \
    do { \
        void* _tmp = realloc(ptr, new_size); \
        if (_tmp == NULL) { \
            fprintf(stderr, "Memory reallocation failed at %s:%d\n", __FILE__, __LINE__); \
            exit(PCC_ERROR_MEMORY); \
        } \
        (ptr) = _tmp; \
    } while(0)

#define PCC_FREE(ptr) \
    do { \
        if ((ptr) != NULL) { \
            free(ptr); \
            (ptr) = NULL; \
        } \
    } while(0)

/* String duplication */
static inline char* pcc_strdup(const char* str) {
    if (str == NULL) return NULL;
    size_t len = strlen(str) + 1;
    char* result = (char*)malloc(len);
    if (result == NULL) return NULL;
    memcpy(result, str, len);
    return result;
}

/* Min/Max macros */
#define PCC_MIN(a, b) ((a) < (b) ? (a) : (b))
#define PCC_MAX(a, b) ((a) > (b) ? (a) : (b))

/* Debug macros */
#ifdef PCC_DEBUG
    #define PCC_DEBUG_PRINT(fmt, ...) \
        fprintf(stderr, "[DEBUG] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#else
    #define PCC_DEBUG_PRINT(fmt, ...) ((void)0)
#endif

/* Error reporting */
#define PCC_ERROR_PRINT(fmt, ...) \
    fprintf(stderr, "[ERROR] %s:%d: " fmt "\n", __FILE__, __LINE__, ##__VA_ARGS__)

#endif /* PCC_COMMON_H */
