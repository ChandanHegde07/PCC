/**
 * @file symbol_table.h
 * @brief Symbol table for PCC compiler
 * @details Tracks variables, templates, prompts, and constraints
 *          DSA Concept: Hash table with scope management
 */

#ifndef PCC_SYMBOL_TABLE_H
#define PCC_SYMBOL_TABLE_H

#include "common.h"
#include "hashtable.h"
#include "array.h"

/* Symbol types */
typedef enum {
    SYMBOL_VARIABLE,
    SYMBOL_TEMPLATE,
    SYMBOL_PROMPT,
    SYMBOL_CONSTRAINT,
    SYMBOL_PARAMETER,
    SYMBOL_UNKNOWN
} SymbolType;

/* Symbol structure */
typedef struct {
    char* name;              /* Symbol name */
    SymbolType type;         /* Symbol type */
    void* data;              /* Associated data (AST node, type info, etc.) */
    int is_defined;          /* Whether symbol has been defined */
    int is_used;             /* Whether symbol has been used */
    PCCPosition position;    /* Definition position */
} Symbol;

/* Scope structure */
typedef struct {
    PCCHashTable* symbols;   /* Symbols in this scope */
    struct Scope* parent;    /* Parent scope (for nested scopes) */
    int scope_level;         /* Scope depth */
} Scope;

/* Symbol table structure */
typedef struct {
    Scope* current_scope;    /* Current scope */
    Scope* global_scope;     /* Global scope */
    PCCArray* all_scopes;    /* All scopes */
    PCCArray* errors;        /* Semantic errors */
} SymbolTable;

/* Semantic error structure */
typedef struct {
    char* message;           /* Error message */
    PCCPosition position;    /* Error position */
    int error_code;          /* Error code */
} SemanticError;

/* Error codes */
#define SEM_ERROR_UNDEFINED_SYMBOL 1
#define SEM_ERROR_REDEFINED_SYMBOL 2
#define SEM_ERROR_TYPE_MISMATCH 3
#define SEM_ERROR_INVALID_OPERATION 4
#define SEM_ERROR_MISSING_ARGUMENT 5
#define SEM_ERROR_TOO_MANY_ARGUMENTS 6

/* Function prototypes */

/**
 * @brief Create a new symbol
 * @param name Symbol name
 * @param type Symbol type
 * @param data Associated data
 * @param position Definition position
 * @return New symbol, or NULL on failure
 */
Symbol* symbol_create(const char* name, SymbolType type, void* data, PCCPosition position);

/**
 * @brief Free a symbol
 * @param symbol Symbol to free
 * @param free_data_func Function to free symbol data
 */
void symbol_free(Symbol* symbol, void (*free_data_func)(void*));

/**
 * @brief Create a new scope
 * @param parent Parent scope (NULL for global scope)
 * @param level Scope level
 * @return New scope, or NULL on failure
 */
Scope* scope_create(Scope* parent, int level);

/**
 * @brief Free a scope and all its symbols
 * @param scope Scope to free
 * @param free_data_func Function to free symbol data
 */
void scope_free(Scope* scope, void (*free_data_func)(void*));

/**
 * @brief Create a new symbol table
 * @return New symbol table, or NULL on failure
 */
SymbolTable* symbol_table_create(void);

/**
 * @brief Free symbol table and all resources
 * @param table Symbol table to free
 * @param free_data_func Function to free symbol data
 */
void symbol_table_free(SymbolTable* table, void (*free_data_func)(void*));

/**
 * @brief Enter a new scope
 * @param table Symbol table
 * @return PCC_SUCCESS on success, error code on failure
 */
PCCError symbol_table_enter_scope(SymbolTable* table);

/**
 * @brief Exit current scope
 * @param table Symbol table
 * @return PCC_SUCCESS on success, error code on failure
 */
PCCError symbol_table_exit_scope(SymbolTable* table);

/**
 * @brief Get current scope
 * @param table Symbol table
 * @return Current scope, or NULL if no scope
 */
Scope* symbol_table_current_scope(SymbolTable* table);

/**
 * @brief Get global scope
 * @param table Symbol table
 * @return Global scope, or NULL if no scope
 */
Scope* symbol_table_global_scope(SymbolTable* table);

/**
 * @brief Add symbol to current scope
 * @param table Symbol table
 * @param symbol Symbol to add
 * @return PCC_SUCCESS on success, error code on failure
 */
PCCError symbol_table_add(SymbolTable* table, Symbol* symbol);

/**
 * @brief Look up symbol in current and parent scopes
 * @param table Symbol table
 * @param name Symbol name
 * @return Symbol, or NULL if not found
 */
Symbol* symbol_table_lookup(SymbolTable* table, const char* name);

/**
 * @brief Look up symbol in current scope only
 * @param table Symbol table
 * @param name Symbol name
 * @return Symbol, or NULL if not found
 */
Symbol* symbol_table_lookup_local(SymbolTable* table, const char* name);

/**
 * @brief Check if symbol exists in any scope
 * @param table Symbol table
 * @param name Symbol name
 * @return PCC_TRUE if found, PCC_FALSE otherwise
 */
PCCBool symbol_table_contains(SymbolTable* table, const char* name);

/**
 * @brief Mark symbol as used
 * @param table Symbol table
 * @param name Symbol name
 * @return PCC_SUCCESS on success, error code on failure
 */
PCCError symbol_table_mark_used(SymbolTable* table, const char* name);

/**
 * @brief Get number of semantic errors
 * @param table Symbol table
 * @return Number of errors
 */
size_t symbol_table_error_count(SymbolTable* table);

/**
 * @brief Get error at index
 * @param table Symbol table
 * @param index Error index
 * @return Semantic error, or NULL if index out of bounds
 */
SemanticError* symbol_table_get_error(SymbolTable* table, size_t index);

/**
 * @brief Add semantic error
 * @param table Symbol table
 * @param message Error message
 * @param position Error position
 * @param error_code Error code
 * @return PCC_SUCCESS on success, error code on failure
 */
PCCError symbol_table_add_error(SymbolTable* table, const char* message,
                                 PCCPosition position, int error_code);

/**
 * @brief Print all semantic errors
 * @param table Symbol table
 */
void symbol_table_print_errors(SymbolTable* table);

/**
 * @brief Print symbol table (for debugging)
 * @param table Symbol table
 */
void symbol_table_print(SymbolTable* table);

/**
 * @brief Get symbol type name as string
 * @param type Symbol type
 * @return String representation
 */
const char* symbol_type_name(SymbolType type);

#endif /* PCC_SYMBOL_TABLE_H */
