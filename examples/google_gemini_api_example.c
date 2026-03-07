#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <time.h>

#include "context_window.h"

/* API Configuration */
#define GEMINI_API_BASE_URL "https://generativelanguage.googleapis.com/v1beta"
#define GEMINI_MODEL "gemini-1.5-flash"
#define MAX_REQUEST_BODY_SIZE 32768
#define MAX_RESPONSE_SIZE 65536

/* Error codes */
typedef enum {
    GEMINI_SUCCESS = 0,
    GEMINI_ERROR_NO_API_KEY,
    GEMINI_ERROR_NETWORK,
    GEMINI_ERROR_HTTP,
    GEMINI_ERROR_JSON_PARSE,
    GEMINI_ERROR_RATE_LIMIT,
    GEMINI_ERROR_AUTH,
    GEMINI_ERROR_INVALID_REQUEST,
    GEMINI_ERROR_SERVER,
    GEMINI_ERROR_NO_MEMORY,
    GEMINI_ERROR_UNKNOWN
} GeminiResult;

/* Request structure */
typedef struct {
    const char* model;
    char** contents;
    int content_count;
    int max_tokens;
    float temperature;
} GeminiRequest;

/* Response structure */
typedef struct {
    char* text;
    GeminiResult result;
    int http_code;
    char* error_message;
} GeminiResponse;

/* HTTP client typedef */
typedef void* HTTPCurl;

/* Function declarations */
HTTPCurl http_init(void);
void http_set_header(HTTPCurl curl, const char* key, const char* value);
int http_post(HTTPCurl curl, const char* url, const char* body,
              char* response, int response_size);
void http_cleanup(HTTPCurl curl);

char* gemini_build_request_json(const GeminiRequest* request);
GeminiResult gemini_parse_response_json(const char* json_response,
                                         GeminiResponse* response);
char* json_escape_string(const char* str);

const char* gemini_get_api_key(void);
char* gemini_create_url(const char* api_key, const char* model);
GeminiResult gemini_chat_completion(const char* api_key,
                                    const char* model,
                                    const GeminiRequest* request,
                                    GeminiResponse* response);
void gemini_free_response(GeminiResponse* response);

void gemini_add_message_to_request(GeminiRequest* request,
                                    MessageType pcc_type,
                                    const char* content);
void gemini_build_request_from_window(GeminiRequest* request,
                                       ContextWindow* window,
                                       const char* user_message);
void gemini_free_request(GeminiRequest* request);

/* Example data */
static const char* EXAMPLE_SYSTEM_PROMPT = 
    "You are a helpful, accurate, and concise AI assistant.";

static const char* EXAMPLE_USER_MESSAGE_1 = 
    "What are the key benefits of using context windows in LLM applications?";

static const char* EXAMPLE_ASSISTANT_MESSAGE_1 = 
    "Key benefits of context windows include: memory management, "
    "token optimization, priority-based retention, and cost control.";

static const char* EXAMPLE_USER_MESSAGE_2 = 
    "How does priority-based eviction work?";

