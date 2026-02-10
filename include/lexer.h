/**
 * @file lexer.h
 * @brief Lexical analyzer (lexer) for PCC compiler
 * @details Converts source code into tokens
 *          DSA Concept: Finite automaton for pattern matching
 */

#ifndef PCC_LEXER_H
#define PCC_LEXER_H

#include "common.h"
#include "array.h"

/* Token types */
typedef enum {
    /* Keywords */
    TOKEN_PROMPT = 100,
    TOKEN_VAR,
    TOKEN_TEMPLATE,
    TOKEN_CONSTRAINT,
    TOKEN_OUTPUT,
    TOKEN_IF,
    TOKEN_ELSE,
    TOKEN_FOR,
    TOKEN_WHILE,
    TOKEN_IN,
    TOKEN_AS,
    TOKEN_AND,
    TOKEN_OR,
    TOKEN_NOT,
    TOKEN_RAW,
    TOKEN_TRUE,
    TOKEN_FALSE,

    /* Identifiers and literals */
    TOKEN_IDENTIFIER,
    TOKEN_STRING,
    TOKEN_NUMBER,
    TOKEN_BOOLEAN,

    /* Operators */
    TOKEN_EQ,          /* == */
    TOKEN_NE,          /* != */
    TOKEN_LT,          /* < */
    TOKEN_GT,          /* > */
    TOKEN_LE,          /* <= */
    TOKEN_GE,          /* >= */
    TOKEN_IN_OP,       /* IN */
    TOKEN_NOT_IN,      /* NOT_IN */
    TOKEN_ADD,         /* + */
    TOKEN_SUB,         /* - */
    TOKEN_MUL,         /* * */
    TOKEN_DIV,         /* / */
    TOKEN_MOD,         /* % */
    TOKEN_POW,         /* ^ */
    TOKEN_ASSIGN,      /* = */

    /* Punctuation */
    TOKEN_LBRACE,      /* { */
    TOKEN_RBRACE,      /* } */
    TOKEN_LPAREN,      /* ( */
    TOKEN_RPAREN,      /* ) */
    TOKEN_LBRACKET,    /* [ */
    TOKEN_RBRACKET,    /* ] */
    TOKEN_COMMA,       /* , */
    TOKEN_SEMICOLON,   /* ; */
    TOKEN_COLON,       /* : */
    TOKEN_DOT,         /* . */

    /* Special tokens */
    TOKEN_VARIABLE_REF,  /* $identifier */
    TOKEN_TEMPLATE_CALL, /* @identifier */

    /* End of file and error */
    TOKEN_EOF,
    TOKEN_ERROR,
    TOKEN_UNKNOWN
} TokenType;

/* Token structure */
typedef struct {
    TokenType type;           /* Token type */
    char* lexeme;             /* Token text */
    PCCPosition position;     /* Source position */
    union {
        char* string_val;     /* String value */
        double number_val;    /* Number value */
        int bool_val;         /* Boolean value */
    } value;
} Token;

/* Lexer structure */
typedef struct {
    const char* source;       /* Source code */
    size_t source_len;        /* Source length */
    size_t position;          /* Current position */
    size_t line;              /* Current line number */
    size_t column;            /* Current column number */
    const char* filename;     /* Source filename */
    PCCArray* tokens;         /* Token list */
    char* error_message;      /* Last error message */
} Lexer;

/* Function prototypes */

/**
 * @brief Create a new lexer
 * @param source Source code string
 * @param filename Source filename (for error reporting)
 * @return Pointer to new lexer, or NULL on failure
 */
Lexer* lexer_create(const char* source, const char* filename);

/**
 * @brief Free lexer and all tokens
 * @param lexer Lexer to free
 */
void lexer_free(Lexer* lexer);

/**
 * @brief Tokenize the source code
 * @param lexer Lexer
 * @return PCC_SUCCESS on success, error code on failure
 */
PCCError lexer_tokenize(Lexer* lexer);

/**
 * @brief Get token at index
 * @param lexer Lexer
 * @param index Token index
 * @return Token, or NULL if index out of bounds
 */
Token* lexer_get_token(Lexer* lexer, size_t index);

/**
 * @brief Get number of tokens
 * @param lexer Lexer
 * @return Number of tokens
 */
static inline size_t lexer_token_count(Lexer* lexer) {
    return lexer ? pcc_array_size(lexer->tokens) : 0;
}

/**
 * @brief Get last error message
 * @param lexer Lexer
 * @return Error message, or NULL if no error
 */
const char* lexer_get_error(Lexer* lexer);

/**
 * @brief Get token type name as string
 * @param type Token type
 * @return String representation of token type
 */
const char* token_type_name(TokenType type);

/**
 * @brief Print all tokens (for debugging)
 * @param lexer Lexer
 */
void lexer_print_tokens(Lexer* lexer);

/**
 * @brief Clear all tokens from lexer
 * @param lexer Lexer
 */
void lexer_clear_tokens(Lexer* lexer);

#endif /* PCC_LEXER_H */
