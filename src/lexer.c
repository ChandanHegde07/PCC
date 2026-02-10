/**
 * @file lexer.c
 * @brief Lexical analyzer (lexer) implementation for PCC compiler
 */

#include "lexer.h"
#include <stdio.h>
#include <ctype.h>
#include <string.h>

/* Keyword table */
typedef struct {
    const char* keyword;
    TokenType token_type;
} KeywordEntry;

static KeywordEntry keywords[] = {
    {"PROMPT", TOKEN_PROMPT},
    {"VAR", TOKEN_VAR},
    {"TEMPLATE", TOKEN_TEMPLATE},
    {"CONSTRAINT", TOKEN_CONSTRAINT},
    {"OUTPUT", TOKEN_OUTPUT},
    {"IF", TOKEN_IF},
    {"ELSE", TOKEN_ELSE},
    {"FOR", TOKEN_FOR},
    {"WHILE", TOKEN_WHILE},
    {"IN", TOKEN_IN},
    {"AS", TOKEN_AS},
    {"AND", TOKEN_AND},
    {"OR", TOKEN_OR},
    {"NOT", TOKEN_NOT},
    {"RAW", TOKEN_RAW},
    {"true", TOKEN_TRUE},
    {"false", TOKEN_FALSE},
    {NULL, TOKEN_UNKNOWN}
};

/* Helper functions */

static char lexer_peek(Lexer* lexer, size_t offset) {
    if (lexer->position + offset >= lexer->source_len) {
        return '\0';
    }
    return lexer->source[lexer->position + offset];
}

static char lexer_advance(Lexer* lexer) {
    if (lexer->position >= lexer->source_len) {
        return '\0';
    }
    char c = lexer->source[lexer->position++];
    lexer->column++;
    if (c == '\n') {
        lexer->line++;
        lexer->column = 1;
    }
    return c;
}

static void lexer_skip_whitespace(Lexer* lexer) {
    while (lexer->position < lexer->source_len) {
        char c = lexer_peek(lexer, 0);
        if (c == ' ' || c == '\t' || c == '\r' || c == '\n') {
            lexer_advance(lexer);
        } else {
            break;
        }
    }
}

static void lexer_skip_comment(Lexer* lexer) {
    /* Single-line comment: // */
    if (lexer_peek(lexer, 0) == '/' && lexer_peek(lexer, 1) == '/') {
        while (lexer->position < lexer->source_len) {
            char c = lexer_advance(lexer);
            if (c == '\n') {
                break;
            }
        }
    }
    /* Multi-line comment: /* ... * / */
    else if (lexer_peek(lexer, 0) == '/' && lexer_peek(lexer, 1) == '*') {
        lexer_advance(lexer); /* / */
        lexer_advance(lexer); /* * */
        while (lexer->position < lexer->source_len) {
            char c = lexer_advance(lexer);
            if (c == '*' && lexer_peek(lexer, 0) == '/') {
                lexer_advance(lexer); /* / */
                break;
            }
        }
    }
}

static TokenType lexer_check_keyword(const char* lexeme) {
    for (int i = 0; keywords[i].keyword != NULL; i++) {
        if (strcmp(lexeme, keywords[i].keyword) == 0) {
            return keywords[i].token_type;
        }
    }
    return TOKEN_IDENTIFIER;
}

static Token* lexer_create_token(Lexer* lexer, TokenType type, const char* lexeme, size_t lexeme_len) {
    Token* token = (Token*)malloc(sizeof(Token));
    if (token == NULL) {
        return NULL;
    }

    token->type = type;
    token->lexeme = (char*)malloc(lexeme_len + 1);
    if (token->lexeme == NULL) {
        free(token);
        return NULL;
    }
    memcpy(token->lexeme, lexeme, lexeme_len);
    token->lexeme[lexeme_len] = '\0';

    token->position.line = (int)lexer->line;
    token->position.column = (int)lexer->column - (int)lexeme_len;
    token->position.filename = lexer->filename;

    /* Initialize values */
    token->value.string_val = NULL;
    token->value.number_val = 0.0;
    token->value.bool_val = 0;

    return token;
}

