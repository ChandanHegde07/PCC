/**
 * @file optimizer.h
 * @brief Optimizer for PCC compiler
 * @details Performs AST optimizations like constant folding, dead code elimination
 *          DSA Concept: Tree transformation and pattern matching
 */

#ifndef PCC_OPTIMIZER_H
#define PCC_OPTIMIZER_H

#include "common.h"
#include "ast.h"

/* Optimization passes */
typedef enum {
    OPT_CONSTANT_FOLDING,      /* Evaluate constant expressions */
    OPT_DEAD_CODE_ELIMINATION, /* Remove unreachable code */
    OPT_UNUSED_REMOVAL,        /* Remove unused variables */
    OPT_INLINE_TEMPLATES,      /* Inline simple templates */
    OPT_ALL                    /* Apply all optimizations */
} OptimizationPass;

/* Optimizer structure */
typedef struct {
    int enabled_passes;        /* Bitmask of enabled passes */
    int optimizations_applied; /* Number of optimizations applied */
} Optimizer;

/* Function prototypes */

/**
 * @brief Create a new optimizer
 * @param enabled_passes Bitmask of enabled optimization passes
 * @return New optimizer, or NULL on failure
 */
Optimizer* optimizer_create(int enabled_passes);

/**
 * @brief Free optimizer
 * @param optimizer Optimizer to free
 */
void optimizer_free(Optimizer* optimizer);

/**
 * @brief Optimize an AST node
 * @param optimizer Optimizer
 * @param node AST node to optimize
 * @return Optimized AST node (may be same or new)
 */
ASTNode* optimizer_optimize(Optimizer* optimizer, ASTNode* node);

/**
 * @brief Enable an optimization pass
 * @param optimizer Optimizer
 * @param pass Pass to enable
 */
void optimizer_enable_pass(Optimizer* optimizer, OptimizationPass pass);

/**
 * @brief Disable an optimization pass
 * @param optimizer Optimizer
 * @param pass Pass to disable
 */
void optimizer_disable_pass(Optimizer* optimizer, OptimizationPass pass);

/**
 * @brief Check if a pass is enabled
 * @param optimizer Optimizer
 * @param pass Pass to check
 * @return PCC_TRUE if enabled, PCC_FALSE otherwise
 */
PCCBool optimizer_is_pass_enabled(Optimizer* optimizer, OptimizationPass pass);

/**
 * @brief Get number of optimizations applied
 * @param optimizer Optimizer
 * @return Number of optimizations applied
 */
int optimizer_get_optimizations_applied(Optimizer* optimizer);

/**
 * @brief Reset optimization counter
 * @param optimizer Optimizer
 */
void optimizer_reset_counter(Optimizer* optimizer);

#endif /* PCC_OPTIMIZER_H */
