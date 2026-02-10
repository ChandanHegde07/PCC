/**
 * @file ast.c
 * @brief Abstract Syntax Tree (AST) implementation for PCC compiler
 */

#include "ast.h"
#include <stdio.h>
#include <string.h>

/* Helper function to free AST node data based on type */
static void ast_data_free(ASTNodeType type, void* data) {
    if (data == NULL) {
        return;
    }

    switch (type) {
        case AST_IDENTIFIER: {
            ASTIdentifier* id = (ASTIdentifier*)data;
            if (id->name) free(id->name);
            free(id);
            break;
        }
        case AST_STRING_LITERAL: {
            ASTLiteral* lit = (ASTLiteral*)data;
            if (lit->string_val) free(lit->string_val);
            free(lit);
            break;
        }
        case AST_NUMBER_LITERAL:
        case AST_BOOLEAN_LITERAL: {
            free(data);
            break;
        }
        case AST_BINARY_EXPR:
        case AST_UNARY_EXPR: {
            free(data);
            break;
        }
        case AST_VARIABLE_REF: {
            ASTVariableRef* ref = (ASTVariableRef*)data;
            if (ref->name) free(ref->name);
            free(ref);
            break;
        }
        case AST_FUNCTION_CALL:
        case AST_TEMPLATE_CALL: {
            ASTCallExpr* call = (ASTCallExpr*)data;
            if (call->name) free(call->name);
            if (call->arguments) {
                for (size_t i = 0; i < pcc_array_size(call->arguments); i++) {
                    ASTNode* arg = (ASTNode*)pcc_array_get(call->arguments, i);
                    ast_node_free(arg);
                }
                pcc_array_free(call->arguments, NULL);
            }
            free(call);
            break;
        }
        case AST_VAR_DECL: {
            ASTVarDecl* decl = (ASTVarDecl*)data;
            if (decl->name) free(decl->name);
            if (decl->initializer) ast_node_free((ASTNode*)decl->initializer);
            free(decl);
            break;
        }
        case AST_TEMPLATE_DEF: {
            ASTTemplateDef* def = (ASTTemplateDef*)data;
            if (def->name) free(def->name);
            if (def->parameters) {
                for (size_t i = 0; i < pcc_array_size(def->parameters); i++) {
                    char* param = (char*)pcc_array_get(def->parameters, i);
                    free(param);
                }
                pcc_array_free(def->parameters, NULL);
            }
            if (def->body) ast_node_free((ASTNode*)def->body);
            free(def);
            break;
        }
        case AST_PROMPT_DEF: {
            ASTPromptDef* def = (ASTPromptDef*)data;
            if (def->name) free(def->name);
            if (def->body) ast_node_free((ASTNode*)def->body);
            free(def);
            break;
        }
        case AST_CONSTRAINT_DEF: {
            ASTConstraintDef* def = (ASTConstraintDef*)data;
            if (def->name) free(def->name);
            if (def->constraints) {
                for (size_t i = 0; i < pcc_array_size(def->constraints); i++) {
                    ASTNode* constraint = (ASTNode*)pcc_array_get(def->constraints, i);
                    ast_node_free(constraint);
                }
                pcc_array_free(def->constraints, NULL);
            }
            free(def);
            break;
        }
        case AST_CONSTRAINT_EXPR: {
            ASTConstraintExpr* expr = (ASTConstraintExpr*)data;
            if (expr->variable) free(expr->variable);
            if (expr->value) ast_node_free((ASTNode*)expr->value);
            free(expr);
            break;
        }
        case AST_OUTPUT_SPEC: {
            ASTOutputSpec* spec = (ASTOutputSpec*)data;
            if (spec->name) free(spec->name);
            free(spec);
            break;
        }
        case AST_IF_STMT: {
            ASTIfStmt* stmt = (ASTIfStmt*)data;
            if (stmt->condition) ast_node_free((ASTNode*)stmt->condition);
            if (stmt->then_body) ast_node_free((ASTNode*)stmt->then_body);
            if (stmt->else_body) ast_node_free((ASTNode*)stmt->else_body);
            free(stmt);
            break;
        }
        case AST_FOR_STMT: {
            ASTForStmt* stmt = (ASTForStmt*)data;
            if (stmt->variable) free(stmt->variable);
            if (stmt->iterable) ast_node_free((ASTNode*)stmt->iterable);
            if (stmt->body) ast_node_free((ASTNode*)stmt->body);
            free(stmt);
            break;
        }
        case AST_WHILE_STMT: {
            ASTWhileStmt* stmt = (ASTWhileStmt*)data;
            if (stmt->condition) ast_node_free((ASTNode*)stmt->condition);
            if (stmt->body) ast_node_free((ASTNode*)stmt->body);
            free(stmt);
            break;
        }
        case AST_TEXT_ELEMENT: {
            ASTTextElement* elem = (ASTTextElement*)data;
            if (elem->text) free(elem->text);
            free(elem);
            break;
        }
        case AST_STATEMENT_LIST:
        case AST_EXPRESSION_LIST:
        case AST_PARAMETER_LIST:
        case AST_ARGUMENT_LIST:
        case AST_CONSTRAINT_LIST:
        case AST_ELEMENT_LIST: {
            PCCArray* list = (PCCArray*)data;
            for (size_t i = 0; i < pcc_array_size(list); i++) {
                ASTNode* elem = (ASTNode*)pcc_array_get(list, i);
                ast_node_free(elem);
            }
            pcc_array_free(list, NULL);
            break;
        }
        case AST_PROGRAM: {
            ASTProgram* prog = (ASTProgram*)data;
            if (prog->statements) {
                for (size_t i = 0; i < pcc_array_size(prog->statements); i++) {
                    ASTNode* stmt = (ASTNode*)pcc_array_get(prog->statements, i);
                    ast_node_free(stmt);
                }
                pcc_array_free(prog->statements, NULL);
            }
            free(prog);
            break;
        }
        default:
            free(data);
            break;
    }
}