static PCCError lexer_read_identifier(Lexer* lexer, Token** out_token) {
    size_t start = lexer->position;
    size_t start_line = lexer->line;
    size_t start_col = lexer->column;

    /* Read first character (must be letter or underscore) */
    char c = lexer_peek(lexer, 0);
    if (!isalpha(c) && c != '_') {
        return PCC_ERROR_SYNTAX;
    }
    lexer_advance(lexer);

    /* Read remaining characters */
    while (lexer->position < lexer->source_len) {
        c = lexer_peek(lexer, 0);
        if (isalnum(c) || c == '_') {
            lexer_advance(lexer);
        } else {
            break;
        }
    }

    size_t len = lexer->position - start;
    char* lexeme = (char*)malloc(len + 1);
    if (lexeme == NULL) {
        return PCC_ERROR_MEMORY;
    }
    memcpy(lexeme, &lexer->source[start], len);
    lexeme[len] = '\0';

    /* Check if it's a keyword */
    TokenType type = lexer_check_keyword(lexeme);

    Token* token = lexer_create_token(lexer, type, lexeme, len);
    free(lexeme);

    if (token == NULL) {
        return PCC_ERROR_MEMORY;
    }

    *out_token = token;
    return PCC_SUCCESS;
}

static PCCError lexer_read_string(Lexer* lexer, Token** out_token) {
    char quote = lexer_peek(lexer, 0); /* " or ' */
    lexer_advance(lexer); /* Skip opening quote */

    size_t start = lexer->position;
    size_t start_line = lexer->line;
    size_t start_col = lexer->column;

    /* Read string content */
    while (lexer->position < lexer->source_len) {
        char c = lexer_peek(lexer, 0);
        if (c == '\\') {
            /* Escape sequence */
            lexer_advance(lexer);
            if (lexer->position < lexer->source_len) {
                lexer_advance(lexer);
            }
        } else if (c == quote) {
            break;
        } else if (c == '\n') {
            /* Unterminated string */
            return PCC_ERROR_SYNTAX;
        } else {
            lexer_advance(lexer);
        }
    }

    if (lexer->position >= lexer->source_len) {
        return PCC_ERROR_SYNTAX; /* Unterminated string */
    }

    size_t len = lexer->position - start;
    char* lexeme = (char*)malloc(len + 1);
    if (lexeme == NULL) {
        return PCC_ERROR_MEMORY;
    }
    memcpy(lexeme, &lexer->source[start], len);
    lexeme[len] = '\0';

    lexer_advance(lexer); /* Skip closing quote */

    Token* token = lexer_create_token(lexer, TOKEN_STRING, &lexer->source[start - 1], len + 2);
    if (token == NULL) {
        free(lexeme);
        return PCC_ERROR_MEMORY;
    }

    /* Store string value */
    token->value.string_val = lexeme;

    *out_token = token;
    return PCC_SUCCESS;
}

static PCCError lexer_read_number(Lexer* lexer, Token** out_token) {
    size_t start = lexer->position;
    size_t start_line = lexer->line;
    size_t start_col = lexer->column;

    /* Read integer part */
    while (lexer->position < lexer->source_len && isdigit(lexer_peek(lexer, 0))) {
        lexer_advance(lexer);
    }

    /* Check for decimal point */
    if (lexer_peek(lexer, 0) == '.') {
        lexer_advance(lexer);
        /* Read fractional part */
        while (lexer->position < lexer->source_len && isdigit(lexer_peek(lexer, 0))) {
            lexer_advance(lexer);
        }
    }

    size_t len = lexer->position - start;
    char* lexeme = (char*)malloc(len + 1);
    if (lexeme == NULL) {
        return PCC_ERROR_MEMORY;
    }
    memcpy(lexeme, &lexer->source[start], len);
    lexeme[len] = '\0';

    Token* token = lexer_create_token(lexer, TOKEN_NUMBER, lexeme, len);
    if (token == NULL) {
        free(lexeme);
        return PCC_ERROR_MEMORY;
    }

    /* Store number value */
    token->value.number_val = atof(lexeme);
    free(lexeme);

    *out_token = token;
    return PCC_SUCCESS;
}

static PCCError lexer_read_operator(Lexer* lexer, Token** out_token) {
    char c = lexer_peek(lexer, 0);
    char next = lexer_peek(lexer, 1);

    TokenType type = TOKEN_UNKNOWN;
    size_t len = 1;

    switch (c) {
        case '=':
            if (next == '=') {
                type = TOKEN_EQ;
                len = 2;
            } else {
                type = TOKEN_ASSIGN;
            }
            break;
        case '!':
            if (next == '=') {
                type = TOKEN_NE;
                len = 2;
            } else {
                type = TOKEN_NOT;
            }
            break;
        case '<':
            if (next == '=') {
                type = TOKEN_LE;
                len = 2;
            } else {
                type = TOKEN_LT;
            }
            break;
        case '>':
            if (next == '=') {
                type = TOKEN_GE;
                len = 2;
            } else {
                type = TOKEN_GT;
            }
            break;
        case '+':
            type = TOKEN_ADD;
            break;
        case '-':
            type = TOKEN_SUB;
            break;
        case '*':
            type = TOKEN_MUL;
            break;
        case '/':
            type = TOKEN_DIV;
            break;
        case '%':
            type = TOKEN_MOD;
            break;
        case '^':
            type = TOKEN_POW;
            break;
        default:
            return PCC_ERROR_SYNTAX;
    }

    Token* token = lexer_create_token(lexer, type, &lexer->source[lexer->position], len);
    if (token == NULL) {
        return PCC_ERROR_MEMORY;
    }

    for (size_t i = 0; i < len; i++) {
        lexer_advance(lexer);
    }

    *out_token = token;
    return PCC_SUCCESS;
}

