/**
 * @file semantic.h
 * @brief Semantic analyzer for PCC compiler
 * @details Performs type checking, scope validation, and semantic analysis
 *          DSA Concept: Tree traversal (AST visitor pattern)
 */

#ifndef PCC_SEMANTIC_H
#define PCC_SEMANTIC_H

#include "common.h"
#include "ast.h"
#include "symbol_table.h"

/* Semantic analyzer structure */
typedef struct {
    SymbolTable* symbol_table;  /* Symbol table for tracking symbols */
    PCCArray* errors;           /* Semantic errors */
    int has_errors;             /* Whether errors occurred */
} SemanticAnalyzer;

/* Function prototypes */

/**
 * @brief Create a new semantic analyzer
 * @return New semantic analyzer, or NULL on failure
 */
SemanticAnalyzer* semantic_analyzer_create(void);

/**
 * @brief Free semantic analyzer and all resources
 * @param analyzer Analyzer to free
 */
void semantic_analyzer_free(SemanticAnalyzer* analyzer);

/**
 * @brief Analyze an AST node
 * @param analyzer Semantic analyzer
 * @param node AST node to analyze
 * @return PCC_SUCCESS on success, error code on failure
 */
PCCError semantic_analyze(SemanticAnalyzer* analyzer, ASTNode* node);

/**
 * @brief Get number of semantic errors
 * @param analyzer Semantic analyzer
 * @return Number of errors
 */
size_t semantic_error_count(SemanticAnalyzer* analyzer);

/**
 * @brief Get error at index
 * @param analyzer Semantic analyzer
 * @param index Error index
 * @return Semantic error, or NULL if index out of bounds
 */
SemanticError* semantic_get_error(SemanticAnalyzer* analyzer, size_t index);

/**
 * @brief Print all semantic errors
 * @param analyzer Semantic analyzer
 */
void semantic_print_errors(SemanticAnalyzer* analyzer);

/**
 * @brief Check if analyzer has errors
 * @param analyzer Semantic analyzer
 * @return PCC_TRUE if errors exist, PCC_FALSE otherwise
 */
PCCBool semantic_has_errors(SemanticAnalyzer* analyzer);

/**
 * @brief Get symbol table
 * @param analyzer Semantic analyzer
 * @return Symbol table
 */
SymbolTable* semantic_get_symbol_table(SemanticAnalyzer* analyzer);

#endif /* PCC_SEMANTIC_H */