/* Create a new AST node */
ASTNode* ast_node_create(ASTNodeType type, void* data, PCCPosition position) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    if (node == NULL) {
        return NULL;
    }

    node->type = type;
    node->data = data;
    node->position = position;

    return node;
}

/* Free an AST node and all its children */
void ast_node_free(ASTNode* node) {
    if (node == NULL) {
        return;
    }

    ast_data_free(node->type, node->data);
    free(node);
}

/* Create an identifier node */
ASTNode* ast_identifier_create(const char* name, PCCPosition position) {
    ASTIdentifier* id = (ASTIdentifier*)malloc(sizeof(ASTIdentifier));
    if (id == NULL) {
        return NULL;
    }

    id->name = pcc_strdup(name);
    if (id->name == NULL) {
        free(id);
        return NULL;
    }

    return ast_node_create(AST_IDENTIFIER, id, position);
}

/* Create a string literal node */
ASTNode* ast_string_literal_create(const char* value, PCCPosition position) {
    ASTLiteral* lit = (ASTLiteral*)malloc(sizeof(ASTLiteral));
    if (lit == NULL) {
        return NULL;
    }

    lit->string_val = pcc_strdup(value);
    if (lit->string_val == NULL) {
        free(lit);
        return NULL;
    }

    return ast_node_create(AST_STRING_LITERAL, lit, position);
}

/* Create a number literal node */
ASTNode* ast_number_literal_create(double value, PCCPosition position) {
    ASTLiteral* lit = (ASTLiteral*)malloc(sizeof(ASTLiteral));
    if (lit == NULL) {
        return NULL;
    }

    lit->number_val = value;
    lit->string_val = NULL;

    return ast_node_create(AST_NUMBER_LITERAL, lit, position);
}

/* Create a boolean literal node */
ASTNode* ast_boolean_literal_create(int value, PCCPosition position) {
    ASTLiteral* lit = (ASTLiteral*)malloc(sizeof(ASTLiteral));
    if (lit == NULL) {
        return NULL;
    }

    lit->bool_val = value;
    lit->string_val = NULL;

    return ast_node_create(AST_BOOLEAN_LITERAL, lit, position);
}

