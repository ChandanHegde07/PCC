# DSA Mapping in PCC Compiler

This document maps the Data Structures and Algorithms (DSA) concepts used in the PCC compiler implementation.

## Data Structures

### 1. Arrays

**Implementation:** [`src/array.c`](../src/array.c), [`include/array.h`](../include/array.h)

**DSA Concept:** Dynamic Array with automatic resizing

**Usage in PCC:**
- Storing tokens in the lexer
- Storing AST nodes in lists
- Storing parameters, arguments, and constraints
- Storing statements and expressions

**Key Operations:**
| Operation | Time Complexity | Description |
|-----------|----------------|-------------|
| `push` | O(1) amortized | Add element to end |
| `pop` | O(1) amortized | Remove last element |
| `get` | O(1) | Access element by index |
| `set` | O(1) | Modify element at index |
| `insert` | O(n) | Insert at arbitrary position |
| `remove` | O(n) | Remove from arbitrary position |
| `find` | O(n) | Linear search |
| `sort` | O(n²) | Bubble sort (can be upgraded) |

**Memory Management:**
- Initial capacity: 16 elements
- Growth factor: 2x when full
- Shrink factor: 0.5x when size < capacity/4

### 2. Trees

**Implementation:** [`src/tree.c`](../src/tree.c), [`include/tree.h`](../include/tree.h)

**DSA Concept:** N-ary Tree with parent-child relationships

**Usage in PCC:**
- Abstract Syntax Tree (AST) representation
- Hierarchical code structure
- Expression trees

**Key Operations:**
| Operation | Time Complexity | Description |
|-----------|----------------|-------------|
| `add_child` | O(1) | Add child node |
| `remove_child` | O(n) | Remove child node |
| `get_child` | O(1) | Access child by index |
| `child_count` | O(1) | Get number of children |
| `depth` | O(h) | Calculate node depth |
| `height` | O(n) | Calculate tree height |
| `find_by_type` | O(n) | DFS search |
| `traverse` | O(n) | Tree traversal |

**Traversal Algorithms:**
- **Pre-order:** Parent → Children (DFS)
- **Post-order:** Children → Parent (DFS)
- **Level-order:** Breadth-first (BFS)

**Memory Management:**
- Each node stores: data, parent pointer, children array
- Children stored in dynamic array
- Recursive cleanup for subtree deletion

### 3. Hash Tables

**Implementation:** [`src/hashtable.c`](../src/hashtable.c), [`include/hashtable.h`](../include/hash_table.h)

**DSA Concept:** Hash Table with Separate Chaining

**Usage in PCC:**
- Symbol table for variable/template/prompt lookup
- Scope management
- Fast symbol resolution

**Key Operations:**
| Operation | Time Complexity | Description |
|-----------|----------------|-------------|
| `put` | O(1) average | Insert/update key-value |
| `get` | O(1) average | Lookup by key |
| `remove` | O(1) average | Delete key-value |
| `contains` | O(1) average | Check key existence |
| `resize` | O(n) | Rehash all entries |

**Hash Function:**
- Algorithm: DJB2
- Formula: `hash = ((hash << 5) + hash) + c`
- Properties: Good distribution, fast computation

**Collision Resolution:**
- Method: Separate chaining (linked lists)
- Load factor threshold: 0.75
- Resize factor: 2x when threshold exceeded

## Algorithms

### 1. Lexical Analysis (Finite Automaton)

**Implementation:** [`src/lexer.c`](../src/lexer.c)

**DSA Concept:** Finite State Machine (FSM)

**Algorithm:**
```
State 0: Start
  - Read character
  - If letter/underscore → State 1 (identifier)
  - If digit → State 2 (number)
  - If quote → State 3 (string)
  - If operator → State 4 (operator)
  - If whitespace → State 0
```

**Time Complexity:** O(n) where n is source length

### 2. Parsing (Recursive Descent)

**Implementation:** [`src/parser.c`](../src/parser.c)

**DSA Concept:** Recursive Descent Parsing

**Algorithm:**
```
parse_program():
    statements = []
    while not EOF:
        stmt = parse_statement()
        statements.append(stmt)
    return Program(statements)

parse_statement():
    if token == PROMPT: return parse_prompt_def()
    if token == VAR: return parse_var_decl()
    if token == TEMPLATE: return parse_template_def()
    ...
```

**Time Complexity:** O(n) where n is number of tokens

### 3. Semantic Analysis (Tree Traversal)

**Implementation:** [`src/semantic.c`](../src/semantic.c)