/* Public functions */

Lexer* lexer_create(const char* source, const char* filename) {
    if (source == NULL) {
        return NULL;
    }

    Lexer* lexer = (Lexer*)malloc(sizeof(Lexer));
    if (lexer == NULL) {
        return NULL;
    }

    lexer->source = source;
    lexer->source_len = strlen(source);
    lexer->position = 0;
    lexer->line = 1;
    lexer->column = 1;
    lexer->filename = filename ? filename : "<unknown>";
    lexer->tokens = pcc_array_create(INITIAL_CAPACITY, sizeof(Token*));
    lexer->error_message = NULL;

    if (lexer->tokens == NULL) {
        free(lexer);
        return NULL;
    }

    return lexer;
}

void lexer_free(Lexer* lexer) {
    if (lexer == NULL) {
        return;
    }

    /* Free all tokens */
    for (size_t i = 0; i < pcc_array_size(lexer->tokens); i++) {
        Token* token = (Token*)pcc_array_get(lexer->tokens, i);
        if (token != NULL) {
            if (token->lexeme != NULL) {
                free(token->lexeme);
            }
            if (token->value.string_val != NULL) {
                free(token->value.string_val);
            }
            free(token);
        }
    }

    pcc_array_free(lexer->tokens, NULL);

    if (lexer->error_message != NULL) {
        free(lexer->error_message);
    }

    free(lexer);
}

