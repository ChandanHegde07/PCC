# PCC — Prompt Context Controller

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Language: C](https://img.shields.io/badge/Language-C-blue.svg)](https://en.wikipedia.org/wiki/C_(programming_language))
[![Platform: POSIX](https://img.shields.io/badge/Platform-POSIX-green.svg)](https://en.wikipedia.org/wiki/POSIX)
[![Build](https://github.com/ChandanHegde07/PCC/actions/workflows/ci.yml/badge.svg)](https://github.com/ChandanHegde07/PCC/actions)
[![Tests](https://img.shields.io/badge/Tests-23%20passed-green.svg)](https://github.com/ChandanHegde07/PCC/actions)

A lightweight, efficient C library for managing conversation history within the context window constraints of Small Language Models (SLMs).

---

## Table of Contents

- [Overview](#overview)
- [Quick Start](#quick-start)
- [Installation](#installation)
- [Usage](#usage)
- [API Reference](#api-reference)
- [Configuration](#configuration)
- [Save & Load](#save--load)
- [Performance](#performance)
- [Testing](#testing)
- [CI/CD](#cicd)
- [Project Structure](#project-structure)
- [Contributing](#contributing)
- [License](#license)

---

## Overview

PCC implements intelligent context window management using proven data structures and algorithms — ensuring optimal token utilization while preserving the most important conversation context.

**Why PCC?**

- Pure C with minimal dependencies — small binary, fast startup
- O(1) amortized message insertion with efficient memory management
- Priority-based retention keeps system prompts and critical messages safe
- Approximate token counting without heavy NLP libraries
- Simple API that drops into existing projects with minimal friction
- 23+ test cases, CI/CD pipeline, and Doxygen-generated docs

---

## Quick Start

```bash
git clone https://github.com/ChandanHegde07/PCC.git
cd PCC
make all
make run
```

Expected output:

```
==============================================
  PCC - Prompt Context Controller
  Version 1.0.0
==============================================

Context Window Statistics:
  Total messages:  6
  Total tokens:    156 / 1000 (15.6% full)
  Tokens remaining: 844
```

---

## Installation

**Prerequisites:** GCC 4.0+ (or any C99-compliant compiler), Make, and a POSIX environment (Linux, macOS, or WSL).

```bash
make all          # Build main app and tests
make run          # Run the demo application
make test         # Run the full test suite
make memtest      # Check for memory leaks with Valgrind
make asan         # Build and run with AddressSanitizer
make benchmark    # Run performance benchmarks
make clean        # Remove build artifacts
make docs         # Generate Doxygen API documentation
```

---

## Usage

### Basic Example

```c
#include <stdio.h>
#include "context_window.h"

int main(void) {
    // Create a context window with a 1000-token limit
    ContextWindow* window = context_window_create(1000);

    // System prompt — PRIORITY_CRITICAL means it is never evicted
    context_window_add_message(window, MESSAGE_SYSTEM, PRIORITY_CRITICAL,
        "You are a helpful AI assistant.");

    // Conversation turns
    context_window_add_message(window, MESSAGE_USER, PRIORITY_HIGH,
        "What is the capital of France?");

    context_window_add_message(window, MESSAGE_ASSISTANT, PRIORITY_NORMAL,
        "The capital of France is Paris.");

    // Get a formatted context string ready for your SLM API
    char* context = context_window_get_context(window);
    printf("%s\n", context);

    context_window_print_stats(window);

    free(context);
    context_window_destroy(window);
    return 0;
}
```

### SLM Integration Pattern

```c
void process_user_query(ContextWindow* window, const char* user_input) {
    // 1. Add the user turn
    context_window_add_message(window, MESSAGE_USER, PRIORITY_HIGH, user_input);

    // 2. Retrieve the optimized context
    char* context = context_window_get_context(window);

    // 3. Call your SLM API
    char* response = call_slm_api(context);

    // 4. Store the assistant turn
    context_window_add_message(window, MESSAGE_ASSISTANT, PRIORITY_NORMAL, response);

    free(context);
    free(response);
}
```

---

## API Reference

### Context Window

| Function | Description | Returns |
|----------|-------------|---------|
| `context_window_create(int max_tokens)` | Create a new context window | `ContextWindow*` |
| `context_window_create_with_config(const ContextConfig*)` | Create with custom config | `ContextWindow*` |
| `context_window_destroy(ContextWindow*)` | Free all resources | `void` |
| `context_window_add_message(...)` | Add a message | `bool` |
| `context_window_add_message_ex(...)` | Add a message with result code | `bool` |
| `context_window_remove_message(...)` | Remove a specific message | `bool` |
| `context_window_clear(ContextWindow*)` | Clear all messages | `void` |
| `context_window_get_context(ContextWindow*)` | Get formatted context string | `char*` |
| `context_window_get_context_json(ContextWindow*)` | Get context as JSON | `char*` |
| `context_window_get_message_count(...)` | Number of messages in window | `int` |
| `context_window_get_token_count(...)` | Current token count | `int` |
| `context_window_get_utilization(...)` | Utilization as a percentage | `double` |
| `context_window_print_stats(ContextWindow*)` | Print window statistics | `void` |
| `context_window_print_metrics(ContextWindow*)` | Print detailed metrics | `void` |
| `calculate_token_count(const char*)` | Estimate tokens in a string | `int` |

### Save & Load

| Function | Description | Returns |
|----------|-------------|---------|
| `context_window_save(...)` | Save to text file | `CWResult` |
| `context_window_load(...)` | Load from text file | `ContextWindow*` |
| `context_window_export_json(...)` | Export to JSON file | `CWResult` |

### Configuration

| Function | Description | Returns |
|----------|-------------|---------|
| `context_config_default()` | Get default configuration | `ContextConfig` |
| `context_config_validate(...)` | Validate a config struct | `bool` |
| `context_window_apply_config(...)` | Apply a new config at runtime | `CWResult` |

### Enumerations

```c
// Message role
typedef enum {
    MESSAGE_USER,       // User input
    MESSAGE_ASSISTANT,  // AI response
    MESSAGE_SYSTEM,     // System prompt
    MESSAGE_TOOL        // Tool / function response
} MessageType;

// Eviction priority — lower priority is evicted first
typedef enum {
    PRIORITY_LOW,       // Removed first under pressure
    PRIORITY_NORMAL,    // Default
    PRIORITY_HIGH,      // User questions
    PRIORITY_CRITICAL   // System prompts — never removed
} MessagePriority;

// Compression behavior
typedef enum {
    COMPRESSION_NONE,           // No compression
    COMPRESSION_LOW_PRIORITY,   // Remove low-priority messages first
    COMPRESSION_AGGRESSIVE      // Aggressive compression
} CompressionStrategy;
```

---

## Configuration

```c
#include "context_window.h"

int main(void) {
    ContextConfig config = context_config_default();

    config.max_tokens    = 2000;
    config.compression   = COMPRESSION_LOW_PRIORITY;
    config.enable_metrics = true;
    config.auto_compress = true;

    ContextWindow* window = context_window_create_with_config(&config);

    // ...

    context_window_destroy(window);
    return 0;
}
```

---

## Save & Load

```c
// Persist a conversation to disk
context_window_save(window, "conversation.txt");
context_window_export_json(window, "conversation.json");

// Restore from a saved file
ContextWindow* window = context_window_load("conversation.txt");
// ...
context_window_destroy(window);
```

---

## Performance

### Time Complexity

| Operation | Complexity | Notes |
|-----------|------------|-------|
| Add message | O(1) amortized | O(n) worst case during compression |
| Remove message | O(1) | Direct pointer manipulation |
| Get context | O(n) | Full message traversal |
| Token count | O(1) | Maintained incrementally |

### Memory Usage

| Messages | Approximate Memory |
|----------|--------------------|
| 100 | ~50 KB |
| 1,000 | ~500 KB |
| 10,000 | ~5 MB |

Per-message overhead is ~100–200 bytes plus content length.

---

## Testing

```bash
make test       # Run the full test suite (23+ cases)
make memtest    # Valgrind memory leak check
make asan       # AddressSanitizer build
```

Test coverage includes basic operations, NULL and boundary inputs, priority-based eviction, token calculation, memory stress, save/load roundtrips, and configuration validation.

---

## CI/CD

GitHub Actions runs on every push and pull request:

- **Build** — compiled with `-Wall -Wextra -Wpedantic -Werror`
- **Test** — full test suite with detailed pass/fail reporting
- **Benchmark** — performance metrics on each run
- **Static analysis** — code quality checks via `cppcheck`
- **Artifacts** — compiled binaries uploaded for versioning and download

---

## Project Structure

```
PCC/
├── .github/workflows/ci.yml       # GitHub Actions CI/CD
├── src/
│   ├── context_window.c           # Core implementation
│   └── main.c                     # Demo application
├── include/
│   └── context_window.h           # Public API header
├── tests/
│   ├── test_window_manager.c      # Test suite
│   └── benchmark.c                # Performance benchmarks
├── examples/
│   ├── basic_usage.c
│   ├── config_example.c
│   ├── save_load_example.c
│   └── sample_conversation.txt
├── docs/
│   ├── architecture.md            # System architecture
│   └── design.md                  # Design decisions
├── Doxyfile                       # Doxygen configuration
├── Makefile
└── README.md
```

---

## Contributing

1. Fork the repository
2. Create a feature branch: `git checkout -b feature/your-feature`
3. Make your changes — add tests for any new functionality
4. Ensure everything passes: `make test`
5. Open a Pull Request with a clear description of what changed and why

Please keep to the existing code style and build with zero warnings.
