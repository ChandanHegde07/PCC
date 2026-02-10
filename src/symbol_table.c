/**
 * @file symbol_table.c
 * @brief Symbol table implementation for PCC compiler
 */

#include "symbol_table.h"
#include <stdio.h>
#include <string.h>

/* Create a new symbol */
Symbol* symbol_create(const char* name, SymbolType type, void* data, PCCPosition position) {
    if (name == NULL) {
        return NULL;
    }

    Symbol* symbol = (Symbol*)malloc(sizeof(Symbol));
    if (symbol == NULL) {
        return NULL;
    }

    symbol->name = pcc_strdup(name);
    if (symbol->name == NULL) {
        free(symbol);
        return NULL;
    }

    symbol->type = type;
    symbol->data = data;
    symbol->is_defined = 1;
    symbol->is_used = 0;
    symbol->position = position;

    return symbol;
}

/* Free a symbol */
void symbol_free(Symbol* symbol, void (*free_data_func)(void*)) {
    if (symbol == NULL) {
        return;
    }

    if (symbol->name != NULL) {
        free(symbol->name);
    }

    if (free_data_func != NULL && symbol->data != NULL) {
        free_data_func(symbol->data);
    }

    free(symbol);
}

/* Create a new scope */
Scope* scope_create(Scope* parent, int level) {
    Scope* scope = (Scope*)malloc(sizeof(Scope));
    if (scope == NULL) {
        return NULL;
    }

    scope->symbols = pcc_hashtable_create(INITIAL_CAPACITY, NULL);
    if (scope->symbols == NULL) {
        free(scope);
        return NULL;
    }

    scope->parent = parent;
    scope->scope_level = level;

    return scope;
}

/* Free a scope and all its symbols */
void scope_free(Scope* scope, void (*free_data_func)(void*)) {
    if (scope == NULL) {
        return;
    }

    /* Free all symbols in scope */
    PCCHashTableIterator iter = pcc_hashtable_iterator_create(scope->symbols);
    char* key;
    void* value;

    while (pcc_hashtable_iterator_next(&iter, &key, &value)) {
        Symbol* symbol = (Symbol*)value;
        symbol_free(symbol, free_data_func);
    }

    pcc_hashtable_free(scope->symbols, NULL);
    free(scope);
}

/* Create a new symbol table */
SymbolTable* symbol_table_create(void) {
    SymbolTable* table = (SymbolTable*)malloc(sizeof(SymbolTable));
    if (table == NULL) {
        return NULL;
    }

    /* Create global scope */
    table->global_scope = scope_create(NULL, 0);
    if (table->global_scope == NULL) {
        free(table);
        return NULL;
    }

    table->current_scope = table->global_scope;
    table->all_scopes = pcc_array_create(INITIAL_CAPACITY, sizeof(Scope*));
    table->errors = pcc_array_create(INITIAL_CAPACITY, sizeof(SemanticError*));

    if (table->all_scopes == NULL || table->errors == NULL) {
        scope_free(table->global_scope, NULL);
        if (table->all_scopes) pcc_array_free(table->all_scopes, NULL);
        if (table->errors) pcc_array_free(table->errors, NULL);
        free(table);
        return NULL;
    }

    /* Add global scope to all scopes */
    pcc_array_push(table->all_scopes, table->global_scope);

    return table;
}

/* Free symbol table and all resources */
void symbol_table_free(SymbolTable* table, void (*free_data_func)(void*)) {
    if (table == NULL) {
        return;
    }

    /* Free all scopes */
    for (size_t i = 0; i < pcc_array_size(table->all_scopes); i++) {
        Scope* scope = (Scope*)pcc_array_get(table->all_scopes, i);
        scope_free(scope, free_data_func);
    }

    pcc_array_free(table->all_scopes, NULL);

    /* Free all errors */
    for (size_t i = 0; i < pcc_array_size(table->errors); i++) {
        SemanticError* error = (SemanticError*)pcc_array_get(table->errors, i);
        if (error != NULL) {
            if (error->message != NULL) {
                free(error->message);
            }
            free(error);
        }
    }

    pcc_array_free(table->errors, NULL);

    free(table);
}