PCCError lexer_tokenize(Lexer* lexer) {
    if (lexer == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    while (lexer->position < lexer->source_len) {
        /* Skip whitespace and comments */
        lexer_skip_whitespace(lexer);
        if (lexer_peek(lexer, 0) == '/' && (lexer_peek(lexer, 1) == '/' || lexer_peek(lexer, 1) == '*')) {
            lexer_skip_comment(lexer);
            continue;
        }

        /* Check for end of input */
        if (lexer->position >= lexer->source_len) {
            break;
        }

        char c = lexer_peek(lexer, 0);
        Token* token = NULL;
        PCCError err = PCC_SUCCESS;

        /* Variable reference: $identifier */
        if (c == '$') {
            lexer_advance(lexer);
            err = lexer_read_identifier(lexer, &token);
            if (err == PCC_SUCCESS) {
                token->type = TOKEN_VARIABLE_REF;
            }
        }
        /* Template call: @identifier */
        else if (c == '@') {
            lexer_advance(lexer);
            err = lexer_read_identifier(lexer, &token);
            if (err == PCC_SUCCESS) {
                token->type = TOKEN_TEMPLATE_CALL;
            }
        }
        /* String literal */
        else if (c == '"' || c == '\'') {
            err = lexer_read_string(lexer, &token);
        }
        /* Number literal */
        else if (isdigit(c)) {
            err = lexer_read_number(lexer, &token);
        }
        /* Identifier or keyword */
        else if (isalpha(c) || c == '_') {
            err = lexer_read_identifier(lexer, &token);
        }
        /* Operators */
        else if (c == '=' || c == '!' || c == '<' || c == '>' ||
                 c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^') {
            err = lexer_read_operator(lexer, &token);
        }
        /* Punctuation */
        else {
            TokenType type = TOKEN_UNKNOWN;
            switch (c) {
                case '{': type = TOKEN_LBRACE; break;
                case '}': type = TOKEN_RBRACE; break;
                case '(': type = TOKEN_LPAREN; break;
                case ')': type = TOKEN_RPAREN; break;
                case '[': type = TOKEN_LBRACKET; break;
                case ']': type = TOKEN_RBRACKET; break;
                case ',': type = TOKEN_COMMA; break;
                case ';': type = TOKEN_SEMICOLON; break;
                case ':': type = TOKEN_COLON; break;
                case '.': type = TOKEN_DOT; break;
                default:
                    /* Unknown character */
                    lexer_advance(lexer);
                    continue;
            }

            token = lexer_create_token(lexer, type, &lexer->source[lexer->position], 1);
            if (token == NULL) {
                return PCC_ERROR_MEMORY;
            }
            lexer_advance(lexer);
        }

        if (err != PCC_SUCCESS) {
            /* Create error token */
            token = lexer_create_token(lexer, TOKEN_ERROR, &lexer->source[lexer->position], 1);
            if (token != NULL) {
                pcc_array_push(lexer->tokens, token);
            }
            return err;
        }

        if (token != NULL) {
            pcc_array_push(lexer->tokens, token);
        }
    }

    /* Add EOF token */
    Token* eof_token = lexer_create_token(lexer, TOKEN_EOF, "", 0);
    if (eof_token != NULL) {
        pcc_array_push(lexer->tokens, eof_token);
    }

    return PCC_SUCCESS;
}

Token* lexer_get_token(Lexer* lexer, size_t index) {
    if (lexer == NULL) {
        return NULL;
    }
    return (Token*)pcc_array_get(lexer->tokens, index);
}

const char* lexer_get_error(Lexer* lexer) {
    if (lexer == NULL) {
        return NULL;
    }
    return lexer->error_message;
}

const char* token_type_name(TokenType type) {
    switch (type) {
        case TOKEN_PROMPT: return "PROMPT";
        case TOKEN_VAR: return "VAR";
        case TOKEN_TEMPLATE: return "TEMPLATE";
        case TOKEN_CONSTRAINT: return "CONSTRAINT";
        case TOKEN_OUTPUT: return "OUTPUT";
        case TOKEN_IF: return "IF";
        case TOKEN_ELSE: return "ELSE";
        case TOKEN_FOR: return "FOR";
        case TOKEN_WHILE: return "WHILE";
        case TOKEN_IN: return "IN";
        case TOKEN_AS: return "AS";
        case TOKEN_AND: return "AND";
        case TOKEN_OR: return "OR";
        case TOKEN_NOT: return "NOT";
        case TOKEN_RAW: return "RAW";
        case TOKEN_TRUE: return "TRUE";
        case TOKEN_FALSE: return "FALSE";
        case TOKEN_IDENTIFIER: return "IDENTIFIER";
        case TOKEN_STRING: return "STRING";
        case TOKEN_NUMBER: return "NUMBER";
        case TOKEN_BOOLEAN: return "BOOLEAN";
        case TOKEN_EQ: return "==";
        case TOKEN_NE: return "!=";
        case TOKEN_LT: return "<";
        case TOKEN_GT: return ">";
        case TOKEN_LE: return "<=";
        case TOKEN_GE: return ">=";
        case TOKEN_IN_OP: return "IN";
        case TOKEN_NOT_IN: return "NOT_IN";
        case TOKEN_ADD: return "+";
        case TOKEN_SUB: return "-";
        case TOKEN_MUL: return "*";
        case TOKEN_DIV: return "/";
        case TOKEN_MOD: return "%";
        case TOKEN_POW: return "^";
        case TOKEN_ASSIGN: return "=";
        case TOKEN_LBRACE: return "{";
        case TOKEN_RBRACE: return "}";
        case TOKEN_LPAREN: return "(";
        case TOKEN_RPAREN: return ")";
        case TOKEN_LBRACKET: return "[";
        case TOKEN_RBRACKET: return "]";
        case TOKEN_COMMA: return ",";
        case TOKEN_SEMICOLON: return ";";
        case TOKEN_COLON: return ":";
        case TOKEN_DOT: return ".";
        case TOKEN_VARIABLE_REF: return "VARIABLE_REF";
        case TOKEN_TEMPLATE_CALL: return "TEMPLATE_CALL";
        case TOKEN_EOF: return "EOF";
        case TOKEN_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

void lexer_print_tokens(Lexer* lexer) {
    if (lexer == NULL) {
        return;
    }

    printf("Tokens:\n");
    for (size_t i = 0; i < pcc_array_size(lexer->tokens); i++) {
        Token* token = (Token*)pcc_array_get(lexer->tokens, i);
        printf("  [%zu] %s: '%s' (line %d, col %d)\n",
               i, token_type_name(token->type), token->lexeme,
               token->position.line, token->position.column);
    }
}

void lexer_clear_tokens(Lexer* lexer) {
    if (lexer == NULL) {
        return;
    }

    /* Free all tokens */
    for (size_t i = 0; i < pcc_array_size(lexer->tokens); i++) {
        Token* token = (Token*)pcc_array_get(lexer->tokens, i);
        if (token != NULL) {
            if (token->lexeme != NULL) {
                free(token->lexeme);
            }
            if (token->value.string_val != NULL) {
                free(token->value.string_val);
            }
            free(token);
        }
    }

    pcc_array_clear(lexer->tokens, NULL);
}
