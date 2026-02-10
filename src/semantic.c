/**
 * @file semantic.c
 * @brief Semantic analyzer implementation for PCC compiler
 */

#include "semantic.h"
#include <stdio.h>
#include <string.h>

/* Forward declarations for analysis functions */
static PCCError analyze_program(SemanticAnalyzer* analyzer, ASTNode* node);
static PCCError analyze_prompt_def(SemanticAnalyzer* analyzer, ASTNode* node);
static PCCError analyze_var_decl(SemanticAnalyzer* analyzer, ASTNode* node);
static PCCError analyze_template_def(SemanticAnalyzer* analyzer, ASTNode* node);
static PCCError analyze_constraint_def(SemanticAnalyzer* analyzer, ASTNode* node);
static PCCError analyze_output_spec(SemanticAnalyzer* analyzer, ASTNode* node);
static PCCError analyze_expression(SemanticAnalyzer* analyzer, ASTNode* node);
static PCCError analyze_identifier(SemanticAnalyzer* analyzer, ASTNode* node);
static PCCError analyze_variable_ref(SemanticAnalyzer* analyzer, ASTNode* node);
static PCCError analyze_function_call(SemanticAnalyzer* analyzer, ASTNode* node);
static PCCError analyze_if_stmt(SemanticAnalyzer* analyzer, ASTNode* node);
static PCCError analyze_for_stmt(SemanticAnalyzer* analyzer, ASTNode* node);
static PCCError analyze_while_stmt(SemanticAnalyzer* analyzer, ASTNode* node);
static PCCError analyze_element_list(SemanticAnalyzer* analyzer, ASTNode* node);

/* Helper function to add error */
static void add_error(SemanticAnalyzer* analyzer, const char* message, PCCPosition position, int error_code) {
    symbol_table_add_error(analyzer->symbol_table, message, position, error_code);
    analyzer->has_errors = 1;
}

/* Create a new semantic analyzer */
SemanticAnalyzer* semantic_analyzer_create(void) {
    SemanticAnalyzer* analyzer = (SemanticAnalyzer*)malloc(sizeof(SemanticAnalyzer));
    if (analyzer == NULL) {
        return NULL;
    }

    analyzer->symbol_table = symbol_table_create();
    if (analyzer->symbol_table == NULL) {
        free(analyzer);
        return NULL;
    }

    analyzer->errors = analyzer->symbol_table->errors;
    analyzer->has_errors = 0;

    return analyzer;
}

/* Free semantic analyzer and all resources */
void semantic_analyzer_free(SemanticAnalyzer* analyzer) {
    if (analyzer == NULL) {
        return;
    }

    if (analyzer->symbol_table != NULL) {
        symbol_table_free(analyzer->symbol_table, NULL);
    }

    free(analyzer);
}

