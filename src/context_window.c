// Context window manager for maintaining message history within token limits using a linked list.

#include "context_window.h"

#define TOKEN_ESTIMATION_RATIO 4

int calculate_token_count(const char* text) {
    if (text == NULL) return 0;
    
    int length = strlen(text);
    return (length + TOKEN_ESTIMATION_RATIO - 1) / TOKEN_ESTIMATION_RATIO;
}

ContextWindow* context_window_create(int max_tokens) {
    ContextWindow* window = (ContextWindow*)malloc(sizeof(ContextWindow));
    if (window == NULL) {
        printf("Failed to allocate memory for context window\n");
        return NULL;
    }
    
    window->head = NULL;
    window->tail = NULL;
    window->total_tokens = 0;
    window->max_tokens = max_tokens;
    window->message_count = 0;
    
    return window;
}

void context_window_destroy(ContextWindow* window) {
    if (window == NULL) return;
    
    Message* current = window->head;
    while (current != NULL) {
        Message* next = current->next;
        free(current->content);
        free(current);
        current = next;
    }
    
    free(window);
}

static Message* create_message(MessageType type, MessagePriority priority, const char* content) {
    Message* msg = (Message*)malloc(sizeof(Message));
    if (msg == NULL) {
        printf("Failed to allocate memory for message\n");
        return NULL;
    }
    
    msg->type = type;
    msg->priority = priority;
    msg->content = strdup(content);
    msg->token_count = calculate_token_count(content);
    msg->next = NULL;
    msg->prev = NULL;
    
    return msg;
}

static void remove_message(ContextWindow* window, Message* msg) {
    if (window == NULL || msg == NULL) return;
    
    window->total_tokens -= msg->token_count;
    window->message_count--;
    
    if (msg->prev != NULL) {
        msg->prev->next = msg->next;
    } else {
        window->head = msg->next;
    }
    
    if (msg->next != NULL) {
        msg->next->prev = msg->prev;
    } else {
        window->tail = msg->prev;
    }
    
    free(msg->content);
    free(msg);
}

static bool compress_old_messages(ContextWindow* window) {
    Message* current = window->head;
    while (current != NULL && window->total_tokens > window->max_tokens) {
        if (current->priority == PRIORITY_LOW) {
            Message* to_remove = current;
            current = current->next;
            remove_message(window, to_remove);
            continue;
        }
        current = current->next;
    }
    
    if (window->total_tokens > window->max_tokens) {
        current = window->head;
        while (current != NULL && window->total_tokens > window->max_tokens) {
            if (current->priority == PRIORITY_NORMAL) {
                Message* to_remove = current;
                current = current->next;
                remove_message(window, to_remove);
                continue;
            }
            current = current->next;
        }
    }
    
    if (window->total_tokens > window->max_tokens) {
        current = window->head;
        while (current != NULL && window->total_tokens > window->max_tokens) {
            if (current->priority == PRIORITY_HIGH) {
                Message* to_remove = current;
                current = current->next;
                remove_message(window, to_remove);
                continue;
            }
            current = current->next;
        }
    }
    
    return window->total_tokens <= window->max_tokens;
}

bool context_window_add_message(ContextWindow* window,
                                MessageType type,
                                MessagePriority priority,
                                const char* content) {
    if (window == NULL || content == NULL) return false;
    
    Message* msg = create_message(type, priority, content);
    if (msg == NULL) return false;
    
    int new_total = window->total_tokens + msg->token_count;
    
    if (new_total > window->max_tokens) {
        if (!compress_old_messages(window)) {
            if (msg->token_count > window->max_tokens) {
                printf("Error: Message is too large for context window\n");
                free(msg->content);
                free(msg);
                return false;
            }
            
            while (window->total_tokens + msg->token_count > window->max_tokens) {
                if (window->head == NULL) break;
                remove_message(window, window->head);
            }
        }
    }
    
    if (window->tail == NULL) {
        window->head = msg;
        window->tail = msg;
    } else {
        window->tail->next = msg;
        msg->prev = window->tail;
        window->tail = msg;
    }
    
    window->total_tokens += msg->token_count;
    window->message_count++;
    
    return true;
}

static const char* get_message_type_string(MessageType type) {
    switch (type) {
        case MESSAGE_USER: return "User";
        case MESSAGE_ASSISTANT: return "Assistant";
        case MESSAGE_SYSTEM: return "System";
        case MESSAGE_TOOL: return "Tool";
        default: return "Unknown";
    }
}

char* context_window_get_context(ContextWindow* window) {
    if (window == NULL || window->head == NULL) {
        char* empty = (char*)malloc(1);
        if (empty != NULL) {
            empty[0] = '\0';
        }
        return empty;
    }
    
    int buffer_size = 0;
    Message* current = window->head;
    while (current != NULL) {
        buffer_size += strlen(get_message_type_string(current->type)) + strlen(": ") + 
                      strlen(current->content) + strlen("\n");
        current = current->next;
    }
    
    char* context = (char*)malloc(buffer_size + 1);
    if (context == NULL) {
        printf("Failed to allocate memory for context string\n");
        return NULL;
    }
    
    context[0] = '\0';
    
    current = window->head;
    while (current != NULL) {
        strcat(context, get_message_type_string(current->type));
        strcat(context, ": ");
        strcat(context, current->content);
        strcat(context, "\n");
        current = current->next;
    }
    
    return context;
}

int context_window_get_message_count(ContextWindow* window) {
    return window ? window->message_count : 0;
}

int context_window_get_token_count(ContextWindow* window) {
    return window ? window->total_tokens : 0;
}

void context_window_print_stats(ContextWindow* window) {
    if (window == NULL) {
        printf("Context window is NULL\n");
        return;
    }
    
    printf("Context Window Statistics:\n");
    printf("Total messages: %d\n", window->message_count);
    printf("Total tokens: %d/%d\n", window->total_tokens, window->max_tokens);
    printf("Tokens remaining: %d\n", window->max_tokens - window->total_tokens);
}