int main(void) {
    printf("========================================\n");
    printf("  PCC - Google Gemini API Integration\n");
    printf("========================================\n\n");
    
    const char* api_key = gemini_get_api_key();
    if (api_key == NULL) {
        printf("Note: GEMINI_API_KEY not set.\n");
        printf("Running in demonstration mode without API calls.\n\n");
    }
    
    printf("Creating context window with 1000 token limit...\n");
    ContextWindow* window = context_window_create(1000);
    if (window == NULL) {
        fprintf(stderr, "Failed to create context window\n");
        return 1;
    }
    
    ContextConfig config = context_config_default();
    config.max_tokens = 1000;
    config.enable_metrics = true;
    config.auto_compress = false;
    context_window_apply_config(window, &config);
    
    printf("Adding system prompt (CRITICAL priority)...\n");
    context_window_add_message(window, MESSAGE_SYSTEM, PRIORITY_CRITICAL,
                                EXAMPLE_SYSTEM_PROMPT);
    
    printf("Adding conversation history...\n");
    context_window_add_message(window, MESSAGE_USER, PRIORITY_HIGH,
                                EXAMPLE_USER_MESSAGE_1);
    context_window_add_message(window, MESSAGE_ASSISTANT, PRIORITY_NORMAL,
                                EXAMPLE_ASSISTANT_MESSAGE_1);
    context_window_add_message(window, MESSAGE_USER, PRIORITY_HIGH,
                                EXAMPLE_USER_MESSAGE_2);
    
    printf("\n--- Context Window State ---\n");
    context_window_print_stats(window);
    printf("\nUtilization: %.1f%%\n", context_window_get_utilization(window));
    printf("Messages: %d\n", context_window_get_message_count(window));
    printf("Tokens: %d\n", context_window_get_token_count(window));
    
    printf("\n--- Context for Gemini API ---\n");
    char* context = context_window_get_context(window);
    if (context != NULL) {
        printf("%s\n", context);
        free(context);
    }
    
    if (api_key != NULL) {
        printf("\n--- Sending Request to Gemini API ---\n");
        
        GeminiRequest gemini_req = {0};
        gemini_req.model = GEMINI_MODEL;
        gemini_req.max_tokens = 500;
        gemini_req.temperature = 0.7f;
        
        gemini_build_request_from_window(&gemini_req, window, 
            "Please explain priority-based eviction in context windows.");
        
        GeminiResponse gemini_resp = {0};
        GeminiResult result = gemini_chat_completion(api_key, 
                                                     GEMINI_MODEL,
                                                     &gemini_req, 
                                                     &gemini_resp);
        
        if (result == GEMINI_SUCCESS) {
            printf("API Response:\n%s\n", gemini_resp.text);
            
            context_window_add_message(window, MESSAGE_ASSISTANT, PRIORITY_NORMAL,
                                        gemini_resp.text);
            
            printf("\nAdded response to context window.\n");
            printf("New token count: %d\n", context_window_get_token_count(window));
        } else {
            fprintf(stderr, "API Error: %s (HTTP %d)\n", 
                    gemini_resp.error_message ? gemini_resp.error_message : "Unknown",
                    gemini_resp.http_code);
        }
        
        gemini_free_response(&gemini_resp);
        gemini_free_request(&gemini_req);
    } else {
        printf("\n--- Demo Mode: Simulated API Response ---\n");
        const char* simulated_response = 
            "Priority-based eviction works by assigning priority levels to messages. "
            "When the context window reaches its token limit, messages are removed "
            "starting from the lowest priority, then normal, then high, "
            "with critical messages being almost never evicted.";
        
        printf("Simulated response:\n%s\n", simulated_response);
        
        context_window_add_message(window, MESSAGE_ASSISTANT, PRIORITY_NORMAL,
                                    simulated_response);
    }
    
    printf("\n--- Final Context Window Statistics ---\n");
    context_window_print_stats(window);
    printf("\nFinal utilization: %.1f%%\n", context_window_get_utilization(window));
    
    printf("\n--- Performance Metrics ---\n");
    context_window_print_metrics(window);
    
    context_window_destroy(window);
    
    printf("\n========================================\n");
    printf("  Example completed successfully!\n");
    printf("========================================\n");
    
    return 0;
}

/* API Implementation */

const char* gemini_get_api_key(void) {
    return getenv("GEMINI_API_KEY");
}

char* gemini_create_url(const char* api_key, const char* model) {
    char* url = malloc(256);
    if (url == NULL) return NULL;
    
    snprintf(url, 256, "%s/models/%s:generateContent?key=%s",
             GEMINI_API_BASE_URL, model, api_key);
    
    return url;
}

char* json_escape_string(const char* str) {
    if (str == NULL) return NULL;
    
    size_t escaped_len = 0;
    const char* p = str;
    while (*p) {
        switch (*p) {
            case '"':  case '\\':  case '\b':  case '\f':  case '\n': 
            case '\r':  case '\t':
                escaped_len += 2;
                break;
            default:
                escaped_len += 1;
                break;
        }
        p++;
    }
    
    char* escaped = malloc(escaped_len + 1);
    if (escaped == NULL) return NULL;
    
    char* out = escaped;
    p = str;
    while (*p) {
        switch (*p) {
            case '"':
                *out++ = '\\'; *out++ = '"';
                break;
            case '\\':
                *out++ = '\\'; *out++ = '\\';
                break;
            case '\b':
                *out++ = '\\'; *out++ = 'b';
                break;
            case '\f':
                *out++ = '\\'; *out++ = 'f';
                break;
            case '\n':
                *out++ = '\\'; *out++ = 'n';
                break;
            case '\r':
                *out++ = '\\'; *out++ = 'r';
                break;
            case '\t':
                *out++ = '\\'; *out++ = 't';
                break;
            default:
                *out++ = *p;
                break;
        }
        p++;
    }
    *out = '\0';
    
    return escaped;
}