/* Analyze an AST node */
PCCError semantic_analyze(SemanticAnalyzer* analyzer, ASTNode* node) {
    if (analyzer == NULL || node == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    switch (node->type) {
        case AST_PROGRAM:
            return analyze_program(analyzer, node);
        case AST_PROMPT_DEF:
            return analyze_prompt_def(analyzer, node);
        case AST_VAR_DECL:
            return analyze_var_decl(analyzer, node);
        case AST_TEMPLATE_DEF:
            return analyze_template_def(analyzer, node);
        case AST_CONSTRAINT_DEF:
            return analyze_constraint_def(analyzer, node);
        case AST_OUTPUT_SPEC:
            return analyze_output_spec(analyzer, node);
        case AST_IDENTIFIER:
            return analyze_identifier(analyzer, node);
        case AST_STRING_LITERAL:
        case AST_NUMBER_LITERAL:
        case AST_BOOLEAN_LITERAL:
            return PCC_SUCCESS; /* Literals are always valid */
        case AST_BINARY_EXPR:
        case AST_UNARY_EXPR:
            return analyze_expression(analyzer, node);
        case AST_VARIABLE_REF:
            return analyze_variable_ref(analyzer, node);
        case AST_FUNCTION_CALL:
        case AST_TEMPLATE_CALL:
            return analyze_function_call(analyzer, node);
        case AST_IF_STMT:
            return analyze_if_stmt(analyzer, node);
        case AST_FOR_STMT:
            return analyze_for_stmt(analyzer, node);
        case AST_WHILE_STMT:
            return analyze_while_stmt(analyzer, node);
        case AST_TEXT_ELEMENT:
            return PCC_SUCCESS; /* Text elements are always valid */
        case AST_CONSTRAINT_EXPR:
            return analyze_expression(analyzer, node);
        case AST_STATEMENT_LIST:
        case AST_EXPRESSION_LIST:
        case AST_ELEMENT_LIST:
        case AST_ARGUMENT_LIST:
        case AST_PARAMETER_LIST:
        case AST_CONSTRAINT_LIST:
            return analyze_element_list(analyzer, node);
        default:
            return PCC_SUCCESS;
    }
}

/* Analyze program */
static PCCError analyze_program(SemanticAnalyzer* analyzer, ASTNode* node) {
    if (node->type != AST_PROGRAM) {
        return PCC_ERROR_RUNTIME;
    }

    ASTProgram* prog = (ASTProgram*)node->data;
    if (prog == NULL || prog->statements == NULL) {
        return PCC_SUCCESS;
    }

    /* Analyze all statements */
    for (size_t i = 0; i < pcc_array_size(prog->statements); i++) {
        ASTNode* stmt = (ASTNode*)pcc_array_get(prog->statements, i);
        PCCError err = semantic_analyze(analyzer, stmt);
        if (err != PCC_SUCCESS) {
            return err;
        }
    }

    return PCC_SUCCESS;
}

/* Analyze prompt definition */
static PCCError analyze_prompt_def(SemanticAnalyzer* analyzer, ASTNode* node) {
    if (node->type != AST_PROMPT_DEF) {
        return PCC_ERROR_RUNTIME;
    }

    ASTPromptDef* def = (ASTPromptDef*)node->data;
    if (def == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    /* Add prompt to symbol table */
    Symbol* symbol = symbol_create(def->name, SYMBOL_PROMPT, node, node->position);
    if (symbol == NULL) {
        return PCC_ERROR_MEMORY;
    }

    PCCError err = symbol_table_add(analyzer->symbol_table, symbol);
    if (err != PCC_SUCCESS) {
        symbol_free(symbol, NULL);
        return err;
    }

    /* Analyze body */
    if (def->body != NULL) {
        return semantic_analyze(analyzer, def->body);
    }

    return PCC_SUCCESS;
}

/* Analyze variable declaration */
static PCCError analyze_var_decl(SemanticAnalyzer* analyzer, ASTNode* node) {
    if (node->type != AST_VAR_DECL) {
        return PCC_ERROR_RUNTIME;
    }

    ASTVarDecl* decl = (ASTVarDecl*)node->data;
    if (decl == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    /* Add variable to symbol table */
    Symbol* symbol = symbol_create(decl->name, SYMBOL_VARIABLE, node, node->position);
    if (symbol == NULL) {
        return PCC_ERROR_MEMORY;
    }

    PCCError err = symbol_table_add(analyzer->symbol_table, symbol);
    if (err != PCC_SUCCESS) {
        symbol_free(symbol, NULL);
        return err;
    }

    /* Analyze initializer */
    if (decl->initializer != NULL) {
        return semantic_analyze(analyzer, decl->initializer);
    }

    return PCC_SUCCESS;
}

/* Analyze template definition */
static PCCError analyze_template_def(SemanticAnalyzer* analyzer, ASTNode* node) {
    if (node->type != AST_TEMPLATE_DEF) {
        return PCC_ERROR_RUNTIME;
    }

    ASTTemplateDef* def = (ASTTemplateDef*)node->data;
    if (def == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    /* Add template to symbol table */
    Symbol* symbol = symbol_create(def->name, SYMBOL_TEMPLATE, node, node->position);
    if (symbol == NULL) {
        return PCC_ERROR_MEMORY;
    }

    PCCError err = symbol_table_add(analyzer->symbol_table, symbol);
    if (err != PCC_SUCCESS) {
        symbol_free(symbol, NULL);
        return err;
    }

    /* Enter new scope for template parameters */
    symbol_table_enter_scope(analyzer->symbol_table);

    /* Add parameters to symbol table */
    if (def->parameters != NULL) {
        for (size_t i = 0; i < pcc_array_size(def->parameters); i++) {
            char* param_name = (char*)pcc_array_get(def->parameters, i);
            Symbol* param_symbol = symbol_create(param_name, SYMBOL_PARAMETER, NULL, node->position);
            if (param_symbol != NULL) {
                symbol_table_add(analyzer->symbol_table, param_symbol);
            }
        }
    }

    /* Analyze body */
    if (def->body != NULL) {
        err = semantic_analyze(analyzer, def->body);
    }

    /* Exit template scope */
    symbol_table_exit_scope(analyzer->symbol_table);

    return err;
}

/* Analyze constraint definition */
static PCCError analyze_constraint_def(SemanticAnalyzer* analyzer, ASTNode* node) {
    if (node->type != AST_CONSTRAINT_DEF) {
        return PCC_ERROR_RUNTIME;
    }

    ASTConstraintDef* def = (ASTConstraintDef*)node->data;
    if (def == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    /* Add constraint to symbol table */
    Symbol* symbol = symbol_create(def->name, SYMBOL_CONSTRAINT, node, node->position);
    if (symbol == NULL) {
        return PCC_ERROR_MEMORY;
    }

    PCCError err = symbol_table_add(analyzer->symbol_table, symbol);
    if (err != PCC_SUCCESS) {
        symbol_free(symbol, NULL);
        return err;
    }

    /* Analyze constraints */
    if (def->constraints != NULL) {
        for (size_t i = 0; i < pcc_array_size(def->constraints); i++) {
            ASTNode* constraint = (ASTNode*)pcc_array_get(def->constraints, i);
            err = semantic_analyze(analyzer, constraint);
            if (err != PCC_SUCCESS) {
                return err;
            }
        }
    }

    return PCC_SUCCESS;
}

/* Analyze output specification */
static PCCError analyze_output_spec(SemanticAnalyzer* analyzer, ASTNode* node) {
    if (node->type != AST_OUTPUT_SPEC) {
        return PCC_ERROR_RUNTIME;
    }

    ASTOutputSpec* spec = (ASTOutputSpec*)node->data;
    if (spec == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    /* Check if referenced prompt exists */
    Symbol* symbol = symbol_table_lookup(analyzer->symbol_table, spec->name);
    if (symbol == NULL) {
        char error_msg[512];
        snprintf(error_msg, sizeof(error_msg),
                 "Undefined prompt '%s' in OUTPUT specification", spec->name);
        add_error(analyzer, error_msg, node->position, SEM_ERROR_UNDEFINED_SYMBOL);
        return PCC_ERROR_SEMANTIC;
    }

    if (symbol->type != SYMBOL_PROMPT) {
        char error_msg[512];
        snprintf(error_msg, sizeof(error_msg),
                 "'%s' is not a prompt in OUTPUT specification", spec->name);
        add_error(analyzer, error_msg, node->position, SEM_ERROR_TYPE_MISMATCH);
        return PCC_ERROR_SEMANTIC;
    }

    return PCC_SUCCESS;
}

/* Analyze expression */
static PCCError analyze_expression(SemanticAnalyzer* analyzer, ASTNode* node) {
    if (node == NULL) {
        return PCC_SUCCESS;
    }

    switch (node->type) {
        case AST_BINARY_EXPR: {
            ASTBinaryExpr* expr = (ASTBinaryExpr*)node->data;
            if (expr != NULL) {
                semantic_analyze(analyzer, expr->left);
                semantic_analyze(analyzer, expr->right);
            }
            break;
        }
        case AST_UNARY_EXPR: {
            ASTUnaryExpr* expr = (ASTUnaryExpr*)node->data;
            if (expr != NULL) {
                semantic_analyze(analyzer, expr->operand);
            }
            break;
        }
        default:
            return semantic_analyze(analyzer, node);
    }

    return PCC_SUCCESS;
}

/* Analyze identifier */
static PCCError analyze_identifier(SemanticAnalyzer* analyzer, ASTNode* node) {
    if (node->type != AST_IDENTIFIER) {
        return PCC_ERROR_RUNTIME;
    }

    ASTIdentifier* id = (ASTIdentifier*)node->data;
    if (id == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    /* Check if identifier is defined */
    Symbol* symbol = symbol_table_lookup(analyzer->symbol_table, id->name);
    if (symbol == NULL) {
        char error_msg[512];
        snprintf(error_msg, sizeof(error_msg),
                 "Undefined identifier '%s'", id->name);
        add_error(analyzer, error_msg, node->position, SEM_ERROR_UNDEFINED_SYMBOL);
        return PCC_ERROR_SEMANTIC;
    }

    return PCC_SUCCESS;
}

/* Analyze variable reference */
static PCCError analyze_variable_ref(SemanticAnalyzer* analyzer, ASTNode* node) {
    if (node->type != AST_VARIABLE_REF) {
        return PCC_ERROR_RUNTIME;
    }

    ASTVariableRef* ref = (ASTVariableRef*)node->data;
    if (ref == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    /* Check if variable is defined */
    Symbol* symbol = symbol_table_lookup(analyzer->symbol_table, ref->name);
    if (symbol == NULL) {
        char error_msg[512];
        snprintf(error_msg, sizeof(error_msg),
                 "Undefined variable '$%s'", ref->name);
        add_error(analyzer, error_msg, node->position, SEM_ERROR_UNDEFINED_SYMBOL);
        return PCC_ERROR_SEMANTIC;
    }

    if (symbol->type != SYMBOL_VARIABLE && symbol->type != SYMBOL_PARAMETER) {
        char error_msg[512];
        snprintf(error_msg, sizeof(error_msg),
                 "'$%s' is not a variable", ref->name);
        add_error(analyzer, error_msg, node->position, SEM_ERROR_TYPE_MISMATCH);
        return PCC_ERROR_SEMANTIC;
    }

    /* Mark as used */
    symbol_table_mark_used(analyzer->symbol_table, ref->name);

    return PCC_SUCCESS;
}

/* Analyze function call */
static PCCError analyze_function_call(SemanticAnalyzer* analyzer, ASTNode* node) {
    if (node->type != AST_FUNCTION_CALL && node->type != AST_TEMPLATE_CALL) {
        return PCC_ERROR_RUNTIME;
    }

    ASTCallExpr* call = (ASTCallExpr*)node->data;
    if (call == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    /* Check if function/template is defined */
    Symbol* symbol = symbol_table_lookup(analyzer->symbol_table, call->name);
    if (symbol == NULL) {
        char error_msg[512];
        snprintf(error_msg, sizeof(error_msg),
                 "Undefined template '%s'", call->name);
        add_error(analyzer, error_msg, node->position, SEM_ERROR_UNDEFINED_SYMBOL);
        return PCC_ERROR_SEMANTIC;
    }

    if (symbol->type != SYMBOL_TEMPLATE) {
        char error_msg[512];
        snprintf(error_msg, sizeof(error_msg),
                 "'%s' is not a template", call->name);
        add_error(analyzer, error_msg, node->position, SEM_ERROR_TYPE_MISMATCH);
        return PCC_ERROR_SEMANTIC;
    }

    /* Analyze arguments */
    if (call->arguments != NULL) {
        for (size_t i = 0; i < pcc_array_size(call->arguments); i++) {
            ASTNode* arg = (ASTNode*)pcc_array_get(call->arguments, i);
            semantic_analyze(analyzer, arg);
        }
    }

    return PCC_SUCCESS;
}

/* Analyze if statement */
static PCCError analyze_if_stmt(SemanticAnalyzer* analyzer, ASTNode* node) {
    if (node->type != AST_IF_STMT) {
        return PCC_ERROR_RUNTIME;
    }

    ASTIfStmt* stmt = (ASTIfStmt*)node->data;
    if (stmt == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    /* Analyze condition */
    if (stmt->condition != NULL) {
        semantic_analyze(analyzer, stmt->condition);
    }

    /* Analyze then body */
    if (stmt->then_body != NULL) {
        semantic_analyze(analyzer, stmt->then_body);
    }

    /* Analyze else body */
    if (stmt->else_body != NULL) {
        semantic_analyze(analyzer, stmt->else_body);
    }

    return PCC_SUCCESS;
}

/* Analyze for statement */
static PCCError analyze_for_stmt(SemanticAnalyzer* analyzer, ASTNode* node) {
    if (node->type != AST_FOR_STMT) {
        return PCC_ERROR_RUNTIME;
    }

    ASTForStmt* stmt = (ASTForStmt*)node->data;
    if (stmt == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    /* Enter new scope for loop variable */
    symbol_table_enter_scope(analyzer->symbol_table);

    /* Add loop variable to symbol table */
    Symbol* loop_var = symbol_create(stmt->variable, SYMBOL_VARIABLE, NULL, node->position);
    if (loop_var != NULL) {
        symbol_table_add(analyzer->symbol_table, loop_var);
    }

    /* Analyze iterable */
    if (stmt->iterable != NULL) {
        semantic_analyze(analyzer, stmt->iterable);
    }

    /* Analyze body */
    if (stmt->body != NULL) {
        semantic_analyze(analyzer, stmt->body);
    }

    /* Exit loop scope */
    symbol_table_exit_scope(analyzer->symbol_table);

    return PCC_SUCCESS;
}

/* Analyze while statement */
static PCCError analyze_while_stmt(SemanticAnalyzer* analyzer, ASTNode* node) {
    if (node->type != AST_WHILE_STMT) {
        return PCC_ERROR_RUNTIME;
    }

    ASTWhileStmt* stmt = (ASTWhileStmt*)node->data;
    if (stmt == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    /* Analyze condition */
    if (stmt->condition != NULL) {
        semantic_analyze(analyzer, stmt->condition);
    }

    /* Analyze body */
    if (stmt->body != NULL) {
        semantic_analyze(analyzer, stmt->body);
    }

    return PCC_SUCCESS;
}

/* Analyze element list */
static PCCError analyze_element_list(SemanticAnalyzer* analyzer, ASTNode* node) {
    PCCArray* list = NULL;

    switch (node->type) {
        case AST_STATEMENT_LIST:
        case AST_EXPRESSION_LIST:
        case AST_ELEMENT_LIST:
        case AST_ARGUMENT_LIST:
        case AST_CONSTRAINT_LIST:
            list = (PCCArray*)node->data;
            break;
        default:
            return PCC_ERROR_RUNTIME;
    }

    if (list == NULL) {
        return PCC_SUCCESS;
    }

    /* Analyze all elements */
    for (size_t i = 0; i < pcc_array_size(list); i++) {
        ASTNode* elem = (ASTNode*)pcc_array_get(list, i);
        PCCError err = semantic_analyze(analyzer, elem);
        if (err != PCC_SUCCESS) {
            return err;
        }
    }

    return PCC_SUCCESS;
}

/* Get number of semantic errors */
size_t semantic_error_count(SemanticAnalyzer* analyzer) {
    return analyzer ? symbol_table_error_count(analyzer->symbol_table) : 0;
}

/* Get error at index */
SemanticError* semantic_get_error(SemanticAnalyzer* analyzer, size_t index) {
    return analyzer ? symbol_table_get_error(analyzer->symbol_table, index) : NULL;
}

/* Print all semantic errors */
void semantic_print_errors(SemanticAnalyzer* analyzer) {
    if (analyzer == NULL) {
        return;
    }
    symbol_table_print_errors(analyzer->symbol_table);
}

/* Check if analyzer has errors */
PCCBool semantic_has_errors(SemanticAnalyzer* analyzer) {
    return (PCCBool)(analyzer ? analyzer->has_errors : PCC_FALSE);
}

/* Get symbol table */
SymbolTable* semantic_get_symbol_table(SemanticAnalyzer* analyzer) {
    return analyzer ? analyzer->symbol_table : NULL;
}
