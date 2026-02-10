/**
 * @file optimizer.c
 * @brief Optimizer implementation for PCC compiler
 */

#include "optimizer.h"
#include "lexer.h"
#include <stdio.h>
#include <string.h>
#include <math.h>

/* Helper function to check if node is a constant */
static PCCBool is_constant(ASTNode* node) {
    if (node == NULL) {
        return PCC_FALSE;
    }
    return (PCCBool)(node->type == AST_NUMBER_LITERAL ||
                     node->type == AST_BOOLEAN_LITERAL ||
                     node->type == AST_STRING_LITERAL);
}

/* Helper function to get numeric value from node */
static double get_numeric_value(ASTNode* node) {
    if (node == NULL || node->type != AST_NUMBER_LITERAL) {
        return 0.0;
    }
    ASTLiteral* lit = (ASTLiteral*)node->data;
    return lit->number_val;
}

/* Helper function to get boolean value from node */
static int get_boolean_value(ASTNode* node) {
    if (node == NULL || node->type != AST_BOOLEAN_LITERAL) {
        return 0;
    }
    ASTLiteral* lit = (ASTLiteral*)node->data;
    return lit->bool_val;
}

/* Constant folding for binary expressions */
static ASTNode* constant_folding_binary(Optimizer* optimizer, ASTNode* node) {
    if (node == NULL || node->type != AST_BINARY_EXPR) {
        return node;
    }

    ASTBinaryExpr* expr = (ASTBinaryExpr*)node->data;
    if (expr == NULL) {
        return node;
    }

    /* Recursively optimize operands */
    expr->left = optimizer_optimize(optimizer, (ASTNode*)expr->left);
    expr->right = optimizer_optimize(optimizer, (ASTNode*)expr->right);

    /* Check if both operands are constants */
    if (!is_constant((ASTNode*)expr->left) || !is_constant((ASTNode*)expr->right)) {
        return node;
    }

    /* Only fold numeric operations */
    if (((ASTNode*)expr->left)->type != AST_NUMBER_LITERAL ||
        ((ASTNode*)expr->right)->type != AST_NUMBER_LITERAL) {
        return node;
    }

    double left_val = get_numeric_value((ASTNode*)expr->left);
    double right_val = get_numeric_value((ASTNode*)expr->right);
    double result = 0.0;
    int can_fold = 1;

    switch (expr->op_type) {
        case TOKEN_ADD:
            result = left_val + right_val;
            break;
        case TOKEN_SUB:
            result = left_val - right_val;
            break;
        case TOKEN_MUL:
            result = left_val * right_val;
            break;
        case TOKEN_DIV:
            if (right_val == 0.0) {
                can_fold = 0; /* Division by zero */
            } else {
                result = left_val / right_val;
            }
            break;
        case TOKEN_MOD:
            if (right_val == 0.0) {
                can_fold = 0;
            } else {
                result = fmod(left_val, right_val);
            }
            break;
        case TOKEN_POW:
            result = pow(left_val, right_val);
            break;
        default:
            can_fold = 0;
            break;
    }

    if (can_fold) {
        /* Create new constant node */
        ASTNode* folded = ast_number_literal_create(result, node->position);
        if (folded != NULL) {
            optimizer->optimizations_applied++;
            /* Free old node */
            ast_node_free((ASTNode*)expr->left);
            ast_node_free((ASTNode*)expr->right);
            free(expr);
            free(node);
            return folded;
        }
    }

    return node;
}