/* Create a binary expression node */
ASTNode* ast_binary_expr_create(int op_type, ASTNode* left, ASTNode* right, PCCPosition position) {
    ASTBinaryExpr* expr = (ASTBinaryExpr*)malloc(sizeof(ASTBinaryExpr));
    if (expr == NULL) {
        return NULL;
    }

    expr->op_type = op_type;
    expr->left = left;
    expr->right = right;

    return ast_node_create(AST_BINARY_EXPR, expr, position);
}

/* Create a unary expression node */
ASTNode* ast_unary_expr_create(int op_type, ASTNode* operand, PCCPosition position) {
    ASTUnaryExpr* expr = (ASTUnaryExpr*)malloc(sizeof(ASTUnaryExpr));
    if (expr == NULL) {
        return NULL;
    }

    expr->op_type = op_type;
    expr->operand = operand;

    return ast_node_create(AST_UNARY_EXPR, expr, position);
}

/* Create a variable reference node */
ASTNode* ast_variable_ref_create(const char* name, PCCPosition position) {
    ASTVariableRef* ref = (ASTVariableRef*)malloc(sizeof(ASTVariableRef));
    if (ref == NULL) {
        return NULL;
    }

    ref->name = pcc_strdup(name);
    if (ref->name == NULL) {
        free(ref);
        return NULL;
    }

    return ast_node_create(AST_VARIABLE_REF, ref, position);
}

/* Create a function call node */
ASTNode* ast_function_call_create(const char* name, PCCArray* arguments, PCCPosition position) {
    ASTCallExpr* call = (ASTCallExpr*)malloc(sizeof(ASTCallExpr));
    if (call == NULL) {
        return NULL;
    }

    call->name = pcc_strdup(name);
    if (call->name == NULL) {
        free(call);
        return NULL;
    }

    call->arguments = arguments;

    return ast_node_create(AST_FUNCTION_CALL, call, position);
}

/* Create a variable declaration node */
ASTNode* ast_var_decl_create(const char* name, ASTNode* initializer, PCCPosition position) {
    ASTVarDecl* decl = (ASTVarDecl*)malloc(sizeof(ASTVarDecl));
    if (decl == NULL) {
        return NULL;
    }

    decl->name = pcc_strdup(name);
    if (decl->name == NULL) {
        free(decl);
        return NULL;
    }

    decl->initializer = initializer;

    return ast_node_create(AST_VAR_DECL, decl, position);
}

/* Create a template definition node */
ASTNode* ast_template_def_create(const char* name, PCCArray* parameters, ASTNode* body, PCCPosition position) {
    ASTTemplateDef* def = (ASTTemplateDef*)malloc(sizeof(ASTTemplateDef));
    if (def == NULL) {
        return NULL;
    }

    def->name = pcc_strdup(name);
    if (def->name == NULL) {
        free(def);
        return NULL;
    }

    def->parameters = parameters;
    def->body = body;

    return ast_node_create(AST_TEMPLATE_DEF, def, position);
}

/* Create a prompt definition node */
ASTNode* ast_prompt_def_create(const char* name, ASTNode* body, PCCPosition position) {
    ASTPromptDef* def = (ASTPromptDef*)malloc(sizeof(ASTPromptDef));
    if (def == NULL) {
        return NULL;
    }

    def->name = pcc_strdup(name);
    if (def->name == NULL) {
        free(def);
        return NULL;
    }

    def->body = body;

    return ast_node_create(AST_PROMPT_DEF, def, position);
}

/* Create a constraint definition node */
ASTNode* ast_constraint_def_create(const char* name, PCCArray* constraints, PCCPosition position) {
    ASTConstraintDef* def = (ASTConstraintDef*)malloc(sizeof(ASTConstraintDef));
    if (def == NULL) {
        return NULL;
    }

    def->name = pcc_strdup(name);
    if (def->name == NULL) {
        free(def);
        return NULL;
    }

    def->constraints = constraints;

    return ast_node_create(AST_CONSTRAINT_DEF, def, position);
}

