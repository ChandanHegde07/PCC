/**
 * @file tree.c
 * @brief Tree data structure implementation for PCC compiler
 */

#include "tree.h"
#include <stdio.h>

/* Create a new tree node */
PCCTreeNode* pcc_tree_node_create(void* data, int node_type) {
    PCCTreeNode* node = (PCCTreeNode*)malloc(sizeof(PCCTreeNode));
    if (node == NULL) {
        return NULL;
    }

    node->data = data;
    node->parent = NULL;
    node->node_type = node_type;
    node->position.line = 0;
    node->position.column = 0;
    node->position.filename = NULL;

    node->children = pcc_array_create(INITIAL_CAPACITY, sizeof(PCCTreeNode*));
    if (node->children == NULL) {
        free(node);
        return NULL;
    }

    return node;
}

/* Free a tree node and all its descendants */
void pcc_tree_node_free(PCCTreeNode* node, void (*free_data_func)(void*)) {
    if (node == NULL) {
        return;
    }

    /* Free all children first (post-order) */
    for (size_t i = 0; i < pcc_array_size(node->children); i++) {
        PCCTreeNode* child = (PCCTreeNode*)pcc_array_get(node->children, i);
        pcc_tree_node_free(child, free_data_func);
    }

    /* Free children array */
    pcc_array_free(node->children, NULL);

    /* Free node data */
    if (free_data_func != NULL && node->data != NULL) {
        free_data_func(node->data);
    }

    free(node);
}

/* Create a new tree */
PCCTree* pcc_tree_create(void* root_data, int root_type) {
    PCCTree* tree = (PCCTree*)malloc(sizeof(PCCTree));
    if (tree == NULL) {
        return NULL;
    }

    tree->root = pcc_tree_node_create(root_data, root_type);
    if (tree->root == NULL) {
        free(tree);
        return NULL;
    }

    tree->node_count = 1;

    return tree;
}

/* Free a tree and all its nodes */
void pcc_tree_free(PCCTree* tree, void (*free_data_func)(void*)) {
    if (tree == NULL) {
        return;
    }

    if (tree->root != NULL) {
        pcc_tree_node_free(tree->root, free_data_func);
    }

    free(tree);
}

