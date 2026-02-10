# PCC Compiler Architecture

## Overview

PCC (Prompt Compiler Compiler) is a lightweight compiler written in C that converts a domain-specific language (DSL) into structured and optimized prompts for Large Language Model (LLM) systems.

## Compiler Pipeline

The PCC compiler follows a traditional multi-phase compiler architecture:

```
Source Code (.pcc)
       |
       v
+--------------+
|   Lexer      |  Tokenization
+--------------+
       |
       v
+--------------+
|   Parser     |  AST Construction
+--------------+
       |
       v
+--------------+
|  Semantic    |  Type Checking & Scope Validation
|  Analyzer    |
+--------------+
       |
       v
+--------------+
|  Optimizer   |  Constant Folding, Dead Code Elimination
+--------------+
       |
       v
+--------------+
| Code Gen     |  JSON/Text/Markdown Output
+--------------+
       |
       v
  Output File
```

## Components

### 1. Lexical Analyzer (Lexer)

**File:** [`src/lexer.c`](../src/lexer.c), [`include/lexer.h`](../include/lexer.h)

The lexer converts the source code into a stream of tokens. It uses a finite automaton approach for pattern matching.

**Key Features:**
- Keyword recognition
- String, number, and boolean literal parsing
- Operator and punctuation tokenization
- Comment handling (single-line `//` and multi-line `/* */`)
- Line and column tracking for error reporting

**DSA Concept:** Finite automaton for pattern matching

### 2. Parser

**File:** [`src/parser.c`](../src/parser.c), [`include/parser.h`](../include/parser.h)

The parser uses recursive descent parsing to construct an Abstract Syntax Tree (AST) from the token stream.

**Key Features:**
- Recursive descent parsing
- AST node creation for all language constructs
- Error recovery and reporting
- Precedence and associativity handling

**DSA Concept:** Recursive descent parsing (tree construction)

### 3. Abstract Syntax Tree (AST)

**File:** [`src/ast.c`](../src/ast.c), [`include/ast.h`](../include/ast.h)

The AST represents the syntactic structure of the program in a hierarchical tree format.

**Node Types:**
- `AST_PROGRAM` - Root node
- `AST_PROMPT_DEF` - Prompt definition
- `AST_VAR_DECL` - Variable declaration
- `AST_TEMPLATE_DEF` - Template definition
- `AST_CONSTRAINT_DEF` - Constraint definition
- `AST_OUTPUT_SPEC` - Output specification
- `AST_IF_STMT`, `AST_FOR_STMT`, `AST_WHILE_STMT` - Control flow
- `AST_BINARY_EXPR`, `AST_UNARY_EXPR` - Expressions
- `AST_TEXT_ELEMENT`, `AST_VARIABLE_REF` - Prompt elements

**DSA Concept:** Tree data structure

### 4. Symbol Table

**File:** [`src/symbol_table.c`](../src/symbol_table.c), [`include/symbol_table.h`](../include/symbol_table.h)

The symbol table tracks all defined symbols (variables, templates, prompts, constraints) with scope management.

**Key Features:**
- Hierarchical scope management
- Symbol type tracking
- Definition and usage tracking
- Hash table implementation for O(1) lookups

**DSA Concept:** Hash table with scope management

### 5. Semantic Analyzer

**File:** [`src/semantic.c`](../src/semantic.c), [`include/semantic.h`](../include/semantic.h)

The semantic analyzer performs type checking, scope validation, and other semantic checks on the AST.

**Key Features:**
- Symbol definition validation
- Undefined symbol detection
- Type checking
- Scope validation

**DSA Concept:** Tree traversal (AST visitor pattern)

### 6. Optimizer

**File:** [`src/optimizer.c`](../src/optimizer.c), [`include/optimizer.h`](../include/optimizer.h)

The optimizer performs various transformations on the AST to improve efficiency.

