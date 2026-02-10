/**
 * @file codegen.h
 * @brief Code generator for PCC compiler
 * @details Generates JSON/text output from AST for LLM prompts
 *          DSA Concept: Tree traversal for code generation
 */

#ifndef PCC_CODEGEN_H
#define PCC_CODEGEN_H

#include "common.h"
#include "ast.h"
#include "symbol_table.h"

/* Output format types */
typedef enum {
    OUTPUT_FORMAT_JSON,
    OUTPUT_FORMAT_TEXT,
    OUTPUT_FORMAT_MARKDOWN
} OutputFormat;

/* Code generator structure */
typedef struct {
    OutputFormat format;       /* Output format */
    SymbolTable* symbol_table; /* Symbol table for variable lookup */
    char* output_buffer;       /* Output buffer */
    size_t buffer_size;        /* Current buffer size */
    size_t buffer_capacity;    /* Buffer capacity */
    int indent_level;          /* Current indentation level */
} CodeGenerator;

/* Function prototypes */

/**
 * @brief Create a new code generator
 * @param format Output format
 * @param symbol_table Symbol table for variable lookup
 * @return New code generator, or NULL on failure
 */
CodeGenerator* codegen_create(OutputFormat format, SymbolTable* symbol_table);

/**
 * @brief Free code generator and all resources
 * @param generator Code generator to free
 */
void codegen_free(CodeGenerator* generator);

/**
 * @brief Generate code from AST node
 * @param generator Code generator
 * @param node AST node to generate code from
 * @return PCC_SUCCESS on success, error code on failure
 */
PCCError codegen_generate(CodeGenerator* generator, ASTNode* node);

/**
 * @brief Get generated output
 * @param generator Code generator
 * @return Output string (owned by generator)
 */
const char* codegen_get_output(CodeGenerator* generator);

/**
 * @brief Clear output buffer
 * @param generator Code generator
 */
void codegen_clear_output(CodeGenerator* generator);

/**
 * @brief Write output to file
 * @param generator Code generator
 * @param filename Output filename
 * @return PCC_SUCCESS on success, error code on failure
 */
PCCError codegen_write_to_file(CodeGenerator* generator, const char* filename);

/**
 * @brief Set output format
 * @param generator Code generator
 * @param format New output format
 */
void codegen_set_format(CodeGenerator* generator, OutputFormat format);

/**
 * @brief Get output format
 * @param generator Code generator
 * @return Current output format
 */
OutputFormat codegen_get_format(CodeGenerator* generator);

#endif /* PCC_CODEGEN_H */