/* Constant folding for unary expressions */
static ASTNode* constant_folding_unary(Optimizer* optimizer, ASTNode* node) {
    if (node == NULL || node->type != AST_UNARY_EXPR) {
        return node;
    }

    ASTUnaryExpr* expr = (ASTUnaryExpr*)node->data;
    if (expr == NULL) {
        return node;
    }

    /* Recursively optimize operand */
    expr->operand = optimizer_optimize(optimizer, (ASTNode*)expr->operand);

    /* Check if operand is constant */
    if (!is_constant((ASTNode*)expr->operand)) {
        return node;
    }

    /* Handle numeric negation */
    if (expr->op_type == TOKEN_SUB && ((ASTNode*)expr->operand)->type == AST_NUMBER_LITERAL) {
        double val = get_numeric_value((ASTNode*)expr->operand);
        ASTNode* folded = ast_number_literal_create(-val, node->position);
        if (folded != NULL) {
            optimizer->optimizations_applied++;
            ast_node_free((ASTNode*)expr->operand);
            free(expr);
            free(node);
            return folded;
        }
    }

    /* Handle logical NOT */
    if (expr->op_type == TOKEN_NOT && ((ASTNode*)expr->operand)->type == AST_BOOLEAN_LITERAL) {
        int val = get_boolean_value((ASTNode*)expr->operand);
        ASTNode* folded = ast_boolean_literal_create(!val, node->position);
        if (folded != NULL) {
            optimizer->optimizations_applied++;
            ast_node_free((ASTNode*)expr->operand);
            free(expr);
            free(node);
            return folded;
        }
    }

    return node;
}

/* Dead code elimination for if statements */
static ASTNode* dead_code_elimination_if(Optimizer* optimizer, ASTNode* node) {
    if (node == NULL || node->type != AST_IF_STMT) {
        return node;
    }

    ASTIfStmt* stmt = (ASTIfStmt*)node->data;
    if (stmt == NULL) {
        return node;
    }

    /* Optimize condition */
    stmt->condition = optimizer_optimize(optimizer, (ASTNode*)stmt->condition);

    /* Check if condition is constant */
    if (((ASTNode*)stmt->condition)->type == AST_BOOLEAN_LITERAL) {
        int condition_val = get_boolean_value((ASTNode*)stmt->condition);

        if (condition_val) {
            /* Always true, replace with then body */
            ASTNode* result = (ASTNode*)stmt->then_body;
            if (result != NULL) {
                optimizer->optimizations_applied++;
                ast_node_free((ASTNode*)stmt->condition);
                if (stmt->else_body != NULL) {
                    ast_node_free((ASTNode*)stmt->else_body);
                }
                free(stmt);
                free(node);
                return result;
            }
        } else {
            /* Always false, replace with else body or empty */
            ASTNode* result = (ASTNode*)stmt->else_body;
            if (result != NULL) {
                optimizer->optimizations_applied++;
                ast_node_free((ASTNode*)stmt->condition);
                ast_node_free((ASTNode*)stmt->then_body);
                free(stmt);
                free(node);
                return result;
            } else {
                /* Remove entire if statement */
                optimizer->optimizations_applied++;
                ast_node_free((ASTNode*)stmt->condition);
                ast_node_free((ASTNode*)stmt->then_body);
                free(stmt);
                free(node);
                return NULL;
            }
        }
    }

    /* Optimize branches */
    if (stmt->then_body != NULL) {
        stmt->then_body = optimizer_optimize(optimizer, (ASTNode*)stmt->then_body);
    }
    if (stmt->else_body != NULL) {
        stmt->else_body = optimizer_optimize(optimizer, (ASTNode*)stmt->else_body);
    }

    return node;
}

/* Optimize program */
static ASTNode* optimize_program(Optimizer* optimizer, ASTNode* node) {
    if (node == NULL || node->type != AST_PROGRAM) {
        return node;
    }

    ASTProgram* prog = (ASTProgram*)node->data;
    if (prog == NULL || prog->statements == NULL) {
        return node;
    }

    /* Optimize all statements */
    for (size_t i = 0; i < pcc_array_size(prog->statements); i++) {
        ASTNode* stmt = (ASTNode*)pcc_array_get(prog->statements, i);
        ASTNode* optimized = optimizer_optimize(optimizer, stmt);
        if (optimized != stmt) {
            pcc_array_set(prog->statements, i, optimized);
        }
    }

    return node;
}