**Optimization Passes:**
- **Constant Folding:** Evaluates constant expressions at compile time
- **Dead Code Elimination:** Removes unreachable code (e.g., always-true/false conditions)
- **Unused Removal:** Removes unused variables (planned)
- **Template Inlining:** Inlines simple templates (planned)

**DSA Concept:** Tree transformation and pattern matching

### 7. Code Generator

**File:** [`src/codegen.c`](../src/codegen.c), [`include/codegen.h`](../include/codegen.h)

The code generator converts the optimized AST into the desired output format.

**Output Formats:**
- **JSON:** Structured representation of the prompt
- **TEXT:** Plain text output
- **MARKDOWN:** Formatted markdown output

**DSA Concept:** Tree traversal for code generation

## Data Structures

### Dynamic Array

**File:** [`src/array.c`](../src/array.c), [`include/array.h`](../include/array.h)

A generic dynamic array implementation with automatic resizing.

**Operations:**
- `push`, `pop`, `get`, `set`
- `insert`, `remove`
- `find`, `contains`
- `sort`, `reverse`

**Time Complexity:**
- Access: O(1)
- Insert/Delete at end: O(1) amortized
- Insert/Delete at index: O(n)
- Search: O(n)

### Tree

**File:** [`src/tree.c`](../src/tree.c), [`include/tree.h`](../include/tree.h)

A generic tree structure used for AST representation.

**Operations:**
- `add_child`, `remove_child`
- `get_child`, `child_count`
- `find_by_type`, `find_all_by_type`
- `traverse` (pre-order, post-order, level-order)
- `depth`, `height`

**Time Complexity:**
- Access: O(1) with reference
- Traversal: O(n)
- Search: O(n)

### Hash Table

**File:** [`src/hashtable.c`](../src/hashtable.c), [`include/hashtable.h`](../include/hashtable.h)

A hash table with separate chaining for collision resolution.

**Operations:**
- `put`, `get`, `remove`
- `contains`, `clear`
- `resize`, `get_keys`, `get_values`

**Time Complexity:**
- Average case: O(1)
- Worst case: O(n)

## Memory Management

PCC uses explicit memory management with `malloc` and `free`:

- All data structures are dynamically allocated
- Each component provides cleanup functions
- Memory is freed in reverse order of allocation
- No garbage collection - manual memory management required

## Error Handling

The compiler uses a comprehensive error reporting system:

1. **Lexical Errors:** Invalid characters, unterminated strings
2. **Syntax Errors:** Unexpected tokens, missing delimiters
3. **Semantic Errors:** Undefined symbols, type mismatches
4. **Runtime Errors:** Memory allocation failures

Each error includes:
- Error message
- Source position (line, column, filename)
- Error code

## Build System

**File:** [`Makefile`](../Makefile)

The Makefile provides targets for:
- Building the compiler (`make`)
- Debug builds (`make debug`)
- Running tests (`make test`)
- Running examples (`make run-example`)
- Installation (`make install`)

## Testing

**Directory:** [`tests/`](../tests/)

Unit tests are provided for each component:
- `test_array.c` - Array data structure tests
- `test_runner.c` - Main test runner

## Examples

**Directory:** [`examples/`](../examples/)

Example programs demonstrating:
- `valid_simple.pcc` - Simple valid program
- `valid_template.pcc` - Template usage
- `valid_conditional.pcc` - Conditional statements
- `invalid_undefined_var.pcc` - Undefined variable error
- `invalid_syntax.pcc` - Syntax error
- `invalid_redefined.pcc` - Redefinition error

## Usage

```bash
# Build the compiler
make

# Compile a PCC file
./build/pcc input.pcc output.json

# Compile with optimizations
./build/pcc -O input.pcc output.json

# Compile with text output
./build/pcc -f text input.pcc output.txt

# Run tests
make test
```

## Future Enhancements

1. Additional optimization passes
2. More output formats (XML, YAML)
3. Interactive mode
4. Linting and style checking
5. Code completion support
6. IDE integration