char* gemini_build_request_json(const GeminiRequest* request) {
    size_t json_size = MAX_REQUEST_BODY_SIZE;
    char* json = malloc(json_size);
    if (json == NULL) return NULL;
    
    strcpy(json, "{");
    strcat(json, "\"contents\":[");
    
    for (int i = 0; i < request->content_count; i++) {
        if (i > 0) strcat(json, ",");
        
        const char* role = "user";
        if (i % 2 == 1) role = "model";
        
        strcat(json, "{\"role\":\"");
        strcat(json, role);
        strcat(json, "\",\"parts\":[{\"text\":");
        
        char* escaped = json_escape_string(request->contents[i]);
        if (escaped) {
            strcat(json, "\"");
            strcat(json, escaped);
            strcat(json, "\"");
            free(escaped);
        } else {
            strcat(json, "\"\"");
        }
        
        strcat(json, "}]}");
    }
    
    strcat(json, "],");
    
    char config[256];
    snprintf(config, sizeof(config),
             "\"generationConfig\":{\"maxOutputTokens\":%d,\"temperature\":%.2f}",
             request->max_tokens, request->temperature);
    strcat(json, config);
    
    strcat(json, "}");
    
    return json;
}

GeminiResult gemini_parse_response_json(const char* json_response,
                                         GeminiResponse* response) {
    if (json_response == NULL || response == NULL) {
        return GEMINI_ERROR_INVALID_REQUEST;
    }
    
    const char* text_start = strstr(json_response, "\"text\"");
    if (text_start == NULL) {
        const char* error_start = strstr(json_response, "\"error\"");
        if (error_start) {
            response->result = GEMINI_ERROR_SERVER;
            return GEMINI_ERROR_SERVER;
        }
        return GEMINI_ERROR_JSON_PARSE;
    }
    
    const char* colon = strchr(text_start, ':');
    if (colon == NULL) return GEMINI_ERROR_JSON_PARSE;
    
    colon++;
    while (*colon && (*colon == ' ' || *colon == '\t' || *colon == '\"')) colon++;
    
    const char* end = colon + 1;
    while (*end && *end != '\"') end++;
    
    if (*end != '\"') return GEMINI_ERROR_JSON_PARSE;
    
    size_t text_len = end - colon;
    response->text = malloc(text_len + 1);
    if (response->text == NULL) return GEMINI_ERROR_NO_MEMORY;
    
    strncpy(response->text, colon, text_len);
    response->text[text_len] = '\0';
    
    return GEMINI_SUCCESS;
}

GeminiResult gemini_chat_completion(const char* api_key,
                                    const char* model,
                                    const GeminiRequest* request,
                                    GeminiResponse* response) {
    if (api_key == NULL || request == NULL || response == NULL) {
        return GEMINI_ERROR_NO_API_KEY;
    }
    
    memset(response, 0, sizeof(GeminiResponse));
    
    char* url = gemini_create_url(api_key, model);
    if (url == NULL) {
        return GEMINI_ERROR_UNKNOWN;
    }
    
    char* json_body = gemini_build_request_json(request);
    if (json_body == NULL) {
        free(url);
        return GEMINI_ERROR_UNKNOWN;
    }
    
    HTTPCurl curl = http_init();
    if (curl == NULL) {
        free(url);
        free(json_body);
        return GEMINI_ERROR_NETWORK;
    }
    
    http_set_header(curl, "Content-Type", "application/json");
    
    char* response_buffer = malloc(MAX_RESPONSE_SIZE);
    if (response_buffer == NULL) {
        http_cleanup(curl);
        free(url);
        free(json_body);
        return GEMINI_ERROR_NO_MEMORY;
    }
    
    int http_code = http_post(curl, url, json_body, response_buffer, MAX_RESPONSE_SIZE);
    
    http_cleanup(curl);
    free(url);
    free(json_body);
    
    response->http_code = http_code;
    
    if (http_code < 0) {
        free(response_buffer);
        return GEMINI_ERROR_NETWORK;
    }
    
    if (http_code == 401) {
        free(response_buffer);
        return GEMINI_ERROR_AUTH;
    }
    
    if (http_code == 429) {
        free(response_buffer);
        return GEMINI_ERROR_RATE_LIMIT;
    }
    
    if (http_code >= 500) {
        free(response_buffer);
        return GEMINI_ERROR_SERVER;
    }
    
    if (http_code != 200) {
        response->error_message = strdup("HTTP request failed");
        free(response_buffer);
        return GEMINI_ERROR_HTTP;
    }
    
    GeminiResult result = gemini_parse_response_json(response_buffer, response);
    free(response_buffer);
    
    if (result != GEMINI_SUCCESS) {
        return result;
    }
    
    return GEMINI_SUCCESS;
}

