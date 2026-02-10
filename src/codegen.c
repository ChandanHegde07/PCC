/**
 * @file codegen.c
 * @brief Code generator implementation for PCC compiler
 */

#include "codegen.h"
#include <stdio.h>
#include <string.h>

/* Buffer initial capacity */
#define BUFFER_INITIAL_CAPACITY 4096

/* Helper function to append to buffer */
static PCCError buffer_append(CodeGenerator* generator, const char* str) {
    if (generator == NULL || str == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    size_t len = strlen(str);
    size_t new_size = generator->buffer_size + len;

    /* Resize buffer if needed */
    if (new_size >= generator->buffer_capacity) {
        size_t new_capacity = generator->buffer_capacity * 2;
        while (new_capacity <= new_size) {
            new_capacity *= 2;
        }

        char* new_buffer = (char*)realloc(generator->output_buffer, new_capacity);
        if (new_buffer == NULL) {
            return PCC_ERROR_MEMORY;
        }

        generator->output_buffer = new_buffer;
        generator->buffer_capacity = new_capacity;
    }

    /* Append string */
    memcpy(generator->output_buffer + generator->buffer_size, str, len + 1);
    generator->buffer_size = new_size;

    return PCC_SUCCESS;
}

/* Helper function to append indentation */
static PCCError buffer_append_indent(CodeGenerator* generator) {
    if (generator == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    for (int i = 0; i < generator->indent_level; i++) {
        buffer_append(generator, "  ");
    }

    return PCC_SUCCESS;
}

/* Generate JSON output */
static PCCError generate_json(CodeGenerator* generator, ASTNode* node);

/* Generate text output */
static PCCError generate_text(CodeGenerator* generator, ASTNode* node);

/* Generate markdown output */
static PCCError generate_markdown(CodeGenerator* generator, ASTNode* node);

/* Generate JSON for program */
static PCCError generate_json_program(CodeGenerator* generator, ASTNode* node) {
    if (node == NULL || node->type != AST_PROGRAM) {
        return PCC_ERROR_RUNTIME;
    }

    ASTProgram* prog = (ASTProgram*)node->data;
    if (prog == NULL || prog->statements == NULL) {
        buffer_append(generator, "{\"type\":\"program\",\"statements\":[]}");
        return PCC_SUCCESS;
    }

    buffer_append(generator, "{\"type\":\"program\",\"statements\":[");

    for (size_t i = 0; i < pcc_array_size(prog->statements); i++) {
        ASTNode* stmt = (ASTNode*)pcc_array_get(prog->statements, i);
        generate_json(generator, stmt);
        if (i < pcc_array_size(prog->statements) - 1) {
            buffer_append(generator, ",");
        }
    }

    buffer_append(generator, "]}");
    return PCC_SUCCESS;
}

/* Generate JSON for prompt definition */
static PCCError generate_json_prompt_def(CodeGenerator* generator, ASTNode* node) {
    if (node == NULL || node->type != AST_PROMPT_DEF) {
        return PCC_ERROR_RUNTIME;
    }

    ASTPromptDef* def = (ASTPromptDef*)node->data;
    if (def == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    buffer_append(generator, "{\"type\":\"prompt_def\",\"name\":\"");
    buffer_append(generator, def->name);
    buffer_append(generator, "\",\"body\":");

    if (def->body != NULL) {
        generate_json(generator, def->body);
    } else {
        buffer_append(generator, "null");
    }

    buffer_append(generator, "}");
    return PCC_SUCCESS;
}

/* Generate JSON for text element */
static PCCError generate_json_text_element(CodeGenerator* generator, ASTNode* node) {
    if (node == NULL || node->type != AST_TEXT_ELEMENT) {
        return PCC_ERROR_RUNTIME;
    }

    ASTTextElement* elem = (ASTTextElement*)node->data;
    if (elem == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    buffer_append(generator, "{\"type\":\"text\",\"text\":\"");
    buffer_append(generator, elem->text);
    buffer_append(generator, "\",\"raw\":");
    buffer_append(generator, elem->is_raw ? "true" : "false");
    buffer_append(generator, "}");

    return PCC_SUCCESS;
}

/* Generate JSON for variable reference */
static PCCError generate_json_variable_ref(CodeGenerator* generator, ASTNode* node) {
    if (node == NULL || node->type != AST_VARIABLE_REF) {
        return PCC_ERROR_RUNTIME;
    }

    ASTVariableRef* ref = (ASTVariableRef*)node->data;
    if (ref == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    buffer_append(generator, "{\"type\":\"variable_ref\",\"name\":\"");
    buffer_append(generator, ref->name);
    buffer_append(generator, "\"}");

    return PCC_SUCCESS;
}

/* Generate JSON for function call */
static PCCError generate_json_function_call(CodeGenerator* generator, ASTNode* node) {
    if (node == NULL || (node->type != AST_FUNCTION_CALL && node->type != AST_TEMPLATE_CALL)) {
        return PCC_ERROR_RUNTIME;
    }

    ASTCallExpr* call = (ASTCallExpr*)node->data;
    if (call == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    buffer_append(generator, "{\"type\":\"");
    buffer_append(generator, node->type == AST_TEMPLATE_CALL ? "template_call" : "function_call");
    buffer_append(generator, "\",\"name\":\"");
    buffer_append(generator, call->name);
    buffer_append(generator, "\",\"arguments\":[");

    if (call->arguments != NULL) {
        for (size_t i = 0; i < pcc_array_size(call->arguments); i++) {
            ASTNode* arg = (ASTNode*)pcc_array_get(call->arguments, i);
            generate_json(generator, arg);
            if (i < pcc_array_size(call->arguments) - 1) {
                buffer_append(generator, ",");
            }
        }
    }

    buffer_append(generator, "]}");
    return PCC_SUCCESS;
}

/* Generate JSON for list */
static PCCError generate_json_list(CodeGenerator* generator, ASTNode* node) {
    if (node == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    PCCArray* list = NULL;
    const char* list_type = "list";

    switch (node->type) {
        case AST_STATEMENT_LIST:
            list = (PCCArray*)node->data;
            list_type = "statement_list";
            break;
        case AST_EXPRESSION_LIST:
            list = (PCCArray*)node->data;
            list_type = "expression_list";
            break;
        case AST_ELEMENT_LIST:
            list = (PCCArray*)node->data;
            list_type = "element_list";
            break;
        case AST_ARGUMENT_LIST:
            list = (PCCArray*)node->data;
            list_type = "argument_list";
            break;
        default:
            return PCC_ERROR_RUNTIME;
    }

    if (list == NULL) {
        buffer_append(generator, "{\"type\":\"");
        buffer_append(generator, list_type);
        buffer_append(generator, "\",\"elements\":[]}");
        return PCC_SUCCESS;
    }

    buffer_append(generator, "{\"type\":\"");
    buffer_append(generator, list_type);
    buffer_append(generator, "\",\"elements\":[");

    for (size_t i = 0; i < pcc_array_size(list); i++) {
        ASTNode* elem = (ASTNode*)pcc_array_get(list, i);
        generate_json(generator, elem);
        if (i < pcc_array_size(list) - 1) {
            buffer_append(generator, ",");
        }
    }

    buffer_append(generator, "]}");
    return PCC_SUCCESS;
}

/* Generate JSON for literal */
static PCCError generate_json_literal(CodeGenerator* generator, ASTNode* node) {
    if (node == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    ASTLiteral* lit = (ASTLiteral*)node->data;
    if (lit == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    switch (node->type) {
        case AST_STRING_LITERAL:
            buffer_append(generator, "{\"type\":\"string\",\"value\":\"");
            buffer_append(generator, lit->string_val);
            buffer_append(generator, "\"}");
            break;
        case AST_NUMBER_LITERAL:
            buffer_append(generator, "{\"type\":\"number\",\"value\":");
            char num_buf[64];
            snprintf(num_buf, sizeof(num_buf), "%g", lit->number_val);
            buffer_append(generator, num_buf);
            buffer_append(generator, "}");
            break;
        case AST_BOOLEAN_LITERAL:
            buffer_append(generator, "{\"type\":\"boolean\",\"value\":");
            buffer_append(generator, lit->bool_val ? "true" : "false");
            buffer_append(generator, "}");
            break;
        default:
            buffer_append(generator, "{\"type\":\"unknown\"}");
            break;
    }

    return PCC_SUCCESS;
}

/* Generate JSON output */
static PCCError generate_json(CodeGenerator* generator, ASTNode* node) {
    if (generator == NULL || node == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    switch (node->type) {
        case AST_PROGRAM:
            return generate_json_program(generator, node);
        case AST_PROMPT_DEF:
            return generate_json_prompt_def(generator, node);
        case AST_TEXT_ELEMENT:
            return generate_json_text_element(generator, node);
        case AST_VARIABLE_REF:
            return generate_json_variable_ref(generator, node);
        case AST_FUNCTION_CALL:
        case AST_TEMPLATE_CALL:
            return generate_json_function_call(generator, node);
        case AST_STRING_LITERAL:
        case AST_NUMBER_LITERAL:
        case AST_BOOLEAN_LITERAL:
            return generate_json_literal(generator, node);
        case AST_STATEMENT_LIST:
        case AST_EXPRESSION_LIST:
        case AST_ELEMENT_LIST:
        case AST_ARGUMENT_LIST:
            return generate_json_list(generator, node);
        default:
            buffer_append(generator, "{\"type\":\"");
            buffer_append(generator, ast_node_type_name(node->type));
            buffer_append(generator, "\"}");
            return PCC_SUCCESS;
    }
}

/* Generate text output */
static PCCError generate_text(CodeGenerator* generator, ASTNode* node) {
    if (generator == NULL || node == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    switch (node->type) {
        case AST_PROGRAM: {
            ASTProgram* prog = (ASTProgram*)node->data;
            if (prog != NULL && prog->statements != NULL) {
                for (size_t i = 0; i < pcc_array_size(prog->statements); i++) {
                    ASTNode* stmt = (ASTNode*)pcc_array_get(prog->statements, i);
                    generate_text(generator, stmt);
                    buffer_append(generator, "\n");
                }
            }
            break;
        }
        case AST_PROMPT_DEF: {
            ASTPromptDef* def = (ASTPromptDef*)node->data;
            if (def != NULL) {
                buffer_append(generator, "Prompt: ");
                buffer_append(generator, def->name);
                buffer_append(generator, "\n");
                if (def->body != NULL) {
                    generate_text(generator, def->body);
                }
            }
            break;
        }
        case AST_TEXT_ELEMENT: {
            ASTTextElement* elem = (ASTTextElement*)node->data;
            if (elem != NULL) {
                buffer_append(generator, elem->text);
            }
            break;
        }
        case AST_VARIABLE_REF: {
            ASTVariableRef* ref = (ASTVariableRef*)node->data;
            if (ref != NULL) {
                buffer_append(generator, "$");
                buffer_append(generator, ref->name);
            }
            break;
        }
        case AST_FUNCTION_CALL:
        case AST_TEMPLATE_CALL: {
            ASTCallExpr* call = (ASTCallExpr*)node->data;
            if (call != NULL) {
                buffer_append(generator, "@");
                buffer_append(generator, call->name);
                buffer_append(generator, "(");
                if (call->arguments != NULL) {
                    for (size_t i = 0; i < pcc_array_size(call->arguments); i++) {
                        ASTNode* arg = (ASTNode*)pcc_array_get(call->arguments, i);
                        generate_text(generator, arg);
                        if (i < pcc_array_size(call->arguments) - 1) {
                            buffer_append(generator, ", ");
                        }
                    }
                }
                buffer_append(generator, ")");
            }
            break;
        }
        case AST_ELEMENT_LIST: {
            PCCArray* list = (PCCArray*)node->data;
            if (list != NULL) {
                for (size_t i = 0; i < pcc_array_size(list); i++) {
                    ASTNode* elem = (ASTNode*)pcc_array_get(list, i);
                    generate_text(generator, elem);
                }
            }
            break;
        }
        default:
            break;
    }

    return PCC_SUCCESS;
}

/* Generate markdown output */
static PCCError generate_markdown(CodeGenerator* generator, ASTNode* node) {
    if (generator == NULL || node == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    switch (node->type) {
        case AST_PROGRAM: {
            ASTProgram* prog = (ASTProgram*)node->data;
            if (prog != NULL && prog->statements != NULL) {
                for (size_t i = 0; i < pcc_array_size(prog->statements); i++) {
                    ASTNode* stmt = (ASTNode*)pcc_array_get(prog->statements, i);
                    generate_markdown(generator, stmt);
                    buffer_append(generator, "\n\n");
                }
            }
            break;
        }
        case AST_PROMPT_DEF: {
            ASTPromptDef* def = (ASTPromptDef*)node->data;
            if (def != NULL) {
                buffer_append(generator, "## Prompt: ");
                buffer_append(generator, def->name);
                buffer_append(generator, "\n\n");
                if (def->body != NULL) {
                    generate_markdown(generator, def->body);
                }
            }
            break;
        }
        case AST_TEXT_ELEMENT: {
            ASTTextElement* elem = (ASTTextElement*)node->data;
            if (elem != NULL) {
                buffer_append(generator, elem->text);
            }
            break;
        }
        case AST_VARIABLE_REF: {
            ASTVariableRef* ref = (ASTVariableRef*)node->data;
            if (ref != NULL) {
                buffer_append(generator, "`$");
                buffer_append(generator, ref->name);
                buffer_append(generator, "`");
            }
            break;
        }
        case AST_FUNCTION_CALL:
        case AST_TEMPLATE_CALL: {
            ASTCallExpr* call = (ASTCallExpr*)node->data;
            if (call != NULL) {
                buffer_append(generator, "`@");
                buffer_append(generator, call->name);
                buffer_append(generator, "(");
                if (call->arguments != NULL) {
                    for (size_t i = 0; i < pcc_array_size(call->arguments); i++) {
                        ASTNode* arg = (ASTNode*)pcc_array_get(call->arguments, i);
                        generate_markdown(generator, arg);
                        if (i < pcc_array_size(call->arguments) - 1) {
                            buffer_append(generator, ", ");
                        }
                    }
                }
                buffer_append(generator, ")`");
            }
            break;
        }
        case AST_ELEMENT_LIST: {
            PCCArray* list = (PCCArray*)node->data;
            if (list != NULL) {
                for (size_t i = 0; i < pcc_array_size(list); i++) {
                    ASTNode* elem = (ASTNode*)pcc_array_get(list, i);
                    generate_markdown(generator, elem);
                }
            }
            break;
        }
        default:
            break;
    }

    return PCC_SUCCESS;
}

/* Create a new code generator */
CodeGenerator* codegen_create(OutputFormat format, SymbolTable* symbol_table) {
    CodeGenerator* generator = (CodeGenerator*)malloc(sizeof(CodeGenerator));
    if (generator == NULL) {
        return NULL;
    }

    generator->format = format;
    generator->symbol_table = symbol_table;
    generator->buffer_size = 0;
    generator->buffer_capacity = BUFFER_INITIAL_CAPACITY;
    generator->indent_level = 0;

    generator->output_buffer = (char*)malloc(BUFFER_INITIAL_CAPACITY);
    if (generator->output_buffer == NULL) {
        free(generator);
        return NULL;
    }

    generator->output_buffer[0] = '\0';

    return generator;
}

/* Free code generator and all resources */
void codegen_free(CodeGenerator* generator) {
    if (generator == NULL) {
        return;
    }

    if (generator->output_buffer != NULL) {
        free(generator->output_buffer);
    }

    free(generator);
}

/* Generate code from AST node */
PCCError codegen_generate(CodeGenerator* generator, ASTNode* node) {
    if (generator == NULL || node == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    codegen_clear_output(generator);

    switch (generator->format) {
        case OUTPUT_FORMAT_JSON:
            return generate_json(generator, node);
        case OUTPUT_FORMAT_TEXT:
            return generate_text(generator, node);
        case OUTPUT_FORMAT_MARKDOWN:
            return generate_markdown(generator, node);
        default:
            return PCC_ERROR_RUNTIME;
    }
}

/* Get generated output */
const char* codegen_get_output(CodeGenerator* generator) {
    return generator ? generator->output_buffer : "";
}

/* Clear output buffer */
void codegen_clear_output(CodeGenerator* generator) {
    if (generator == NULL) {
        return;
    }

    generator->buffer_size = 0;
    if (generator->output_buffer != NULL) {
        generator->output_buffer[0] = '\0';
    }
}

/* Write output to file */
PCCError codegen_write_to_file(CodeGenerator* generator, const char* filename) {
    if (generator == NULL || filename == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    FILE* file = fopen(filename, "w");
    if (file == NULL) {
        return PCC_ERROR_IO;
    }

    fprintf(file, "%s", generator->output_buffer);
    fclose(file);

    return PCC_SUCCESS;
}

/* Set output format */
void codegen_set_format(CodeGenerator* generator, OutputFormat format) {
    if (generator != NULL) {
        generator->format = format;
    }
}

/* Get output format */
OutputFormat codegen_get_format(CodeGenerator* generator) {
    return generator ? generator->format : OUTPUT_FORMAT_TEXT;
}
