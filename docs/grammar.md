# PCC DSL Grammar Specification

## Overview
PCC (Prompt Compiler Compiler) is a domain-specific language for creating structured and optimized prompts for Large Language Model (LLM) systems.

## Grammar Definition (EBNF)

```
<program> ::= <statement_list>

<statement_list> ::= <statement> | <statement> <statement_list>

<statement> ::= <prompt_definition>
              | <variable_declaration>
              | <template_definition>
              | <constraint_definition>
              | <output_specification>

<prompt_definition> ::= "PROMPT" <identifier> "{" <prompt_body> "}"

<prompt_body> ::= <prompt_element_list>

<prompt_element_list> ::= <prompt_element> | <prompt_element> <prompt_element_list>

<prompt_element> ::= <text_literal>
                   | <variable_reference>
                   | <template_call>
                   | <conditional_block>
                   | <loop_block>

<variable_declaration> ::= "VAR" <identifier> "=" <expression> ";"

<template_definition> ::= "TEMPLATE" <identifier> "(" <parameter_list> ")" "{" <prompt_body> "}"

<parameter_list> ::= <identifier> | <identifier> "," <parameter_list> | ε

<constraint_definition> ::= "CONSTRAINT" <identifier> "{" <constraint_body> "}"

<constraint_body> ::= <constraint_list>

<constraint_list> ::= <constraint> | <constraint> <constraint_list>

<constraint> ::= <identifier> <constraint_operator> <value> ";"

<constraint_operator> ::= "==" | "!=" | "<" | ">" | "<=" | ">=" | "IN" | "NOT_IN"

<output_specification> ::= "OUTPUT" <identifier> "AS" <output_format> ";"

<output_format> ::= "JSON" | "TEXT" | "MARKDOWN"

<conditional_block> ::= "IF" <condition> "{" <prompt_body> "}" 
                       | "IF" <condition> "{" <prompt_body> "}" "ELSE" "{" <prompt_body> "}"

<condition> ::= <expression> <logical_operator> <expression>

<logical_operator> ::= "AND" | "OR" | "NOT"

<loop_block> ::= "FOR" <identifier> "IN" <expression> "{" <prompt_body> "}"
               | "WHILE" <condition> "{" <prompt_body> "}"

<expression> ::= <term> <binary_operator> <term>
               | <unary_operator> <term>
               | <term>

<term> ::= <identifier> | <literal> | <function_call> | "(" <expression> ")"

<function_call> ::= <identifier> "(" <argument_list> ")"

<argument_list> ::= <expression> | <expression> "," <argument_list> | ε

<variable_reference> ::= "$" <identifier>

<template_call> ::= "@" <identifier> "(" <argument_list> ")"

<text_literal> ::= <string_literal> | <raw_text>

<string_literal> ::= '"' <string_content> '"'

<raw_text> ::= "RAW" <string_literal>

<identifier> ::= <letter> { <letter> | <digit> | "_" }

<letter> ::= [a-zA-Z]

<digit> ::= [0-9]

<literal> ::= <string_literal> | <number_literal> | <boolean_literal>

<number_literal> ::= <integer> | <float>

<integer> ::= <digit> { <digit> }

<float> ::= <digit> { <digit> } "." <digit> { <digit> }

<boolean_literal> ::= "true" | "false"

<value> ::= <literal> | <identifier>

<binary_operator> ::= "+" | "-" | "*" | "/" | "%" | "^"

<unary_operator> ::= "-" | "!"

<string_content> ::= { <character> | <escape_sequence> }

<escape_sequence> ::= "\n" | "\t" | "\"" | "\\" | "\$"

<character> ::= Any character except " and \
```

## Token Types