/* Add a child node to a parent */
PCCError pcc_tree_add_child(PCCTreeNode* parent, PCCTreeNode* child) {
    if (parent == NULL || child == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    /* Set parent reference */
    child->parent = parent;

    /* Add to parent's children array */
    return pcc_array_push(parent->children, child);
}

/* Remove a child node from parent */
PCCError pcc_tree_remove_child(PCCTreeNode* parent, PCCTreeNode* child, void (*free_data_func)(void*)) {
    if (parent == NULL || child == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    /* Find child index */
    int index = pcc_array_find(parent->children, child,
        (int (*)(const void*, const void*))memcmp);

    if (index == -1) {
        return PCC_ERROR_RUNTIME;
    }

    /* Remove from array */
    void* removed;
    PCCError err = pcc_array_remove(parent->children, (size_t)index, &removed);
    if (err != PCC_SUCCESS) {
        return err;
    }

    /* Free the child and its descendants */
    pcc_tree_node_free((PCCTreeNode*)removed, free_data_func);

    return PCC_SUCCESS;
}

/* Get child at index */
PCCTreeNode* pcc_tree_get_child(PCCTreeNode* parent, size_t index) {
    if (parent == NULL) {
        return NULL;
    }

    return (PCCTreeNode*)pcc_array_get(parent->children, index);
}

/* Get number of children */
size_t pcc_tree_child_count(PCCTreeNode* node) {
    if (node == NULL) {
        return 0;
    }

    return pcc_array_size(node->children);
}

/* Check if node is root */
PCCBool pcc_tree_is_root(PCCTreeNode* node) {
    return (PCCBool)(node != NULL && node->parent == NULL);
}

/* Check if node is leaf */
PCCBool pcc_tree_is_leaf(PCCTreeNode* node) {
    if (node == NULL) {
        return PCC_TRUE;
    }

    return (PCCBool)(pcc_array_size(node->children) == 0);
}

/* Get depth of node (distance from root) */
int pcc_tree_depth(PCCTreeNode* node) {
    if (node == NULL) {
        return -1;
    }

    int depth = 0;
    PCCTreeNode* current = node;

    while (current->parent != NULL) {
        depth++;
        current = current->parent;
    }

    return depth;
}

/* Get height of node (longest path to leaf) - recursive */
static int pcc_tree_height_recursive(PCCTreeNode* node) {
    if (node == NULL || pcc_tree_is_leaf(node)) {
        return 0;
    }

    int max_child_height = 0;

    for (size_t i = 0; i < pcc_array_size(node->children); i++) {
        PCCTreeNode* child = (PCCTreeNode*)pcc_array_get(node->children, i);
        int child_height = pcc_tree_height_recursive(child);
        if (child_height > max_child_height) {
            max_child_height = child_height;
        }
    }

    return max_child_height + 1;
}

int pcc_tree_height(PCCTreeNode* node) {
    return pcc_tree_height_recursive(node);
}

/* Find node by type using DFS */
static PCCTreeNode* pcc_tree_find_by_type_dfs(PCCTreeNode* node, int node_type) {
    if (node == NULL) {
        return NULL;
    }

    /* Check current node */
    if (node->node_type == node_type) {
        return node;
    }

    /* Search children */
    for (size_t i = 0; i < pcc_array_size(node->children); i++) {
        PCCTreeNode* child = (PCCTreeNode*)pcc_array_get(node->children, i);
        PCCTreeNode* result = pcc_tree_find_by_type_dfs(child, node_type);
        if (result != NULL) {
            return result;
        }
    }

    return NULL;
}

PCCTreeNode* pcc_tree_find_by_type(PCCTreeNode* node, int node_type) {
    return pcc_tree_find_by_type_dfs(node, node_type);
}

/* Find all nodes of a given type */
static void pcc_tree_find_all_by_type_dfs(PCCTreeNode* node, int node_type, PCCArray* results) {
    if (node == NULL) {
        return;
    }

    /* Check current node */
    if (node->node_type == node_type) {
        pcc_array_push(results, node);
    }

    /* Search children */
    for (size_t i = 0; i < pcc_array_size(node->children); i++) {
        PCCTreeNode* child = (PCCTreeNode*)pcc_array_get(node->children, i);
        pcc_tree_find_all_by_type_dfs(child, node_type, results);
    }
}

size_t pcc_tree_find_all_by_type(PCCTreeNode* node, int node_type, PCCArray* results) {
    if (results == NULL) {
        return 0;
    }

    size_t initial_size = pcc_array_size(results);
    pcc_tree_find_all_by_type_dfs(node, node_type, results);

    return pcc_array_size(results) - initial_size;
}

/* Pre-order traversal */
static void pcc_tree_traverse_pre_order(PCCTreeNode* node, PCCTreeVisitor visitor, void* user_data) {
    if (node == NULL) {
        return;
    }

    visitor(node, user_data);

    for (size_t i = 0; i < pcc_array_size(node->children); i++) {
        PCCTreeNode* child = (PCCTreeNode*)pcc_array_get(node->children, i);
        pcc_tree_traverse_pre_order(child, visitor, user_data);
    }
}

/* Post-order traversal */
static void pcc_tree_traverse_post_order(PCCTreeNode* node, PCCTreeVisitor visitor, void* user_data) {
    if (node == NULL) {
        return;
    }

    for (size_t i = 0; i < pcc_array_size(node->children); i++) {
        PCCTreeNode* child = (PCCTreeNode*)pcc_array_get(node->children, i);
        pcc_tree_traverse_post_order(child, visitor, user_data);
    }

    visitor(node, user_data);
}

/* Level-order traversal (BFS) */
static void pcc_tree_traverse_level_order(PCCTreeNode* node, PCCTreeVisitor visitor, void* user_data) {
    if (node == NULL) {
        return;
    }

    /* Use array as queue */
    PCCArray* queue = pcc_array_create(INITIAL_CAPACITY, sizeof(PCCTreeNode*));
    if (queue == NULL) {
        return;
    }

    pcc_array_push(queue, node);

    while (pcc_array_size(queue) > 0) {
        void* current_ptr;
        pcc_array_pop(queue, &current_ptr);
        PCCTreeNode* current = (PCCTreeNode*)current_ptr;

        visitor(current, user_data);

        /* Add children to queue */
        for (size_t i = 0; i < pcc_array_size(current->children); i++) {
            PCCTreeNode* child = (PCCTreeNode*)pcc_array_get(current->children, i);
            pcc_array_push(queue, child);
        }
    }

    pcc_array_free(queue, NULL);
}

void pcc_tree_traverse(PCCTreeNode* node, PCCTraversalOrder order,
                       PCCTreeVisitor visitor, void* user_data) {
    switch (order) {
        case PCC_TRAVERSAL_PRE_ORDER:
            pcc_tree_traverse_pre_order(node, visitor, user_data);
            break;
        case PCC_TRAVERSAL_POST_ORDER:
            pcc_tree_traverse_post_order(node, visitor, user_data);
            break;
        case PCC_TRAVERSAL_LEVEL_ORDER:
            pcc_tree_traverse_level_order(node, visitor, user_data);
            break;
        case PCC_TRAVERSAL_IN_ORDER:
            /* In-order only makes sense for binary trees */
            /* For general trees, treat as pre-order */
            pcc_tree_traverse_pre_order(node, visitor, user_data);
            break;
    }
}

/* Clone a tree node and all its descendants */
PCCTreeNode* pcc_tree_node_clone(PCCTreeNode* node, void* (*clone_data_func)(void*)) {
    if (node == NULL) {
        return NULL;
    }

    /* Clone data if function provided */
    void* cloned_data = NULL;
    if (clone_data_func != NULL && node->data != NULL) {
        cloned_data = clone_data_func(node->data);
    }

    /* Create new node */
    PCCTreeNode* clone = pcc_tree_node_create(cloned_data, node->node_type);
    if (clone == NULL) {
        if (cloned_data != NULL && cloned_data != node->data) {
            free(cloned_data);
        }
        return NULL;
    }

    /* Copy position */
    clone->position = node->position;

    /* Clone all children */
    for (size_t i = 0; i < pcc_array_size(node->children); i++) {
        PCCTreeNode* child = (PCCTreeNode*)pcc_array_get(node->children, i);
        PCCTreeNode* child_clone = pcc_tree_node_clone(child, clone_data_func);
        if (child_clone != NULL) {
            pcc_tree_add_child(clone, child_clone);
        }
    }

    return clone;
}

/* Get sibling at offset */
PCCTreeNode* pcc_tree_get_sibling(PCCTreeNode* node, int offset) {
    if (node == NULL || node->parent == NULL) {
        return NULL;
    }

    int index = pcc_tree_get_child_index(node);
    if (index == -1) {
        return NULL;
    }

    int sibling_index = index + offset;
    if (sibling_index < 0) {
        return NULL;
    }

    return pcc_tree_get_child(node->parent, (size_t)sibling_index);
}

/* Get index of node in parent's children */
int pcc_tree_get_child_index(PCCTreeNode* node) {
    if (node == NULL || node->parent == NULL) {
        return -1;
    }

    for (size_t i = 0; i < pcc_array_size(node->parent->children); i++) {
        PCCTreeNode* child = (PCCTreeNode*)pcc_array_get(node->parent->children, i);
        if (child == node) {
            return (int)i;
        }
    }

    return -1;
}

/* Replace child node with another */
PCCError pcc_tree_replace_child(PCCTreeNode* parent, PCCTreeNode* old_child,
                                PCCTreeNode* new_child, void (*free_data_func)(void*)) {
    if (parent == NULL || old_child == NULL || new_child == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    int index = pcc_tree_get_child_index(old_child);
    if (index == -1) {
        return PCC_ERROR_RUNTIME;
    }

    /* Set new child's parent */
    new_child->parent = parent;

    /* Replace in array */
    PCCError err = pcc_array_set(parent->children, (size_t)index, new_child);
    if (err != PCC_SUCCESS) {
        return err;
    }

    /* Free old child */
    pcc_tree_node_free(old_child, free_data_func);

    return PCC_SUCCESS;
}

/* Print tree structure (for debugging) */
static void pcc_tree_print_recursive(PCCTreeNode* node, int depth, void (*print_data_func)(void*)) {
    if (node == NULL) {
        return;
    }

    /* Print indentation */
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }

    /* Print node */
    printf("[Type: %d] ", node->node_type);
    if (print_data_func != NULL && node->data != NULL) {
        print_data_func(node->data);
    }
    printf("\n");

    /* Print children */
    for (size_t i = 0; i < pcc_array_size(node->children); i++) {
        PCCTreeNode* child = (PCCTreeNode*)pcc_array_get(node->children, i);
        pcc_tree_print_recursive(child, depth + 1, print_data_func);
    }
}

void pcc_tree_print(PCCTreeNode* node, void (*print_data_func)(void*)) {
    pcc_tree_print_recursive(node, 0, print_data_func);
}