**DSA Concept:** AST Visitor Pattern (Tree Traversal)

**Algorithm:**
```
analyze(node):
    switch node.type:
        case PROGRAM:
            for child in node.statements:
                analyze(child)
        case VAR_DECL:
            add_to_symbol_table(node.name)
            analyze(node.initializer)
        case VARIABLE_REF:
            check_symbol_defined(node.name)
        ...
```

**Time Complexity:** O(n) where n is number of AST nodes

### 4. Optimization (Tree Transformation)

**Implementation:** [`src/optimizer.c`](../src/optimizer.c)

**DSA Concept:** Pattern Matching + Tree Transformation

**Constant Folding Algorithm:**
```
fold_constant(node):
    if node is binary expression:
        left = fold_constant(node.left)
        right = fold_constant(node.right)
        if left and right are constants:
            result = evaluate(left, right, node.operator)
            return Constant(result)
        return Binary(node.operator, left, right)
```

**Dead Code Elimination Algorithm:**
```
eliminate_dead_code(node):
    if node is IF statement:
        condition = evaluate(node.condition)
        if condition is TRUE:
            return node.then_body
        if condition is FALSE:
            return node.else_body or NULL
```

**Time Complexity:** O(n) where n is number of AST nodes

### 5. Code Generation (Tree Traversal)

**Implementation:** [`src/codegen.c`](../src/codegen.c)

**DSA Concept:** Tree Traversal for Code Generation

**Algorithm:**
```
generate(node):
    switch node.type:
        case PROGRAM:
            output = "{"
            for stmt in node.statements:
                output += generate(stmt)
            output += "}"
        case STRING_LITERAL:
            output = '"' + escape(node.value) + '"'
        ...
    return output
```

**Time Complexity:** O(n) where n is number of AST nodes

## Algorithm Complexity Summary

| Phase | Algorithm | Time Complexity | Space Complexity |
|-------|-----------|-----------------|------------------|
| Lexical Analysis | Finite Automaton | O(n) | O(n) |
| Parsing | Recursive Descent | O(n) | O(n) |
| Semantic Analysis | Tree Traversal | O(n) | O(n) |
| Optimization | Tree Transformation | O(n) | O(n) |
| Code Generation | Tree Traversal | O(n) | O(n) |

**Overall Complexity:** O(n) for each phase, where n is the size of the input

## Memory Usage Analysis

| Component | Memory Usage | Notes |
|-----------|--------------|-------|
| Lexer | O(n) | Token storage |
| Parser | O(n) | AST construction |
| Symbol Table | O(s) | s = number of symbols |
| Optimizer | O(n) | In-place transformation |
| Code Generator | O(n) | Output buffer |

## Recursion Usage

PCC uses recursion in several components:

1. **Parser:** Recursive descent parsing
   - Maximum depth: Expression nesting level
   - Stack usage: O(depth)

2. **Semantic Analyzer:** AST traversal
   - Maximum depth: AST height
   - Stack usage: O(height)

3. **Optimizer:** Tree transformation
   - Maximum depth: AST height
   - Stack usage: O(height)

4. **Code Generator:** Tree traversal
   - Maximum depth: AST height
   - Stack usage: O(height)

## Graph Concepts

While PCC doesn't explicitly use graph data structures, several graph concepts are implicitly used:

1. **AST as a Directed Acyclic Graph (DAG):**
   - Nodes represent program constructs
   - Edges represent parent-child relationships
   - No cycles (acyclic)

2. **Control Flow Graph (CFG):**
   - Implicit in IF/FOR/WHILE statements
   - Used in dead code elimination

3. **Dependency Graph:**
   - Symbol dependencies in semantic analysis
   - Template call dependencies

## Performance Considerations

1. **Hash Table Load Factor:**
   - Target: < 0.75
   - Ensures O(1) average case operations

2. **Array Resizing:**
   - Amortized O(1) for push/pop
   - Reduces memory allocations

3. **Tree Traversal:**
   - Single pass for most operations
   - No redundant traversals

4. **Memory Locality:**
   - Arrays provide good cache performance
   - Contiguous memory access patterns

## Future DSA Enhancements

1. **Balanced Binary Search Tree:**
   - For ordered symbol storage
   - O(log n) lookup, insert, delete

2. **Graph Algorithms:**
   - Dependency analysis
   - Cycle detection

3. **Advanced Sorting:**
   - Replace bubble sort with quicksort/mergesort
   - O(n log n) sorting

4. **String Algorithms:**
   - KMP for pattern matching
   - Rabin-Karp for substring search