/* Create a constraint expression node */
ASTNode* ast_constraint_expr_create(const char* variable, int op_type, ASTNode* value, PCCPosition position) {
    ASTConstraintExpr* expr = (ASTConstraintExpr*)malloc(sizeof(ASTConstraintExpr));
    if (expr == NULL) {
        return NULL;
    }

    expr->variable = pcc_strdup(variable);
    if (expr->variable == NULL) {
        free(expr);
        return NULL;
    }

    expr->op_type = op_type;
    expr->value = value;

    return ast_node_create(AST_CONSTRAINT_EXPR, expr, position);
}

/* Create an output specification node */
ASTNode* ast_output_spec_create(const char* name, int format, PCCPosition position) {
    ASTOutputSpec* spec = (ASTOutputSpec*)malloc(sizeof(ASTOutputSpec));
    if (spec == NULL) {
        return NULL;
    }

    spec->name = pcc_strdup(name);
    if (spec->name == NULL) {
        free(spec);
        return NULL;
    }

    spec->format = format;

    return ast_node_create(AST_OUTPUT_SPEC, spec, position);
}

/* Create an if statement node */
ASTNode* ast_if_stmt_create(ASTNode* condition, ASTNode* then_body, ASTNode* else_body, PCCPosition position) {
    ASTIfStmt* stmt = (ASTIfStmt*)malloc(sizeof(ASTIfStmt));
    if (stmt == NULL) {
        return NULL;
    }

    stmt->condition = condition;
    stmt->then_body = then_body;
    stmt->else_body = else_body;

    return ast_node_create(AST_IF_STMT, stmt, position);
}

/* Create a for statement node */
ASTNode* ast_for_stmt_create(const char* variable, ASTNode* iterable, ASTNode* body, PCCPosition position) {
    ASTForStmt* stmt = (ASTForStmt*)malloc(sizeof(ASTForStmt));
    if (stmt == NULL) {
        return NULL;
    }

    stmt->variable = pcc_strdup(variable);
    if (stmt->variable == NULL) {
        free(stmt);
        return NULL;
    }

    stmt->iterable = iterable;
    stmt->body = body;

    return ast_node_create(AST_FOR_STMT, stmt, position);
}

/* Create a while statement node */
ASTNode* ast_while_stmt_create(ASTNode* condition, ASTNode* body, PCCPosition position) {
    ASTWhileStmt* stmt = (ASTWhileStmt*)malloc(sizeof(ASTWhileStmt));
    if (stmt == NULL) {
        return NULL;
    }

    stmt->condition = condition;
    stmt->body = body;

    return ast_node_create(AST_WHILE_STMT, stmt, position);
}

/* Create a text element node */
ASTNode* ast_text_element_create(const char* text, int is_raw, PCCPosition position) {
    ASTTextElement* elem = (ASTTextElement*)malloc(sizeof(ASTTextElement));
    if (elem == NULL) {
        return NULL;
    }

    elem->text = pcc_strdup(text);
    if (elem->text == NULL) {
        free(elem);
        return NULL;
    }

    elem->is_raw = is_raw;

    return ast_node_create(AST_TEXT_ELEMENT, elem, position);
}

/* Create a list node */
ASTNode* ast_list_create(ASTNodeType type, PCCArray* elements, PCCPosition position) {
    return ast_node_create(type, elements, position);
}

/* Create a program node */
ASTNode* ast_program_create(PCCArray* statements) {
    ASTProgram* prog = (ASTProgram*)malloc(sizeof(ASTProgram));
    if (prog == NULL) {
        return NULL;
    }

    prog->statements = statements;

    PCCPosition pos = {0, 0, "<program>"};
    return ast_node_create(AST_PROGRAM, prog, pos);
}

