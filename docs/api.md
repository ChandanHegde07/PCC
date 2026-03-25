# PCC API Documentation

This document describes the public API for the Prompt Context Controller (PCC) library.

## Table of Contents

- [Data Types](#data-types)
- [Enums](#enums)
- [Structures](#structures)
- [Context Window Management](#context-window-management)
- [Message Operations](#message-operations)
- [Context Retrieval](#context-retrieval)
- [Utility Functions](#utility-functions)
- [Metrics](#metrics)
- [Thread Safety](#thread-safety)
- [Logging](#logging)
- [Version Information](#version-information)

---

## Data Types

### MessageType

```c
typedef enum {
    MESSAGE_USER = 0,      /* User input message */
    MESSAGE_ASSISTANT,     /* AI/assistant response */
    MESSAGE_SYSTEM,        /* System prompt/instructions */
    MESSAGE_TOOL           /* Tool or function call output */
} MessageType;
```

### MessagePriority

```c
typedef enum {
    PRIORITY_LOW = 0,      /* Lowest priority - evicted first */
    PRIORITY_NORMAL,       /* Normal/default priority */
    PRIORITY_HIGH,         /* High priority - retained longer */
    PRIORITY_CRITICAL      /* Highest priority - almost never evicted */
} MessagePriority;
```

### CWResult

```c
typedef enum {
    CW_SUCCESS = 0,        /* Operation succeeded */
    CW_ERROR_NULL_PTR,     /* NULL pointer provided */
    CW_ERROR_INVALID_PARAM,/* Invalid parameter value */
    CW_ERROR_NO_MEMORY,    /* Memory allocation failed */
    CW_ERROR_FULL,         /* Window is full */
    CW_ERROR_NOT_FOUND,    /* Item not found */
    CW_ERROR_IO,           /* File I/O error */
    CW_ERROR_LOCKED        /* Resource is locked */
} CWResult;
```

### CompressionStrategy

```c
typedef enum {
    COMPRESSION_NONE = 0,     /* No compression, use eviction only */
    COMPRESSION_LOW_PRIORITY, /* Remove low priority messages first */
    COMPRESSION_SUMMARIZE,     /* Summarize old messages (future) */
    COMPRESSION_AGGRESSIVE     /* Aggressive compression */
} CompressionStrategy;
```

### CWLogLevel

```c
typedef enum {
    CW_LOG_ERROR = 0,    /* Error messages */
    CW_LOG_WARN,         /* Warning messages */
    CW_LOG_INFO,         /* Informational messages */
    CW_LOG_DEBUG,        /* Debug messages */
    CW_LOG_TRACE         /* Trace messages (most verbose) */
} CWLogLevel;
```

---

## Structures

### ContextConfig

```c
typedef struct ContextConfig {
    int max_tokens;                   /* Maximum tokens allowed */
    int min_tokens_reserve;           /* Minimum tokens to reserve */
    CompressionStrategy compression;  /* Compression strategy to use */
    bool enable_metrics;               /* Enable metrics collection */
    bool thread_safe;                  /* Enable thread safety */
    int token_ratio;                   /* Characters per token estimate */
    bool auto_compress;                /* Enable automatic compression */
} ContextConfig;
```

### ContextMetrics

```c
typedef struct ContextMetrics {
    uint64_t messages_added;      /* Total messages added */
    uint64_t messages_evicted;    /* Total messages evicted */
    uint64_t tokens_added;        /* Total tokens added */
    uint64_t tokens_evicted;      /* Total tokens evicted */
    uint64_t compressions;        /* Number of compressions performed */
    uint64_t context_retrievals;  /* Number of context retrievals */
    double peak_utilization;      /* Peak token utilization percentage */
    clock_t total_time;           /* Total time window has been active */
} ContextMetrics;
```

### Message

```c
typedef struct Message {
    MessageType type;              /* Type of message */
    MessagePriority priority;      /* Priority level */
    char* content;                /* Message content */
    int token_count;              /* Token count for this message */
    struct Message* next;         /* Next message in list */
    struct Message* prev;         /* Previous message in list */
} Message;
```

### ContextWindow

```c
typedef struct ContextWindow {
    Message* head;                 /* Head of message list */
    Message* tail;                 /* Tail of message list */
    int total_tokens;              /* Current total tokens */
    int max_tokens;               /* Maximum allowed tokens */
    int message_count;            /* Number of messages in window */
    ContextConfig config;         /* Window configuration */
    ContextMetrics* metrics;      /* Performance metrics */
    ContextMutex* mutex;          /* Mutex for thread safety */
} ContextWindow;
```

---

## Context Window Management

### context_window_create

Creates a new context window with default configuration.

```c
ContextWindow* context_window_create(int max_tokens);
```

**Parameters:**
- `max_tokens` - Maximum number of tokens the window can hold

**Returns:** Pointer to new ContextWindow, or NULL on failure

**Example:**
```c
ContextWindow* window = context_window_create(4096);
if (window == NULL) {
    fprintf(stderr, "Failed to create context window\n");
    return;
}
```

---

### context_window_create_with_config

Creates a new context window with custom configuration.

```c
ContextWindow* context_window_create_with_config(const ContextConfig* config);
```

**Parameters:**
- `config` - Pointer to ContextConfig with desired settings

**Returns:** Pointer to new ContextWindow, or NULL on failure

**Example:**
```c
ContextConfig config = context_config_default();
config.max_tokens = 8192;
config.thread_safe = true;
config.enable_metrics = true;

ContextWindow* window = context_window_create_with_config(&config);
```

---

### context_window_destroy

Destroys a context window and frees all associated memory.

```c
void context_window_destroy(ContextWindow* window);
```

**Parameters:**
- `window` - Pointer to the ContextWindow to destroy

**Example:**
```c
context_window_destroy(window);
window = NULL;
```

---

### context_config_default

Returns a default context configuration.

```c
ContextConfig context_config_default(void);
```

**Returns:** ContextConfig with default values

**Example:**
```c
ContextConfig config = context_config_default();
config.max_tokens = 4096;
```

---

### context_config_validate

Validates a context configuration.

```c
bool context_config_validate(const ContextConfig* config);
```

**Parameters:**
- `config` - Pointer to ContextConfig to validate

**Returns:** true if valid, false otherwise

---

### context_window_apply_config

Applies a new configuration to an existing window.

```c
CWResult context_window_apply_config(ContextWindow* window, const ContextConfig* config);
```

**Parameters:**
- `window` - Pointer to the ContextWindow
- `config` - Pointer to new configuration

**Returns:** CWResult status code

---

## Message Operations

### context_window_add_message

Adds a message to the context window.

```c
bool context_window_add_message(ContextWindow* window,
                                MessageType type,
                                MessagePriority priority,
                                const char* content);
```

**Parameters:**
- `window` - Pointer to the ContextWindow
- `type` - Type of message (MESSAGE_USER, MESSAGE_ASSISTANT, etc.)
- `priority` - Priority level (PRIORITY_LOW, PRIORITY_NORMAL, etc.)
- `content` - Message content string

**Returns:** true on success, false on failure

**Example:**
```c
bool success = context_window_add_message(window, MESSAGE_USER, 
                                          PRIORITY_NORMAL, "Hello, AI!");
if (!success) {
    fprintf(stderr, "Failed to add message\n");
}
```

---

### context_window_add_message_ex

Adds a message with extended error reporting.

```c
bool context_window_add_message_ex(ContextWindow* window,
                                    MessageType type,
                                    MessagePriority priority,
                                    const char* content,
                                    CWResult* result);
```

**Parameters:**
- `window` - Pointer to the ContextWindow
- `type` - Type of message
- `priority` - Priority level
- `content` - Message content string
- `result` - Pointer to store detailed result code (can be NULL)

**Returns:** true on success, false on failure

**Example:**
```c
CWResult result;
bool success = context_window_add_message_ex(window, MESSAGE_ASSISTANT,
                                              PRIORITY_HIGH, "Response text",
                                              &result);
if (!success) {
    printf("Error: %s\n", context_window_result_to_string(result));
}
```

---

### context_window_remove_message

Removes a message by content.

```c
bool context_window_remove_message(ContextWindow* window, const char* content);
```

**Parameters:**
- `window` - Pointer to the ContextWindow
- `content` - Message content to remove

**Returns:** true if found and removed, false otherwise

---

### context_window_clear

Clears all messages from the context window.

```c
void context_window_clear(ContextWindow* window);
```

**Parameters:**
- `window` - Pointer to the ContextWindow

**Example:**
```c
context_window_clear(window);
printf("Window cleared. Messages: %d\n", context_window_get_message_count(window));
```

---

## Context Retrieval

### context_window_get_context

Retrieves all messages as a concatenated string.

```c
char* context_window_get_context(ContextWindow* window);
```

**Parameters:**
- `window` - Pointer to the ContextWindow

**Returns:** Concatenated string of all messages (caller must free), or NULL on error

**Example:**
```c
char* context = context_window_get_context(window);
if (context) {
    printf("Context:\n%s\n", context);
    free(context);
}
```

---

### context_window_get_context_json

Retrieves all messages as a JSON formatted string.

```c
char* context_window_get_context_json(ContextWindow* window);
```

**Parameters:**
- `window` - Pointer to the ContextWindow

**Returns:** JSON string (caller must free), or NULL on error

**Example:**
```c
char* json = context_window_get_context_json(window);
if (json) {
    printf("JSON Context:\n%s\n", json);
    free(json);
}
```

---

### context_window_get_messages_by_type

Retrieves all messages of a specific type.

```c
int context_window_get_messages_by_type(const ContextWindow* window, MessageType type, char*** messages);
```

**Parameters:**
- `window` - Pointer to the ContextWindow
- `type` - MessageType to filter by
- `messages` - Pointer to store array of message strings

**Returns:** Number of messages found (0 if none or error)

**Example:**
```c
char** user_messages = NULL;
int count = context_window_get_messages_by_type(window, MESSAGE_USER, &user_messages);
for (int i = 0; i < count; i++) {
    printf("User message %d: %s\n", i + 1, user_messages[i]);
}
context_window_free_message_array(user_messages, count);
```

---

### context_window_get_messages_by_priority

Retrieves all messages of a specific priority.

```c
int context_window_get_messages_by_priority(const ContextWindow* window, MessagePriority priority, char*** messages);
```

**Parameters:**
- `window` - Pointer to the ContextWindow
- `priority` - MessagePriority to filter by
- `messages` - Pointer to store array of message strings

**Returns:** Number of messages found

---

### context_window_free_message_array

Frees a message array allocated by get messages functions.

```c
void context_window_free_message_array(char** messages, int count);
```

**Parameters:**
- `messages` - Array of message strings
- `count` - Number of messages in array

---

## Utility Functions

### calculate_token_count

Calculates token count using default ratio (4 chars per token).

```c
int calculate_token_count(const char* text);
```

**Parameters:**
- `text` - Text to count tokens for

**Returns:** Estimated token count

**Example:**
```c
int tokens = calculate_token_count("Hello world");
printf("Estimated tokens: %d\n", tokens);
```

---

### calculate_token_count_ex

Calculates token count with custom ratio.

```c
int calculate_token_count_ex(const char* text, int ratio);
```

**Parameters:**
- `text` - Text to count tokens for
- `ratio` - Characters per token

**Returns:** Estimated token count

---

### context_window_get_utilization

Gets current token utilization percentage.

```c
double context_window_get_utilization(ContextWindow* window);
```

**Parameters:**
- `window` - Pointer to the ContextWindow

**Returns:** Utilization as percentage (0.0 to 100.0)

**Example:**
```c
double util = context_window_get_utilization(window);
printf("Utilization: %.1f%%\n", util);
```

---

### context_window_get_message_count

Gets the number of messages in the window.

```c
int context_window_get_message_count(const ContextWindow* window);
```

**Parameters:**
- `window` - Pointer to the ContextWindow

**Returns:** Message count

---

### context_window_get_token_count

Gets the current total token count.

```c
int context_window_get_token_count(const ContextWindow* window);
```

**Parameters:**
- `window` - Pointer to the ContextWindow

**Returns:** Total token count

---

### context_window_get_max_tokens

Gets the maximum token limit.

```c
int context_window_get_max_tokens(const ContextWindow* window);
```

**Parameters:**
- `window` - Pointer to the ContextWindow

**Returns:** Maximum token limit

---

### context_window_get_remaining_capacity

Gets remaining token capacity.

```c
int context_window_get_remaining_capacity(const ContextWindow* window);
```

**Parameters:**
- `window` - Pointer to the ContextWindow

**Returns:** Remaining token capacity

---

### context_window_is_empty

Checks if window is empty.

```c
bool context_window_is_empty(const ContextWindow* window);
```

**Parameters:**
- `window` - Pointer to the ContextWindow

**Returns:** true if empty, false otherwise

---

### context_window_is_full

Checks if window is full.

```c
bool context_window_is_full(const ContextWindow* window);
```

**Parameters:**
- `window` - Pointer to the ContextWindow

**Returns:** true if full, false otherwise

---

### context_window_print_stats

Prints window statistics to stdout.

```c
void context_window_print_stats(ContextWindow* window);
```

**Parameters:**
- `window` - Pointer to the ContextWindow

---

## Metrics

### context_window_get_metrics

Gets current metrics.

```c
const ContextMetrics* context_window_get_metrics(const ContextWindow* window);
```

**Parameters:**
- `window` - Pointer to the ContextWindow

**Returns:** Pointer to metrics structure (or NULL if disabled)

---

### context_window_print_metrics

Prints metrics to stdout.

```c
void context_window_print_metrics(ContextWindow* window);
```

**Parameters:**
- `window` - Pointer to the ContextWindow

---

### context_window_reset_metrics

Resets all metrics to zero.

```c
void context_window_reset_metrics(ContextWindow* window);
```

**Parameters:**
- `window` - Pointer to the ContextWindow

---

### context_window_set_metrics_enabled

Enables or disables metrics collection.

```c
void context_window_set_metrics_enabled(ContextWindow* window, bool enable);
```

**Parameters:**
- `window` - Pointer to the ContextWindow
- `enable` - true to enable, false to disable

---

## Thread Safety

### context_window_lock

Locks the window for thread-safe access.

```c
CWResult context_window_lock(ContextWindow* window);
```

**Parameters:**
- `window` - Pointer to the ContextWindow

**Returns:** CW_SUCCESS on success, CW_ERROR_LOCKED if already locked

---

### context_window_unlock

Unlocks the window.

```c
CWResult context_window_unlock(ContextWindow* window);
```

**Parameters:**
- `window` - Pointer to the ContextWindow

**Returns:** CW_SUCCESS on success

---

### context_window_is_thread_safe

Checks if thread safety is enabled.

```c
bool context_window_is_thread_safe(const ContextWindow* window);
```

**Parameters:**
- `window` - Pointer to the ContextWindow

**Returns:** true if thread safe, false otherwise

---

## Logging

### context_window_log

Logs a message at the specified level.

```c
void context_window_log(CWLogLevel level, const char* format, ...);
```

**Parameters:**
- `level` - Log level (CW_LOG_ERROR, CW_LOG_WARN, etc.)
- `format` - Printf-style format string

**Example:**
```c
context_window_log(CW_LOG_INFO, "Processing request: %s", request_id);
context_window_log(CW_LOG_ERROR, "Failed to connect: %s", error_msg);
```

---

### context_window_log_set_level

Sets the minimum log level.

```c
void context_window_log_set_level(CWLogLevel level);
```

**Parameters:**
- `level` - Minimum level to log

**Example:**
```c
context_window_log_set_level(CW_LOG_DEBUG);  // Enable debug logging
```

---

### context_window_log_set_callback

Sets a custom log callback function.

```c
void context_window_log_set_callback(CWLogCallback callback);
```

**Parameters:**
- `callback` - Function pointer to handle logs

**Example:**
```c
void my_log_handler(CWLogLevel level, const char* message) {
    fprintf(logfile, "[%d] %s\n", level, message);
}

context_window_log_set_callback(my_log_handler);
```

---

### context_window_result_to_string

Converts a CWResult to a string.

```c
const char* context_window_result_to_string(CWResult result);
```

**Parameters:**
- `result` - CWResult code

**Returns:** String description

**Example:**
```c
CWResult res = context_window_add_message(window, MESSAGE_USER, 
                                           PRIORITY_NORMAL, "test");
printf("Result: %s\n", context_window_result_to_string(res));
```

---

## Persistence

### context_window_save

Saves context window to a file.

```c
CWResult context_window_save(const ContextWindow* window, const char* filename);
```

**Parameters:**
- `window` - Pointer to the ContextWindow
- `filename` - Output file path

**Returns:** CWResult status

---

### context_window_load

Loads context window from a file.

```c
ContextWindow* context_window_load(const char* filename);
```

**Parameters:**
- `filename` - Input file path

**Returns:** Loaded ContextWindow, or NULL on failure

---

### context_window_export_json

Exports context window to JSON file.

```c
CWResult context_window_export_json(const ContextWindow* window, const char* filename);
```

**Parameters:**
- `window` - Pointer to the ContextWindow
- `filename` - Output JSON file path

**Returns:** CWResult status

---

## Version Information

### context_window_version

Returns the version string.

```c
const char* context_window_version(void);
```

**Returns:** Version string (e.g., "1.0.0")

**Example:**
```c
printf("PCC version: %s\n", context_window_version());
```

---

### context_window_version_major

Returns the major version number.

```c
int context_window_version_major(void);
```

**Returns:** Major version

---

### context_window_version_minor

Returns the minor version number.

```c
int context_window_version_minor(void);
```

**Returns:** Minor version

---

### context_window_version_patch

Returns the patch version number.

```c
int context_window_version_patch(void);
```

**Returns:** Patch version

---

## Quick Start Example

```c
#include "context_window.h"
#include <stdio.h>
#include <stdlib.h>

int main(void) {
    ContextWindow* window = context_window_create(4096);
    if (!window) return 1;

    context_window_add_message(window, MESSAGE_SYSTEM, PRIORITY_CRITICAL, 
                                "You are a helpful assistant.");
    context_window_add_message(window, MESSAGE_USER, PRIORITY_NORMAL, 
                                "What is the capital of France?");
    context_window_add_message(window, MESSAGE_ASSISTANT, PRIORITY_NORMAL, 
                                "The capital of France is Paris.");

    printf("Messages: %d\n", context_window_get_message_count(window));
    printf("Tokens: %d/%d\n", context_window_get_token_count(window),
           context_window_get_max_tokens(window));
    printf("Utilization: %.1f%%\n", context_window_get_utilization(window));

    char* context = context_window_get_context(window);
    if (context) {
        printf("\nContext:\n%s\n", context);
        free(context);
    }

    context_window_destroy(window);
    return 0;
}
```