/* Optimize list nodes */
static ASTNode* optimize_list(Optimizer* optimizer, ASTNode* node) {
    if (node == NULL) {
        return NULL;
    }

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
            return node;
    }

    if (list == NULL) {
        return node;
    }

    /* Optimize all elements */
    for (size_t i = 0; i < pcc_array_size(list); i++) {
        ASTNode* elem = (ASTNode*)pcc_array_get(list, i);
        ASTNode* optimized = optimizer_optimize(optimizer, elem);
        if (optimized != elem) {
            pcc_array_set(list, i, optimized);
        }
    }

    return node;
}

/* Optimize other node types */
static ASTNode* optimize_node(Optimizer* optimizer, ASTNode* node) {
    if (node == NULL) {
        return NULL;
    }

    switch (node->type) {
        case AST_PROGRAM:
            return optimize_program(optimizer, node);
        case AST_BINARY_EXPR:
            return constant_folding_binary(optimizer, node);
        case AST_UNARY_EXPR:
            return constant_folding_unary(optimizer, node);
        case AST_IF_STMT:
            return dead_code_elimination_if(optimizer, node);
        case AST_STATEMENT_LIST:
        case AST_EXPRESSION_LIST:
        case AST_ELEMENT_LIST:
        case AST_ARGUMENT_LIST:
        case AST_CONSTRAINT_LIST:
            return optimize_list(optimizer, node);
        case AST_PROMPT_DEF:
        case AST_VAR_DECL:
        case AST_TEMPLATE_DEF:
        case AST_CONSTRAINT_DEF:
        case AST_OUTPUT_SPEC:
        case AST_FOR_STMT:
        case AST_WHILE_STMT:
            /* These nodes contain child nodes that need optimization */
            /* For simplicity, we'll just return them as-is */
            return node;
        default:
            return node;
    }
}

/* Create a new optimizer */
Optimizer* optimizer_create(int enabled_passes) {
    Optimizer* optimizer = (Optimizer*)malloc(sizeof(Optimizer));
    if (optimizer == NULL) {
        return NULL;
    }

    optimizer->enabled_passes = enabled_passes;
    optimizer->optimizations_applied = 0;

    return optimizer;
}

/* Free optimizer */
void optimizer_free(Optimizer* optimizer) {
    if (optimizer != NULL) {
        free(optimizer);
    }
}

/* Optimize an AST node */
ASTNode* optimizer_optimize(Optimizer* optimizer, ASTNode* node) {
    if (optimizer == NULL || node == NULL) {
        return node;
    }

    /* Apply enabled optimization passes */
    if (optimizer->enabled_passes & (1 << OPT_CONSTANT_FOLDING) ||
        optimizer->enabled_passes & (1 << OPT_ALL)) {
        node = optimize_node(optimizer, node);
    }

    if (optimizer->enabled_passes & (1 << OPT_DEAD_CODE_ELIMINATION) ||
        optimizer->enabled_passes & (1 << OPT_ALL)) {
        node = optimize_node(optimizer, node);
    }

    return node;
}

/* Enable an optimization pass */
void optimizer_enable_pass(Optimizer* optimizer, OptimizationPass pass) {
    if (optimizer == NULL) {
        return;
    }
    optimizer->enabled_passes |= (1 << pass);
}

/* Disable an optimization pass */
void optimizer_disable_pass(Optimizer* optimizer, OptimizationPass pass) {
    if (optimizer == NULL) {
        return;
    }
    optimizer->enabled_passes &= ~(1 << pass);
}

/* Check if a pass is enabled */
PCCBool optimizer_is_pass_enabled(Optimizer* optimizer, OptimizationPass pass) {
    if (optimizer == NULL) {
        return PCC_FALSE;
    }
    return (PCCBool)((optimizer->enabled_passes & (1 << pass)) != 0);
}

/* Get number of optimizations applied */
int optimizer_get_optimizations_applied(Optimizer* optimizer) {
    return optimizer ? optimizer->optimizations_applied : 0;
}

/* Reset optimization counter */
void optimizer_reset_counter(Optimizer* optimizer) {
    if (optimizer != NULL) {
        optimizer->optimizations_applied = 0;
    }
}