/* Enter a new scope */
PCCError symbol_table_enter_scope(SymbolTable* table) {
    if (table == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    Scope* new_scope = scope_create(table->current_scope,
                                     table->current_scope->scope_level + 1);
    if (new_scope == NULL) {
        return PCC_ERROR_MEMORY;
    }

    table->current_scope = new_scope;
    pcc_array_push(table->all_scopes, new_scope);

    return PCC_SUCCESS;
}

/* Exit current scope */
PCCError symbol_table_exit_scope(SymbolTable* table) {
    if (table == NULL || table->current_scope == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    if (table->current_scope->parent == NULL) {
        /* Cannot exit global scope */
        return PCC_ERROR_RUNTIME;
    }

    table->current_scope = table->current_scope->parent;

    return PCC_SUCCESS;
}

/* Get current scope */
Scope* symbol_table_current_scope(SymbolTable* table) {
    return table ? table->current_scope : NULL;
}

/* Get global scope */
Scope* symbol_table_global_scope(SymbolTable* table) {
    return table ? table->global_scope : NULL;
}

/* Add symbol to current scope */
PCCError symbol_table_add(SymbolTable* table, Symbol* symbol) {
    if (table == NULL || symbol == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    /* Check if symbol already exists in current scope */
    if (pcc_hashtable_contains(table->current_scope->symbols, symbol->name)) {
        /* Symbol redefinition error */
        char error_msg[512];
        snprintf(error_msg, sizeof(error_msg),
                 "Symbol '%s' already defined in this scope", symbol->name);
        symbol_table_add_error(table, error_msg, symbol->position,
                               SEM_ERROR_REDEFINED_SYMBOL);
        return PCC_ERROR_SEMANTIC;
    }

    /* Add symbol to current scope */
    PCCError err = pcc_hashtable_put(table->current_scope->symbols,
                                      symbol->name, symbol, NULL);
    if (err != PCC_SUCCESS) {
        return err;
    }

    return PCC_SUCCESS;
}

/* Look up symbol in current and parent scopes */
Symbol* symbol_table_lookup(SymbolTable* table, const char* name) {
    if (table == NULL || name == NULL) {
        return NULL;
    }

    Scope* scope = table->current_scope;

    while (scope != NULL) {
        Symbol* symbol = (Symbol*)pcc_hashtable_get(scope->symbols, name);
        if (symbol != NULL) {
            return symbol;
        }
        scope = scope->parent;
    }

    return NULL;
}

/* Look up symbol in current scope only */
Symbol* symbol_table_lookup_local(SymbolTable* table, const char* name) {
    if (table == NULL || name == NULL) {
        return NULL;
    }

    return (Symbol*)pcc_hashtable_get(table->current_scope->symbols, name);
}

/* Check if symbol exists in any scope */
PCCBool symbol_table_contains(SymbolTable* table, const char* name) {
    return (PCCBool)(symbol_table_lookup(table, name) != NULL);
}

/* Mark symbol as used */
PCCError symbol_table_mark_used(SymbolTable* table, const char* name) {
    if (table == NULL || name == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    Symbol* symbol = symbol_table_lookup(table, name);
    if (symbol == NULL) {
        /* Undefined symbol error */
        PCCPosition pos = {0, 0, "<unknown>"};
        char error_msg[512];
        snprintf(error_msg, sizeof(error_msg),
                 "Undefined symbol '%s'", name);
        symbol_table_add_error(table, error_msg, pos,
                               SEM_ERROR_UNDEFINED_SYMBOL);
        return PCC_ERROR_SEMANTIC;
    }

    symbol->is_used = 1;
    return PCC_SUCCESS;
}

/* Get number of semantic errors */
size_t symbol_table_error_count(SymbolTable* table) {
    return table ? pcc_array_size(table->errors) : 0;
}

/* Get error at index */
SemanticError* symbol_table_get_error(SymbolTable* table, size_t index) {
    if (table == NULL) {
        return NULL;
    }
    return (SemanticError*)pcc_array_get(table->errors, index);
}

/* Add semantic error */
PCCError symbol_table_add_error(SymbolTable* table, const char* message,
                                 PCCPosition position, int error_code) {
    if (table == NULL || message == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    SemanticError* error = (SemanticError*)malloc(sizeof(SemanticError));
    if (error == NULL) {
        return PCC_ERROR_MEMORY;
    }

    error->message = pcc_strdup(message);
    if (error->message == NULL) {
        free(error);
        return PCC_ERROR_MEMORY;
    }

    error->position = position;
    error->error_code = error_code;

    pcc_array_push(table->errors, error);

    return PCC_SUCCESS;
}

/* Print all semantic errors */
void symbol_table_print_errors(SymbolTable* table) {
    if (table == NULL) {
        return;
    }

    for (size_t i = 0; i < pcc_array_size(table->errors); i++) {
        SemanticError* error = (SemanticError*)pcc_array_get(table->errors, i);
        if (error != NULL) {
            printf("Semantic error at line %d, column %d: %s\n",
                   error->position.line, error->position.column, error->message);
        }
    }
}

/* Print symbol table (for debugging) */
static void scope_print(Scope* scope, int indent) {
    if (scope == NULL) {
        return;
    }

    /* Print indentation */
    for (int i = 0; i < indent; i++) {
        printf("  ");
    }

    printf("Scope (level %d):\n", scope->scope_level);

    /* Print symbols */
    PCCHashTableIterator iter = pcc_hashtable_iterator_create(scope->symbols);
    char* key;
    void* value;

    while (pcc_hashtable_iterator_next(&iter, &key, &value)) {
        Symbol* symbol = (Symbol*)value;

        for (int i = 0; i < indent + 1; i++) {
            printf("  ");
        }

        printf("%s: %s (defined: %d, used: %d)\n",
               symbol->name,
               symbol_type_name(symbol->type),
               symbol->is_defined,
               symbol->is_used);
    }
}

void symbol_table_print(SymbolTable* table) {
    if (table == NULL) {
        return;
    }

    printf("Symbol Table:\n");

    for (size_t i = 0; i < pcc_array_size(table->all_scopes); i++) {
        Scope* scope = (Scope*)pcc_array_get(table->all_scopes, i);
        scope_print(scope, 1);
    }
}

/* Get symbol type name as string */
const char* symbol_type_name(SymbolType type) {
    switch (type) {
        case SYMBOL_VARIABLE: return "VARIABLE";
        case SYMBOL_TEMPLATE: return "TEMPLATE";
        case SYMBOL_PROMPT: return "PROMPT";
        case SYMBOL_CONSTRAINT: return "CONSTRAINT";
        case SYMBOL_PARAMETER: return "PARAMETER";
        default: return "UNKNOWN";
    }
}
