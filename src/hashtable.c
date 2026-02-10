/**
 * @file hashtable.c
 * @brief Hash table implementation for PCC compiler
 */

#include "hashtable.h"
#include <string.h>

/* Default hash function - DJB2 algorithm */
size_t pcc_default_hash(const char* key) {
    if (key == NULL) {
        return 0;
    }

    size_t hash = 5381;
    int c;

    while ((c = *key++)) {
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */
    }

    return hash;
}

/* Create a new hash table */
PCCHashTable* pcc_hashtable_create(size_t initial_capacity, size_t (*hash_func)(const char*)) {
    if (initial_capacity == 0) {
        initial_capacity = INITIAL_CAPACITY;
    }

    PCCHashTable* table = (PCCHashTable*)malloc(sizeof(PCCHashTable));
    if (table == NULL) {
        return NULL;
    }

    table->buckets = (PCCHashEntry**)calloc(initial_capacity, sizeof(PCCHashEntry*));
    if (table->buckets == NULL) {
        free(table);
        return NULL;
    }

    table->size = 0;
    table->capacity = initial_capacity;
    table->hash_func = hash_func ? hash_func : pcc_default_hash;

    return table;
}

/* Free hash table and all entries */
void pcc_hashtable_free(PCCHashTable* table, void (*free_value_func)(void*)) {
    if (table == NULL) {
        return;
    }

    /* Free all entries */
    for (size_t i = 0; i < table->capacity; i++) {
        PCCHashEntry* entry = table->buckets[i];
        while (entry != NULL) {
            PCCHashEntry* next = entry->next;
            free(entry->key);
            if (free_value_func != NULL && entry->value != NULL) {
                free_value_func(entry->value);
            }
            free(entry);
            entry = next;
        }
    }

    free(table->buckets);
    free(table);
}

/* Insert or update a key-value pair */
PCCError pcc_hashtable_put(PCCHashTable* table, const char* key, void* value,
                           void (*free_old_value)(void*)) {
    if (table == NULL || key == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    /* Check if we need to resize (load factor > 0.75) */
    if (table->size > table->capacity * 3 / 4) {
        PCCError err = pcc_hashtable_resize(table, table->capacity * 2);
        if (err != PCC_SUCCESS) {
            return err;
        }
    }

    /* Calculate bucket index */
    size_t hash = table->hash_func(key);
    size_t index = hash % table->capacity;

    /* Check if key already exists */
    PCCHashEntry* entry = table->buckets[index];
    PCCHashEntry* prev = NULL;

    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            /* Key exists, update value */
            if (free_old_value != NULL && entry->value != NULL) {
                free_old_value(entry->value);
            }
            entry->value = value;
            return PCC_SUCCESS;
        }
        prev = entry;
        entry = entry->next;
    }

    /* Create new entry */
    PCCHashEntry* new_entry = (PCCHashEntry*)malloc(sizeof(PCCHashEntry));
    if (new_entry == NULL) {
        return PCC_ERROR_MEMORY;
    }

    new_entry->key = pcc_strdup(key);
    if (new_entry->key == NULL) {
        free(new_entry);
        return PCC_ERROR_MEMORY;
    }

    new_entry->value = value;
    new_entry->next = NULL;

    /* Add to bucket */
    if (prev == NULL) {
        table->buckets[index] = new_entry;
    } else {
        prev->next = new_entry;
    }

    table->size++;

    return PCC_SUCCESS;
}

/* Get value associated with key */
void* pcc_hashtable_get(PCCHashTable* table, const char* key) {
    if (table == NULL || key == NULL) {
        return NULL;
    }

    size_t hash = table->hash_func(key);
    size_t index = hash % table->capacity;

    PCCHashEntry* entry = table->buckets[index];

    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            return entry->value;
        }
        entry = entry->next;
    }

    return NULL;
}

/* Check if key exists in table */
PCCBool pcc_hashtable_contains(PCCHashTable* table, const char* key) {
    return (PCCBool)(pcc_hashtable_get(table, key) != NULL);
}

/* Remove key-value pair from table */
PCCError pcc_hashtable_remove(PCCHashTable* table, const char* key, void** out_value) {
    if (table == NULL || key == NULL) {
        return PCC_ERROR_RUNTIME;
    }

    size_t hash = table->hash_func(key);
    size_t index = hash % table->capacity;

    PCCHashEntry* entry = table->buckets[index];
    PCCHashEntry* prev = NULL;

    while (entry != NULL) {
        if (strcmp(entry->key, key) == 0) {
            /* Found the entry */
            if (out_value != NULL) {
                *out_value = entry->value;
            }

            if (prev == NULL) {
                table->buckets[index] = entry->next;
            } else {
                prev->next = entry->next;
            }

            free(entry->key);
            free(entry);
            table->size--;

            return PCC_SUCCESS;
        }
        prev = entry;
        entry = entry->next;
    }

    return PCC_ERROR_RUNTIME; /* Key not found */
}

/* Clear all entries from table */
void pcc_hashtable_clear(PCCHashTable* table, void (*free_value_func)(void*)) {
    if (table == NULL) {
        return;
    }

    /* Free all entries */
    for (size_t i = 0; i < table->capacity; i++) {
        PCCHashEntry* entry = table->buckets[i];
        while (entry != NULL) {
            PCCHashEntry* next = entry->next;
            free(entry->key);
            if (free_value_func != NULL && entry->value != NULL) {
                free_value_func(entry->value);
            }
            free(entry);
            entry = next;
        }
        table->buckets[i] = NULL;
    }

    table->size = 0;
}

