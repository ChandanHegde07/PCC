# PCC - Prompt Compiler Compiler

A lightweight compiler written in C that converts a domain-specific language (DSL) into structured and optimized prompts for Large Language Model (LLM) systems.

## Overview

PCC (Prompt Compiler Compiler) is a modular compiler that implements a complete compilation pipeline from DSL source code to optimized prompt representations in JSON, text, or markdown format.

## Features

- **Modular Architecture:** Separate components for lexical analysis, parsing, AST construction, semantic analysis, optimization, and code generation
- **Core DSA Implementation:** Arrays, trees, hash tables, recursion, and graph traversal
- **Explicit Memory Management:** Manual memory management using `malloc`/`free`
- **Multiple Output Formats:** JSON, text, and markdown output
- **Optimization Passes:** Constant folding, dead code elimination
- **Comprehensive Error Reporting:** Line/column tracking for all errors

## Project Structure

```
PCC/
├── src/              # Compiler implementation
│   ├── main.c       # Main entry point
│   ├── lexer.c      # Lexical analyzer
│   ├── parser.c     # Parser (recursive descent)
│   ├── ast.c        # AST node operations
│   ├── symbol_table.c  # Symbol table
│   ├── semantic.c   # Semantic analyzer
│   ├── optimizer.c  # Optimizer
│   ├── codegen.c    # Code generator
│   ├── array.c      # Dynamic array
│   ├── tree.c       # Tree data structure
│   └── hashtable.c  # Hash table
├── include/         # Header files
│   ├── common.h     # Common definitions
│   ├── lexer.h
│   ├── parser.h
│   ├── ast.h
│   ├── symbol_table.h
│   ├── semantic.h
│   ├── optimizer.h
│   ├── codegen.h
│   ├── array.h
│   ├── tree.h
│   └── hashtable.h
├── docs/            # Documentation
│   ├── grammar.md   # DSL grammar specification
│   ├── architecture.md  # Compiler architecture
│   └── dsa_mapping.md   # DSA concepts mapping
├── examples/        # Example programs
│   ├── valid_simple.pcc
│   ├── valid_template.pcc
│   ├── valid_conditional.pcc
│   ├── invalid_undefined_var.pcc
│   ├── invalid_syntax.pcc
│   └── invalid_redefined.pcc
├── tests/           # Unit tests
│   ├── test_runner.c
│   └── test_array.c
├── outputs/         # Generated output files
├── Makefile         # Build configuration
└── Readme.md        # This file
```

## Building

### Prerequisites

- GCC or compatible C compiler
- Make utility
- Standard C library

### Compilation

```bash
# Build the compiler
make

# Build with debug symbols
make debug

# Clean build artifacts
make clean
```

## Usage

### Basic Usage

```bash
# Compile a PCC file to JSON
./build/pcc input.pcc output.json

# Compile with optimizations
./build/pcc -O input.pcc output.json

# Compile to text format
./build/pcc -f text input.pcc output.txt

# Compile to markdown format
./build/pcc -f markdown input.pcc output.md
```

### Command Line Options

| Option | Description |
|--------|-------------|
| `-h, --help` | Show help message |
| `-v, --version` | Show version information |
| `-o <file>` | Specify output file |
| `-f <format>` | Output format: json, text, markdown |
| `-O` | Enable optimizations |
| `--no-optimize` | Disable optimizations |
| `--debug` | Enable debug output |

### Examples

```bash
# Simple compilation
./build/pcc examples/valid_simple.pcc outputs/output.json

# With optimizations and debug output
./build/pcc -O --debug examples/valid_template.pcc outputs/output.json

# Text output
./build/pcc -f text examples/valid_conditional.pcc outputs/output.txt
```

## DSL Language Reference

### Variable Declaration

```
VAR name = "Alice";
VAR count = 42;
```

### Prompt Definition

```
PROMPT greeting {
    "Hello, $name! Welcome to $systemName."
}
```

### Template Definition

```
TEMPLATE formatSection(title, content) {
    "## $title\n$content"
}
```

### Conditional Statements

```
IF debugMode {
    "DEBUG: Enabled"
} ELSE {
    "Production mode"
}
```

### Loop Statements

```
FOR item IN items {
    "- $item"
}

WHILE condition {
    "Processing..."
}
```

### Output Specification

```
OUTPUT greeting AS JSON;
```

## Data Structures and Algorithms

### Arrays
- Dynamic array with automatic resizing
- O(1) amortized push/pop
- O(n) insert/remove at index

### Trees
- N-ary tree for AST representation
- Pre-order, post-order, level-order traversal
- O(n) search and traversal

### Hash Tables
- Separate chaining for collision resolution
- DJB2 hash function
- O(1) average case operations

### Algorithms
- **Lexical Analysis:** Finite automaton
- **Parsing:** Recursive descent
- **Semantic Analysis:** Tree traversal (visitor pattern)
- **Optimization:** Pattern matching + tree transformation
- **Code Generation:** Tree traversal

## Testing

```bash
# Run all tests
make test

# Run specific test
./build/test_runner
```

## Documentation

- [`docs/grammar.md`](docs/grammar.md) - Complete DSL grammar specification
- [`docs/architecture.md`](docs/architecture.md) - Compiler architecture overview
- [`docs/dsa_mapping.md`](docs/dsa_mapping.md) - DSA concepts mapping

## Compiler Pipeline

```
Source Code (.pcc)
       ↓
   Lexer (Tokenization)
       ↓
   Parser (AST Construction)
       ↓
   Semantic Analyzer (Type Checking)
       ↓
   Optimizer (Constant Folding, DCE)
       ↓
   Code Generator (JSON/Text/Markdown)
       ↓
   Output File
```

## Error Handling

The compiler provides comprehensive error reporting:

- **Lexical Errors:** Invalid characters, unterminated strings
- **Syntax Errors:** Unexpected tokens, missing delimiters
- **Semantic Errors:** Undefined symbols, type mismatches
- **Runtime Errors:** Memory allocation failures

Each error includes:
- Error message
- Source position (line, column, filename)
- Error code

## Memory Management

PCC uses explicit memory management:
- All data structures are dynamically allocated
- Each component provides cleanup functions
- Memory is freed in reverse order of allocation
- No garbage collection

## License

This project is provided as-is for educational purposes.

## Version

Current version: 1.0.0
