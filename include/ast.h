/**
 * @file ast.h
 * @brief Abstract Syntax Tree (AST) node definitions for PCC compiler
 * @details Defines all AST node types for the DSL
 *          DSA Concept: Tree structure for representing program structure
 */

#ifndef PCC_AST_H
#define PCC_AST_H

#include "common.h"
#include "tree.h"

/* AST node types */
typedef enum {
    /* Program */
    AST_PROGRAM,

    /* Statements */
    AST_PROMPT_DEF,
    AST_VAR_DECL,
    AST_TEMPLATE_DEF,
    AST_CONSTRAINT_DEF,
    AST_OUTPUT_SPEC,

    /* Expressions */
    AST_IDENTIFIER,
    AST_STRING_LITERAL,
    AST_NUMBER_LITERAL,
    AST_BOOLEAN_LITERAL,
    AST_BINARY_EXPR,
    AST_UNARY_EXPR,
    AST_VARIABLE_REF,
    AST_TEMPLATE_CALL,
    AST_FUNCTION_CALL,

    /* Control flow */
    AST_IF_STMT,
    AST_FOR_STMT,
    AST_WHILE_STMT,

    /* Prompt elements */
    AST_TEXT_ELEMENT,
    AST_VARIABLE_ELEMENT,
    AST_TEMPLATE_ELEMENT,

    /* Constraints */
    AST_CONSTRAINT_EXPR,

    /* Output format */
    AST_OUTPUT_FORMAT,

    /* Lists */
    AST_STATEMENT_LIST,
    AST_EXPRESSION_LIST,
    AST_PARAMETER_LIST,
    AST_ARGUMENT_LIST,
    AST_CONSTRAINT_LIST,
    AST_ELEMENT_LIST,

    /* Special */
    AST_EMPTY
} ASTNodeType;

/* AST node data structures */

/* Identifier */
typedef struct {
    char* name;
} ASTIdentifier;

/* Literal values */
typedef struct {
    char* string_val;
    double number_val;
    int bool_val;
} ASTLiteral;

/* Binary expression */
typedef struct {
    int op_type;   /* Token type */
    void* left;    /* AST node */
    void* right;   /* AST node */
} ASTBinaryExpr;

/* Unary expression */
typedef struct {
    int op_type;   /* Token type */
    void* operand; /* AST node */
} ASTUnaryExpr;

/* Variable reference */
typedef struct {
    char* name;
} ASTVariableRef;

/* Function/Template call */
typedef struct {
    char* name;
    PCCArray* arguments;  /* Array of AST nodes */
} ASTCallExpr;

/* Variable declaration */
typedef struct {
    char* name;
    void* initializer;   /* AST node */
} ASTVarDecl;

/* Template definition */
typedef struct {
    char* name;
    PCCArray* parameters; /* Array of identifiers */
    void* body;          /* AST node (element list) */
} ASTTemplateDef;

/* Prompt definition */
typedef struct {
    char* name;
    void* body;          /* AST node (element list) */
} ASTPromptDef;

/* Constraint definition */
typedef struct {
    char* name;
    PCCArray* constraints; /* Array of constraint expressions */
} ASTConstraintDef;

/* Constraint expression */
typedef struct {
    char* variable;
    int op_type;
    void* value;         /* AST node */
} ASTConstraintExpr;

/* Output specification */
typedef struct {
    char* name;
    int format;          /* Output format type */
} ASTOutputSpec;

/* If statement */
typedef struct {
    void* condition;     /* AST node */
    void* then_body;     /* AST node */
    void* else_body;     /* AST node (can be NULL) */
} ASTIfStmt;

/* For statement */
typedef struct {
    char* variable;
    void* iterable;      /* AST node */
    void* body;          /* AST node */
} ASTForStmt;

/* While statement */
typedef struct {
    void* condition;     /* AST node */
    void* body;          /* AST node */
} ASTWhileStmt;

/* Text element */
typedef struct {
    char* text;
    int is_raw;          /* 1 if raw text, 0 otherwise */
} ASTTextElement;

/* Program */
typedef struct {
    PCCArray* statements; /* Array of AST nodes */
} ASTProgram;

/* Generic AST node */
typedef struct {
    ASTNodeType type;
    void* data;
    PCCPosition position;
} ASTNode;

/* Function prototypes */

/**
 * @brief Create a new AST node
 * @param type Node type
 * @param data Node data
 * @param position Source position
 * @return New AST node, or NULL on failure
 */
ASTNode* ast_node_create(ASTNodeType type, void* data, PCCPosition position);

/**
 * @brief Free an AST node and all its children
 * @param node Node to free
 */
void ast_node_free(ASTNode* node);

/**
 * @brief Create an identifier node
 * @param name Identifier name
 * @param position Source position
 * @return New identifier node
 */
ASTNode* ast_identifier_create(const char* name, PCCPosition position);

/**
 * @brief Create a string literal node
 * @param value String value
 * @param position Source position
 * @return New string literal node
 */