void gemini_free_response(GeminiResponse* response) {
    if (response == NULL) return;
    if (response->text) free(response->text);
    if (response->error_message) free(response->error_message);
    memset(response, 0, sizeof(GeminiResponse));
}

void gemini_add_message_to_request(GeminiRequest* request,
                                    MessageType pcc_type,
                                    const char* content) {
    (void)pcc_type;
    if (request == NULL || content == NULL) return;
    
    char** new_contents = realloc(request->contents,
                                   sizeof(char*) * (request->content_count + 1));
    if (new_contents == NULL) return;
    
    request->contents = new_contents;
    request->contents[request->content_count] = strdup(content);
    request->content_count++;
}

void gemini_build_request_from_window(GeminiRequest* request,
                                       ContextWindow* window,
                                       const char* user_message) {
    if (request == NULL || window == NULL) return;
    
    for (int i = 0; i < request->content_count; i++) {
        free(request->contents[i]);
    }
    free(request->contents);
    
    request->contents = NULL;
    request->content_count = 0;
    
    Message* msg = window->head;
    while (msg != NULL) {
        gemini_add_message_to_request(request, msg->type, msg->content);
        msg = msg->next;
    }
    
    if (user_message != NULL) {
        gemini_add_message_to_request(request, MESSAGE_USER, user_message);
    }
}

void gemini_free_request(GeminiRequest* request) {
    if (request == NULL) return;
    
    for (int i = 0; i < request->content_count; i++) {
        free(request->contents[i]);
    }
    free(request->contents);
    
    request->contents = NULL;
    request->content_count = 0;
}

/* HTTP Client Implementation using libcurl */
#include <curl/curl.h>

struct CurlHeaders {
    struct curl_slist* headers;
};

HTTPCurl http_init(void) {
    CURL* curl = curl_easy_init();
    return (HTTPCurl)curl;
}

void http_set_header(HTTPCurl curl_handle, const char* key, const char* value) {
    CURL* curl = (CURL*)curl_handle;
    if (curl == NULL) return;
    
    char header[512];
    snprintf(header, sizeof(header), "%s: %s", key, value);
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, curl_slist_append(NULL, header));
}

static size_t write_callback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    char** buffer = (char**)userp;
    
    char* ptr = realloc(*buffer, strlen(*buffer) + realsize + 1);
    if (ptr == NULL) return 0;
    
    *buffer = ptr;
    memcpy((*buffer) + strlen(*buffer), contents, realsize);
    (*buffer)[strlen(*buffer)] = '\0';
    
    return realsize;
}

int http_post(HTTPCurl curl_handle, const char* url, const char* body,
              char* response, int response_size) {
    (void)response_size;
    CURL* curl = (CURL*)curl_handle;
    if (curl == NULL) return -1;
    
    response[0] = '\0';
    
    curl_easy_setopt(curl, CURLOPT_URL, url);
    curl_easy_setopt(curl, CURLOPT_POST, 1L);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, body);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    
    CURLcode res = curl_easy_perform(curl);
    
    if (res != CURLE_OK) {
        return -1;
    }
    
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    
    return (int)http_code;
}

void http_cleanup(HTTPCurl curl_handle) {
    CURL* curl = (CURL*)curl_handle;
    if (curl != NULL) {
        curl_easy_cleanup(curl);
    }
}
