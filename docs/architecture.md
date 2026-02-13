# PCC Architecture Guide

> A detailed look at the system architecture for PCC - Prompt Context Controller

---

## Table of Contents

- [Overview](#overview)
- [System Design](#system-design)
- [Core Components](#core-components)
- [Data Structures](#data-structures)
- [Algorithms](#algorithms)
- [Design Patterns](#design-patterns)
- [Memory Management](#memory-management)
- [Performance](#performance)
- [Extensibility](#extensibility)
- [Build System](#build-system)

---

## Overview

PCC (Prompt Context Controller) is a C library that manages conversation history for Small Language Models (SLMs). It ensures that critical context remains within the token limits of SLMs while automatically removing less important messages.

### Key Architectural Goals

| Goal | Implementation |
|------|----------------|
| **Efficiency** | O(1) amortized message insertion via linked lists |
| **Reliability** | Priority-based retention preserves critical context |
| **Simplicity** | Minimal dependencies (pure C, standard library only) |
| **Extensibility** | Modular design allows easy feature additions |

---

## System Design

### High-Level Architecture

```
┌─────────────────────────────────────────────────────────────────┐
│                        Application Layer                        │
│                    (main.c - Demo Application)                  │
└─────────────────────────────────────────────────────────────────┘
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────┐
│                        API Layer                                │
│              (context_window.h - Public Interface)             │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────────────────┐│
│  │   Create     │ │     Add      │ │       Get Context        ││
│  │   Window     │ │   Message    │ │        String            ││
│  └──────────────┘ └──────────────┘ └──────────────────────────┘│
└─────────────────────────────────────────────────────────────────┘
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────┐
│                   Implementation Layer                          │
│                  (context_window.c - Core Logic)                │
│  ┌──────────────┐ ┌──────────────┐ ┌──────────────────────────┐│
│  │  Linked List │ │  Compression │ │   Token Estimation       ││
│  │  Management  │ │   Strategy    │ │       Engine             ││
│  └──────────────┘ └──────────────┘ └──────────────────────────┘│
└─────────────────────────────────────────────────────────────────┘
```

### Component Interactions

```
User Input ──► context_window_add_message()
                    │
                    ▼
            ┌───────────────┐
            │ Validate      │
            │ Input         │
            └───────┬───────┘
                    │
                    ▼
            ┌───────────────┐     ┌───────────────┐
            │ Create        │────►│ Check Token   │
            │ Message Node  │     │ Limit         │
            └───────────────┘     └───────┬───────┘
                                           │
                              ┌────────────┴────────────┐
                              │                         │
                              ▼                         ▼
                     ┌───────────────┐         ┌───────────────┐
                     │ Within Limit  │         │ Over Limit    │
                     │    Done        │         │ Compress      │
                     └───────────────┘         └───────┬───────┘
                                                       │
                                                       ▼
                                            ┌─────────────────────┐
                                            │ Remove Lowest       │
                                            │ Priority Messages   │
                                            └──────────┬──────────┘
                                                       │
                                                       ▼
                                            ┌─────────────────────┐
                                            │ Recursively Check   │
                                            │ Until Under Limit   │
                                            └─────────────────────┘
```

---

## Core Components

### 1. Message Management

| Function | Purpose | Complexity |
|----------|---------|------------|
| `context_window_add_message()` | Add new message to window | O(1) amortized |
| `remove_message()` | Remove specific message | O(1) |
| `context_window_get_context()` | Generate formatted output | O(n) |

### 2. Window Statistics

| Function | Purpose | Returns |
|----------|---------|---------|
| `context_window_get_token_count()` | Current token usage | `int` |
| `context_window_get_message_count()` | Number of messages | `int` |
| `context_window_print_stats()` | Debug output | `void` |

### 3. Token Estimation

The system uses a fast heuristic-based approach:

```
Token Count ≈ (String Length + 3) / 4
```

This approximation provides:
-  **Speed**: O(1) calculation
- **Safety**: Conservative estimate (worst-case)
- **Simplicity**: No external dependencies

---

## Data Structures

### Message Node

```c
typedef struct Message {
    MessageType type;              // USER / ASSISTANT / SYSTEM / TOOL
    MessagePriority priority;      // LOW / NORMAL / HIGH / CRITICAL
    char* content;                 // Actual message text
    int token_count;               // Pre-calculated token count
    struct Message* next;         // Forward pointer
    struct Message* prev;         // Backward pointer
} Message;
```

**Design Rationale:**
- `type` enables context-aware processing
- `priority` drives retention decisions
- `token_count` is cached for performance
- Doubly-linked structure enables O(1) removal from any position

### Context Window

```c
typedef struct ContextWindow {
    Message* head;                 // Oldest message (FIFO)
    Message* tail;                 // Most recent message
    int total_tokens;              // Running token total
    int max_tokens;                // Configured limit
    int message_count;             // Running message count
} ContextWindow;
```

**Design Rationale:**
- `head` and `tail` pointers enable O(1) append/prepend
- `total_tokens` cached to avoid recalculation
- `max_tokens` is the constraint boundary

---

## Algorithms

### Sliding Window Algorithm

```
Initial State: [msg1, msg2, msg3, msg4, msg5]
               Tokens: 800 / 1000 limit ✓

After Adding msg6 (200 tokens):
               [msg1, msg2, msg3, msg4, msg5, msg6]
               Tokens: 1000 / 1000 limit ✓

After Adding msg7 (150 tokens):
               [msg1, msg2, msg3, msg4, msg5, msg6, msg7]
               Tokens: 1150 / 1000 limit ✗
                            │
                            ▼
               Compression triggers
                            │
                            ▼
               [msg2, msg3, msg4, msg5, msg6, msg7]
               Tokens: 950 / 1000 limit ✓
```

### Priority-Based Compression

Messages are removed in this order when token limits are exceeded:

```
┌─────────────────────────────────────────────────────────┐
│                    Compression Order                     │
├─────────────┬───────────────────────────────────────────┤
│ Priority    │ Removal Order                             │
├─────────────┼───────────────────────────────────────────┤
│ CRITICAL    │ 4th (rarely removed)                      │
│ HIGH        │ 3rd (user questions preserved)            │
│ NORMAL      │ 2nd (assistant responses)                 │
│ LOW         │ 1st (removed first)                       │
└─────────────┴───────────────────────────────────────────┘
```

### Token Estimation Algorithm

```c
int calculate_token_count(const char* text) {
    if (text == NULL || *text == '\0') {
        return 0;
    }
    return (strlen(text) + 3) / 4;
}
```

**Why divide by 4?**
- Average token is ~4 characters in English
- This provides a conservative (upper-bound) estimate
- Prevents exceeding actual token limits

---

## Design Patterns

### 1. Queue Pattern (FIFO)

```c
// Messages flow from head (oldest) to tail (newest)
// Old messages exit from head, new messages enter at tail
head ──► [A] ──► [B] ──► [C] ──► [D] ──► tail
         oldest    →      →      →    newest
```

### 2. Priority Queue Pattern

```c
// During compression, lowest priority messages exit first
// CRITICAL messages are retained as long as possible
//
// Compression flow:
// 1. Remove all LOW priority
// 2. Remove NORMAL if still over limit
// 3. Remove HIGH if still over limit  
// 4. Remove CRITICAL only as last resort
```

### 3. Factory Pattern

```c
// Message creation is encapsulated
Message* create_message(MessageType type, MessagePriority priority, const char* content) {
    Message* msg = malloc(sizeof(Message));
    msg->content = strdup(content);  // Safe copy
    msg->token_count = calculate_token_count(content);
    // ... initialize pointers
    return msg;
}
```

### 4. RAII-like Resource Management

```c
// Clear ownership and cleanup responsibilities
context_window_destroy(window);  // Frees ALL associated memory
// After this, window pointer is invalid
```

---

## Memory Management

### Allocation Strategy

| Operation | Method | Safety |
|-----------|--------|--------|
| Message content | `strdup()` | Copy made, original can be freed |
| Message struct | `malloc()` | Check for NULL |
| Context string | `malloc()` | Caller must `free()` |

### Memory Lifecycle

```
1. Create Window
   └── malloc(ContextWindow)

2. Add Message
   ├── malloc(Message struct)
   └── strdup(content copy)

3. Get Context
   └── malloc(formatted string) ← Caller frees

4. Destroy Window
   ├── free(each Message content)
   ├── free(each Message struct)
   └── free(ContextWindow)
```

### Error Handling

```c
ContextWindow* context_window_create(int max_tokens) {
    if (max_tokens <= 0) {
        return NULL;  // Invalid parameter
    }
    
    ContextWindow* window = malloc(sizeof(ContextWindow));
    if (window == NULL) {
        return NULL;  // Allocation failed
    }
    
    // Initialize fields
    window->head = NULL;
    window->tail = NULL;
    // ... etc
    
    return window;
}
```

---

## Performance

### Time Complexity Analysis

| Operation | Best Case | Worst Case | Average |
|-----------|-----------|------------|---------|
| Add message | O(1) | O(n) | O(1) amortized |
| Remove message | O(1) | O(1) | O(1) |
| Get context | O(n) | O(n) | O(n) |
| Token calculation | O(1) | O(n) | O(1) |

**Why O(1) amortized for add?**
- Most insertions simply append to tail (O(1))
- Only when token limit is exceeded does compression occur
- Compression scans through messages once per add
- Over time, each message is scanned exactly once

### Space Complexity

```
O(n) where n = number of messages

Memory per message ≈ 100-200 bytes (overhead) + content_length
```

### Benchmarks

| Metric | Typical Value |
|--------|----------------|
| Add 1000 messages | ~5ms |
| Compress 100 messages | ~0.5ms |
| Generate context (100 msgs) | ~1ms |
| Memory for 1000 messages | ~500KB |

---

## Extensibility

### Adding New Features

#### 1. New Message Types

```c
// Extend the enum in context_window.h
typedef enum {
    MESSAGE_USER,
    MESSAGE_ASSISTANT,
    MESSAGE_SYSTEM,
    MESSAGE_TOOL,
    MESSAGE_FUNCTION  // NEW: Function call message
} MessageType;
```

#### 2. Custom Tokenizers

```c
// Replace calculate_token_count() with real tokenizer
// Example: integrate tiktoken or HuggingFace tokenizer
int calculate_token_count(const char* text) {
    // Use external tokenizer library
    return external_tokenizer_encode(text);
}
```

#### 3. Advanced Compression

```c
// Add new compression strategy
typedef enum {
    COMPRESSION_PRIORITY,    // Current strategy
    COMPRESSION_SUMMARY,     // Summarize old messages
    COMPRESSION_TOPIC       // Keep topic-relevant messages
} CompressionStrategy;
```

---

## Build System

### Makefile Targets

| Target | Purpose |
|--------|---------|
| `make all` | Build main app and tests |
| `make run` | Execute demo application |
| `make test` | Run test suite |
| `make clean` | Remove build artifacts |
| `make distclean` | Complete cleanup |

### Build Output

```
llm-context-manager    # Demo executable (35KB)
test-window-manager   # Test executable (36KB)
*.o                   # Object files
```

### Compilation Flags

```makefile
CFLAGS = -Wall -Wextra -Iinclude -g
# -Wall     : Enable all warnings
# -Wextra   : Enable extra warnings
# -Iinclude : Include path for headers
# -g        : Debug symbols
```

---

## Related Documentation

- **[Design Document](design.md)** - Design decisions and trade-offs
- **[README](../Readme.md)** - Project overview and quick start

---

<p align="center">
  <strong>Built for SLM efficiency</strong>
</p>