| Token Type | Description | Example |
|------------|-------------|---------|
| KEYWORD_PROMPT | PROMPT keyword | `PROMPT` |
| KEYWORD_VAR | VAR keyword | `VAR` |
| KEYWORD_TEMPLATE | TEMPLATE keyword | `TEMPLATE` |
| KEYWORD_CONSTRAINT | CONSTRAINT keyword | `CONSTRAINT` |
| KEYWORD_OUTPUT | OUTPUT keyword | `OUTPUT` |
| KEYWORD_IF | IF keyword | `IF` |
| KEYWORD_ELSE | ELSE keyword | `ELSE` |
| KEYWORD_FOR | FOR keyword | `FOR` |
| KEYWORD_WHILE | WHILE keyword | `WHILE` |
| KEYWORD_IN | IN keyword | `IN` |
| KEYWORD_AS | AS keyword | `AS` |
| KEYWORD_AND | AND keyword | `AND` |
| KEYWORD_OR | OR keyword | `OR` |
| KEYWORD_NOT | NOT keyword | `NOT` |
| KEYWORD_RAW | RAW keyword | `RAW` |
| IDENTIFIER | Variable/function name | `myVar`, `func_name` |
| STRING_LITERAL | String value | `"Hello World"` |
| NUMBER_LITERAL | Numeric value | `42`, `3.14` |
| BOOLEAN_LITERAL | Boolean value | `true`, `false` |
| OPERATOR_EQ | Equality operator | `==` |
| OPERATOR_NE | Not equal operator | `!=` |
| OPERATOR_LT | Less than operator | `<` |
| OPERATOR_GT | Greater than operator | `>` |
| OPERATOR_LE | Less than or equal | `<=` |
| OPERATOR_GE | Greater than or equal | `>=` |
| OPERATOR_IN | In operator | `IN` |
| OPERATOR_NOT_IN | Not in operator | `NOT_IN` |
| OPERATOR_ADD | Addition operator | `+` |
| OPERATOR_SUB | Subtraction operator | `-` |
| OPERATOR_MUL | Multiplication operator | `*` |
| OPERATOR_DIV | Division operator | `/` |
| OPERATOR_MOD | Modulo operator | `%` |
| OPERATOR_POW | Power operator | `^` |
| OPERATOR_NOT | Logical NOT | `!` |
| VARIABLE_REF | Variable reference | `$varName` |
| TEMPLATE_CALL | Template call | `@templateName()` |
| LBRACE | Left brace | `{` |
| RBRACE | Right brace | `}` |
| LPAREN | Left parenthesis | `(` |
| RPAREN | Right parenthesis | `)` |
| LBRACKET | Left bracket | `[` |
| RBRACKET | Right bracket | `]` |
| COMMA | Comma | `,` |
| SEMICOLON | Semicolon | `;` |
| COLON | Colon | `:` |
| DOT | Dot | `.` |
| NEWLINE | Line break | `\n` |
| EOF | End of file | - |

## Language Features

### 1. Prompt Definitions
Define reusable prompt templates with named prompts.

```
PROMPT greeting {
    "Hello, $name! Welcome to $context."
}
```

### 2. Variable Declarations
Declare and initialize variables.

```
VAR name = "Alice";
VAR count = 42;
```

### 3. Template Definitions
Create reusable template functions.

```
TEMPLATE formatUser(name, role) {
    "User: $name (Role: $role)"
}
```

### 4. Constraint Definitions
Define validation constraints.

```
CONSTRAINT maxLength {
    length <= 1000;
}
```

### 5. Conditional Blocks
Conditional prompt generation.

```
IF isPremium {
    "You have premium access."
} ELSE {
    "You have standard access."
}
```

### 6. Loop Blocks
Iterative prompt generation.

```
FOR item IN items {
    "- $item"
}
```

### 7. Output Specifications
Define output format.

```
OUTPUT result AS JSON;
```

## Precedence and Associativity

| Precedence | Operators | Associativity |
|------------|-----------|---------------|
| 1 (highest) | `()` `[]` `.` | Left to right |
| 2 | `!` `-` (unary) | Right to left |
| 3 | `^` | Right to left |
| 4 | `*` `/` `%` | Left to right |
| 5 | `+` `-` | Left to right |
| 6 | `==` `!=` `<` `>` `<=` `>=` `IN` `NOT_IN` | Left to right |
| 7 | `NOT` | Right to left |
| 8 | `AND` | Left to right |
| 9 (lowest) | `OR` | Left to right |

## Example Program

```
VAR systemName = "PCC";
VAR maxTokens = 1000;

TEMPLATE formatSection(title, content) {
    "## $title\n$content"
}

CONSTRAINT tokenLimit {
    maxTokens <= 4096;
}

PROMPT mainPrompt {
    "You are $systemName, a prompt compiler.\n"
    @formatSection("Task", "Convert DSL to optimized prompts")
    IF debugMode {
        "DEBUG: Enabled"
    }
}

OUTPUT mainPrompt AS JSON;
```