/* Resize hash table to new capacity */
PCCError pcc_hashtable_resize(PCCHashTable* table, size_t new_capacity) {
    if (table == NULL || new_capacity < table->size) {
        return PCC_ERROR_RUNTIME;
    }

    /* Allocate new buckets */
    PCCHashEntry** new_buckets = (PCCHashEntry**)calloc(new_capacity, sizeof(PCCHashEntry*));
    if (new_buckets == NULL) {
        return PCC_ERROR_MEMORY;
    }

    /* Rehash all entries */
    for (size_t i = 0; i < table->capacity; i++) {
        PCCHashEntry* entry = table->buckets[i];
        while (entry != NULL) {
            PCCHashEntry* next = entry->next;

            /* Calculate new bucket index */
            size_t hash = table->hash_func(entry->key);
            size_t new_index = hash % new_capacity;

            /* Add to new bucket */
            entry->next = new_buckets[new_index];
            new_buckets[new_index] = entry;

            entry = next;
        }
    }

    /* Free old buckets */
    free(table->buckets);

    /* Update table */
    table->buckets = new_buckets;
    table->capacity = new_capacity;

    return PCC_SUCCESS;
}

/* Get all keys in table */
size_t pcc_hashtable_get_keys(PCCHashTable* table, PCCArray* keys) {
    if (table == NULL || keys == NULL) {
        return 0;
    }

    size_t count = 0;

    for (size_t i = 0; i < table->capacity; i++) {
        PCCHashEntry* entry = table->buckets[i];
        while (entry != NULL) {
            pcc_array_push(keys, entry->key);
            count++;
            entry = entry->next;
        }
    }

    return count;
}

/* Get all values in table */
size_t pcc_hashtable_get_values(PCCHashTable* table, PCCArray* values) {
    if (table == NULL || values == NULL) {
        return 0;
    }

    size_t count = 0;

    for (size_t i = 0; i < table->capacity; i++) {
        PCCHashEntry* entry = table->buckets[i];
        while (entry != NULL) {
            pcc_array_push(values, entry->value);
            count++;
            entry = entry->next;
        }
    }

    return count;
}

/* Get all entries (key-value pairs) in table */
size_t pcc_hashtable_get_entries(PCCHashTable* table, PCCArray* entries) {
    if (table == NULL || entries == NULL) {
        return 0;
    }

    size_t count = 0;

    for (size_t i = 0; i < table->capacity; i++) {
        PCCHashEntry* entry = table->buckets[i];
        while (entry != NULL) {
            pcc_array_push(entries, entry);
            count++;
            entry = entry->next;
        }
    }

    return count;
}

/* Calculate load factor */
double pcc_hashtable_load_factor(PCCHashTable* table) {
    if (table == NULL || table->capacity == 0) {
        return 0.0;
    }

    return (double)table->size / (double)table->capacity;
}

/* Create iterator for hash table */
PCCHashTableIterator pcc_hashtable_iterator_create(PCCHashTable* table) {
    PCCHashTableIterator iterator;
    iterator.table = table;
    iterator.bucket_index = 0;
    iterator.current_entry = NULL;

    if (table != NULL) {
        /* Find first non-empty bucket */
        while (iterator.bucket_index < table->capacity &&
               table->buckets[iterator.bucket_index] == NULL) {
            iterator.bucket_index++;
        }
        if (iterator.bucket_index < table->capacity) {
            iterator.current_entry = table->buckets[iterator.bucket_index];
        }
    }

    return iterator;
}

/* Get next entry from iterator */
PCCBool pcc_hashtable_iterator_next(PCCHashTableIterator* iterator, char** out_key, void** out_value) {
    if (iterator == NULL || iterator->table == NULL || iterator->current_entry == NULL) {
        return PCC_FALSE;
    }

    /* Return current entry */
    if (out_key != NULL) {
        *out_key = iterator->current_entry->key;
    }
    if (out_value != NULL) {
        *out_value = iterator->current_entry->value;
    }

    /* Move to next entry */
    iterator->current_entry = iterator->current_entry->next;

    /* If no more entries in current bucket, find next non-empty bucket */
    if (iterator->current_entry == NULL) {
        iterator->bucket_index++;
        while (iterator->bucket_index < iterator->table->capacity &&
               iterator->table->buckets[iterator->bucket_index] == NULL) {
            iterator->bucket_index++;
        }
        if (iterator->bucket_index < iterator->table->capacity) {
            iterator->current_entry = iterator->table->buckets[iterator->bucket_index];
        }
    }

    return PCC_TRUE;
}

/* Reset iterator to beginning */
void pcc_hashtable_iterator_reset(PCCHashTableIterator* iterator) {
    if (iterator == NULL) {
        return;
    }

    iterator->bucket_index = 0;
    iterator->current_entry = NULL;

    if (iterator->table != NULL) {
        /* Find first non-empty bucket */
        while (iterator->bucket_index < iterator->table->capacity &&
               iterator->table->buckets[iterator->bucket_index] == NULL) {
            iterator->bucket_index++;
        }
        if (iterator->bucket_index < iterator->table->capacity) {
            iterator->current_entry = iterator->table->buckets[iterator->bucket_index];
        }
    }
}