/* Get AST node type name as string */
const char* ast_node_type_name(ASTNodeType type) {
    switch (type) {
        case AST_PROGRAM: return "PROGRAM";
        case AST_PROMPT_DEF: return "PROMPT_DEF";
        case AST_VAR_DECL: return "VAR_DECL";
        case AST_TEMPLATE_DEF: return "TEMPLATE_DEF";
        case AST_CONSTRAINT_DEF: return "CONSTRAINT_DEF";
        case AST_OUTPUT_SPEC: return "OUTPUT_SPEC";
        case AST_IDENTIFIER: return "IDENTIFIER";
        case AST_STRING_LITERAL: return "STRING_LITERAL";
        case AST_NUMBER_LITERAL: return "NUMBER_LITERAL";
        case AST_BOOLEAN_LITERAL: return "BOOLEAN_LITERAL";
        case AST_BINARY_EXPR: return "BINARY_EXPR";
        case AST_UNARY_EXPR: return "UNARY_EXPR";
        case AST_VARIABLE_REF: return "VARIABLE_REF";
        case AST_TEMPLATE_CALL: return "TEMPLATE_CALL";
        case AST_FUNCTION_CALL: return "FUNCTION_CALL";
        case AST_IF_STMT: return "IF_STMT";
        case AST_FOR_STMT: return "FOR_STMT";
        case AST_WHILE_STMT: return "WHILE_STMT";
        case AST_TEXT_ELEMENT: return "TEXT_ELEMENT";
        case AST_CONSTRAINT_EXPR: return "CONSTRAINT_EXPR";
        case AST_OUTPUT_FORMAT: return "OUTPUT_FORMAT";
        case AST_STATEMENT_LIST: return "STATEMENT_LIST";
        case AST_EXPRESSION_LIST: return "EXPRESSION_LIST";
        case AST_PARAMETER_LIST: return "PARAMETER_LIST";
        case AST_ARGUMENT_LIST: return "ARGUMENT_LIST";
        case AST_CONSTRAINT_LIST: return "CONSTRAINT_LIST";
        case AST_ELEMENT_LIST: return "ELEMENT_LIST";
        case AST_EMPTY: return "EMPTY";
        default: return "UNKNOWN";
    }
}

/* Print AST tree (for debugging) */
static void ast_print_recursive(ASTNode* node, int depth) {
    if (node == NULL) {
        return;
    }

    /* Print indentation */
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }

    printf("%s", ast_node_type_name(node->type));

    /* Print additional info based on type */
    switch (node->type) {
        case AST_IDENTIFIER: {
            ASTIdentifier* id = (ASTIdentifier*)node->data;
            printf(": %s", id->name);
            break;
        }
        case AST_STRING_LITERAL: {
            ASTLiteral* lit = (ASTLiteral*)node->data;
            printf(": \"%s\"", lit->string_val);
            break;
        }
        case AST_NUMBER_LITERAL: {
            ASTLiteral* lit = (ASTLiteral*)node->data;
            printf(": %g", lit->number_val);
            break;
        }
        case AST_BOOLEAN_LITERAL: {
            ASTLiteral* lit = (ASTLiteral*)node->data;
            printf(": %s", lit->bool_val ? "true" : "false");
            break;
        }
        case AST_VARIABLE_REF: {
            ASTVariableRef* ref = (ASTVariableRef*)node->data;
            printf(": $%s", ref->name);
            break;
        }
        case AST_TEXT_ELEMENT: {
            ASTTextElement* elem = (ASTTextElement*)node->data;
            printf(": \"%s\"%s", elem->text, elem->is_raw ? " (raw)" : "");
            break;
        }
        default:
            break;
    }

    printf("\n");

    /* Print children for list nodes */
    if (node->type == AST_STATEMENT_LIST || node->type == AST_EXPRESSION_LIST ||
        node->type == AST_ELEMENT_LIST || node->type == AST_ARGUMENT_LIST ||
        node->type == AST_PARAMETER_LIST || node->type == AST_CONSTRAINT_LIST) {
        PCCArray* list = (PCCArray*)node->data;
        for (size_t i = 0; i < pcc_array_size(list); i++) {
            ASTNode* child = (ASTNode*)pcc_array_get(list, i);
            ast_print_recursive(child, depth + 1);
        }
    }
}

void ast_print(ASTNode* node) {
    ast_print_recursive(node, 0);
}
