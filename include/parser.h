/**
 * @file parser.h
 * @brief Parser for PCC compiler
 * @details Converts tokens into Abstract Syntax Tree (AST)
 *          DSA Concept: Recursive descent parsing
 */

#ifndef PCC_PARSER_H
#define PCC_PARSER_H

#include "common.h"
#include "lexer.h"
#include "ast.h"

/* Parser structure */
typedef struct {
    Lexer* lexer;              /* Lexer for token access */
    size_t current;            /* Current token index */
    PCCArray* errors;          /* Parse errors */
    char* error_message;       /* Last error message */
} Parser;

/* Parse error structure */
typedef struct {
    char* message;             /* Error message */
    PCCPosition position;      /* Error position */
} ParseError;

/* Function prototypes */

/**
 * @brief Create a new parser
 * @param lexer Lexer with tokenized source
 * @return Pointer to new parser, or NULL on failure
 */
Parser* parser_create(Lexer* lexer);

/**
 * @brief Free parser and all resources
 * @param parser Parser to free
 */
void parser_free(Parser* parser);

/**
 * @brief Parse the entire program
 * @param parser Parser
 * @return AST node for program, or NULL on failure
 */
ASTNode* parser_parse(Parser* parser);

/**
 * @brief Get number of parse errors
 * @param parser Parser
 * @return Number of errors
 */
size_t parser_error_count(Parser* parser);

/**
 * @brief Get error at index
 * @param parser Parser
 * @param index Error index
 * @return Parse error, or NULL if index out of bounds
 */
ParseError* parser_get_error(Parser* parser, size_t index);

/**
 * @brief Get last error message
 * @param parser Parser
 * @return Error message, or NULL if no error
 */
const char* parser_get_error_message(Parser* parser);

/**
 * @brief Print all parse errors
 * @param parser Parser
 */
void parser_print_errors(Parser* parser);

/**
 * @brief Check if parser has errors
 * @param parser Parser
 * @return PCC_TRUE if errors exist, PCC_FALSE otherwise
 */
PCCBool parser_has_errors(Parser* parser);

#endif /* PCC_PARSER_H */
