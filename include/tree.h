/**
 * @file tree.h
 * @brief Tree data structure implementation for PCC compiler
 * @details Implements a generic tree structure for AST representation
 *          DSA Concept: Tree data structure with parent-child relationships
 */

#ifndef PCC_TREE_H
#define PCC_TREE_H

#include "common.h"
#include "array.h"

/* Forward declaration */
typedef struct PCCTreeNode PCCTreeNode;

/* Tree node structure */
struct PCCTreeNode {
    void* data;                    /* Node data (can be any type) */
    PCCTreeNode* parent;           /* Parent node (NULL for root) */
    PCCArray* children;            /* Array of child nodes */
    int node_type;                 /* Node type identifier */
    PCCPosition position;          /* Source code position */
};

/* Tree structure */
typedef struct {
    PCCTreeNode* root;             /* Root node of the tree */
    size_t node_count;             /* Total number of nodes */
} PCCTree;

/* Tree traversal order */
typedef enum {
    PCC_TRAVERSAL_PRE_ORDER,       /* Parent, then children */
    PCC_TRAVERSAL_IN_ORDER,        /* Left, parent, right (for binary trees) */
    PCC_TRAVERSAL_POST_ORDER,      /* Children, then parent */
    PCC_TRAVERSAL_LEVEL_ORDER      /* Breadth-first traversal */
} PCCTraversalOrder;

/* Visitor function for tree traversal */
typedef void (*PCCTreeVisitor)(PCCTreeNode* node, void* user_data);

/* Function prototypes */

/**
 * @brief Create a new tree node
 * @param data Node data
 * @param node_type Node type identifier
 * @return Pointer to new node, or NULL on failure
 */
PCCTreeNode* pcc_tree_node_create(void* data, int node_type);

/**
 * @brief Free a tree node and all its descendants
 * @param node Node to free
 * @param free_data_func Function to free node data (can be NULL)
 */
void pcc_tree_node_free(PCCTreeNode* node, void (*free_data_func)(void*));

/**
 * @brief Create a new tree
 * @param root_data Data for root node
 * @param root_type Type of root node
 * @return Pointer to new tree, or NULL on failure
 */
PCCTree* pcc_tree_create(void* root_data, int root_type);

/**
 * @brief Free a tree and all its nodes
 * @param tree Tree to free
 * @param free_data_func Function to free node data
 */
void pcc_tree_free(PCCTree* tree, void (*free_data_func)(void*));

/**
 * @brief Add a child node to a parent
 * @param parent Parent node
 * @param child Child node to add
 * @return PCC_SUCCESS on success, error code on failure
 */
PCCError pcc_tree_add_child(PCCTreeNode* parent, PCCTreeNode* child);

/**
 * @brief Remove a child node from parent
 * @param parent Parent node
 * @param child Child node to remove
 * @param free_data_func Function to free child data
 * @return PCC_SUCCESS on success, error code on failure
 */
PCCError pcc_tree_remove_child(PCCTreeNode* parent, PCCTreeNode* child, void (*free_data_func)(void*));

/**
 * @brief Get child at index
 * @param parent Parent node
 * @param index Child index
 * @return Child node, or NULL if index out of bounds
 */
PCCTreeNode* pcc_tree_get_child(PCCTreeNode* parent, size_t index);

/**
 * @brief Get number of children
 * @param node Node to query
 * @return Number of children
 */
size_t pcc_tree_child_count(PCCTreeNode* node);

/**
 * @brief Check if node is root
 * @param node Node to check
 * @return PCC_TRUE if root, PCC_FALSE otherwise
 */
PCCBool pcc_tree_is_root(PCCTreeNode* node);

/**
 * @brief Check if node is leaf
 * @param node Node to check
 * @return PCC_TRUE if leaf, PCC_FALSE otherwise
 */
PCCBool pcc_tree_is_leaf(PCCTreeNode* node);

/**
 * @brief Get depth of node (distance from root)
 * @param node Node to query
 * @return Depth of node (0 for root)
 */
int pcc_tree_depth(PCCTreeNode* node);

/**
 * @brief Get height of node (longest path to leaf)
 * @param node Node to query
 * @return Height of node (0 for leaf)
 */
int pcc_tree_height(PCCTreeNode* node);

/**
 * @brief Find node by type using DFS
 * @param node Starting node
 * @param node_type Type to find
 * @return First matching node, or NULL if not found
 */
PCCTreeNode* pcc_tree_find_by_type(PCCTreeNode* node, int node_type);

/**
 * @brief Find all nodes of a given type
 * @param node Starting node
 * @param node_type Type to find
 * @param results Array to store results
 * @return Number of nodes found
 */
size_t pcc_tree_find_all_by_type(PCCTreeNode* node, int node_type, PCCArray* results);

/**
 * @brief Traverse tree with visitor function
 * @param node Starting node
 * @param order Traversal order
 * @param visitor Visitor function
 * @param user_data User data passed to visitor
 */
void pcc_tree_traverse(PCCTreeNode* node, PCCTraversalOrder order,
                       PCCTreeVisitor visitor, void* user_data);

/**
 * @brief Clone a tree node and all its descendants
 * @param node Node to clone
 * @param clone_data_func Function to clone node data (can be NULL)
 * @return Cloned node, or NULL on failure
 */
PCCTreeNode* pcc_tree_node_clone(PCCTreeNode* node, void* (*clone_data_func)(void*));

/**
 * @brief Get sibling at offset
 * @param node Node to query
 * @param offset Offset from current node (positive = right, negative = left)
 * @return Sibling node, or NULL if not found
 */
PCCTreeNode* pcc_tree_get_sibling(PCCTreeNode* node, int offset);

/**
 * @brief Get index of node in parent's children
 * @param node Node to query
 * @return Index in parent's children, or -1 if node is root
 */
int pcc_tree_get_child_index(PCCTreeNode* node);

/**
 * @brief Replace child node with another
 * @param parent Parent node
 * @param old_child Child to replace
 * @param new_child New child
 * @param free_data_func Function to free old child data
 * @return PCC_SUCCESS on success, error code on failure
 */
PCCError pcc_tree_replace_child(PCCTreeNode* parent, PCCTreeNode* old_child,
                                PCCTreeNode* new_child, void (*free_data_func)(void*));

/**
 * @brief Print tree structure (for debugging)
 * @param node Root node
 * @param print_data_func Function to print node data
 */
void pcc_tree_print(PCCTreeNode* node, void (*print_data_func)(void*));

#endif /* PCC_TREE_H */