ASTNode* ast_string_literal_create(const char* value, PCCPosition position);

/**
 * @brief Create a number literal node
 * @param value Number value
 * @param position Source position
 * @return New number literal node
 */
ASTNode* ast_number_literal_create(double value, PCCPosition position);

/**
 * @brief Create a boolean literal node
 * @param value Boolean value
 * @param position Source position
 * @return New boolean literal node
 */
ASTNode* ast_boolean_literal_create(int value, PCCPosition position);

/**
 * @brief Create a binary expression node
 * @param op_type Operator token type
 * @param left Left operand
 * @param right Right operand
 * @param position Source position
 * @return New binary expression node
 */
ASTNode* ast_binary_expr_create(int op_type, ASTNode* left, ASTNode* right, PCCPosition position);

/**
 * @brief Create a unary expression node
 * @param op_type Operator token type
 * @param operand Operand
 * @param position Source position
 * @return New unary expression node
 */
ASTNode* ast_unary_expr_create(int op_type, ASTNode* operand, PCCPosition position);

/**
 * @brief Create a variable reference node
 * @param name Variable name
 * @param position Source position
 * @return New variable reference node
 */
ASTNode* ast_variable_ref_create(const char* name, PCCPosition position);

/**
 * @brief Create a function call node
 * @param name Function name
 * @param arguments Array of argument nodes
 * @param position Source position
 * @return New function call node
 */
ASTNode* ast_function_call_create(const char* name, PCCArray* arguments, PCCPosition position);

/**
 * @brief Create a variable declaration node
 * @param name Variable name
 * @param initializer Initializer expression
 * @param position Source position
 * @return New variable declaration node
 */
ASTNode* ast_var_decl_create(const char* name, ASTNode* initializer, PCCPosition position);

/**
 * @brief Create a template definition node
 * @param name Template name
 * @param parameters Array of parameter names
 * @param body Template body
 * @param position Source position
 * @return New template definition node
 */
ASTNode* ast_template_def_create(const char* name, PCCArray* parameters, ASTNode* body, PCCPosition position);

/**
 * @brief Create a prompt definition node
 * @param name Prompt name
 * @param body Prompt body
 * @param position Source position
 * @return New prompt definition node
 */
ASTNode* ast_prompt_def_create(const char* name, ASTNode* body, PCCPosition position);

/**
 * @brief Create a constraint definition node
 * @param name Constraint name
 * @param constraints Array of constraint expressions
 * @param position Source position
 * @return New constraint definition node
 */
ASTNode* ast_constraint_def_create(const char* name, PCCArray* constraints, PCCPosition position);

/**
 * @brief Create a constraint expression node
 * @param variable Variable name
 * @param op_type Operator
 * @param value Value node
 * @param position Source position
 * @return New constraint expression node
 */
ASTNode* ast_constraint_expr_create(const char* variable, int op_type, ASTNode* value, PCCPosition position);

/**
 * @brief Create an output specification node
 * @param name Output name
 * @param format Output format
 * @param position Source position
 * @return New output specification node
 */
ASTNode* ast_output_spec_create(const char* name, int format, PCCPosition position);

/**
 * @brief Create an if statement node
 * @param condition Condition expression
 * @param then_body Then body
 * @param else_body Else body (can be NULL)
 * @param position Source position
 * @return New if statement node
 */
ASTNode* ast_if_stmt_create(ASTNode* condition, ASTNode* then_body, ASTNode* else_body, PCCPosition position);

/**
 * @brief Create a for statement node
 * @param variable Loop variable
 * @param iterable Iterable expression
 * @param body Loop body
 * @param position Source position
 * @return New for statement node
 */
ASTNode* ast_for_stmt_create(const char* variable, ASTNode* iterable, ASTNode* body, PCCPosition position);

/**
 * @brief Create a while statement node
 * @param condition Condition expression
 * @param body Loop body
 * @param position Source position
 * @return New while statement node
 */
ASTNode* ast_while_stmt_create(ASTNode* condition, ASTNode* body, PCCPosition position);

/**
 * @brief Create a text element node
 * @param text Text content
 * @param is_raw Whether text is raw
 * @param position Source position
 * @return New text element node
 */
ASTNode* ast_text_element_create(const char* text, int is_raw, PCCPosition position);

/**
 * @brief Create a list node
 * @param type List type (statement_list, expression_list, etc.)
 * @param elements Array of elements
 * @param position Source position
 * @return New list node
 */
ASTNode* ast_list_create(ASTNodeType type, PCCArray* elements, PCCPosition position);

/**
 * @brief Create a program node
 * @param statements Array of statement nodes
 * @return New program node
 */
ASTNode* ast_program_create(PCCArray* statements);

/**
 * @brief Get AST node type name as string
 * @param type Node type
 * @return String representation
 */
const char* ast_node_type_name(ASTNodeType type);

/**
 * @brief Print AST tree (for debugging)
 * @param node Root node
 */
void ast_print(ASTNode* node);

#endif /* PCC_AST_H */
