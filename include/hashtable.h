/**
 * @file hashtable.h
 * @brief Hash table implementation for PCC compiler
 * @details Implements a generic hash table with separate chaining
 *          DSA Concept: Hash table for O(1) average case lookups
 */

#ifndef PCC_HASHTABLE_H
#define PCC_HASHTABLE_H

#include "common.h"
#include "array.h"

/* Hash table entry */
typedef struct PCCHashEntry {
    char* key;                    /* String key */
    void* value;                  /* Associated value */
    struct PCCHashEntry* next;    /* Next entry in chain (for collision resolution) */
} PCCHashEntry;

/* Hash table structure */
typedef struct {
    PCCHashEntry** buckets;       /* Array of bucket pointers */
    size_t size;                  /* Number of entries */
    size_t capacity;              /* Number of buckets */
    size_t (*hash_func)(const char*);  /* Hash function */
} PCCHashTable;

/* Default hash function */
size_t pcc_default_hash(const char* key);

/* Function prototypes */

/**
 * @brief Create a new hash table
 * @param initial_capacity Initial number of buckets
 * @param hash_func Optional custom hash function (NULL for default)
 * @return Pointer to new hash table, or NULL on failure
 */
PCCHashTable* pcc_hashtable_create(size_t initial_capacity, size_t (*hash_func)(const char*));

/**
 * @brief Free hash table and all entries
 * @param table Hash table to free
 * @param free_value_func Optional function to free values (can be NULL)
 */
void pcc_hashtable_free(PCCHashTable* table, void (*free_value_func)(void*));

/**
 * @brief Insert or update a key-value pair
 * @param table Hash table
 * @param key Key string
 * @param value Value to associate with key
 * @param free_old_value Optional function to free old value if key exists
 * @return PCC_SUCCESS on success, error code on failure
 */
PCCError pcc_hashtable_put(PCCHashTable* table, const char* key, void* value,
                           void (*free_old_value)(void*));

/**
 * @brief Get value associated with key
 * @param table Hash table
 * @param key Key to look up
 * @return Value associated with key, or NULL if not found
 */
void* pcc_hashtable_get(PCCHashTable* table, const char* key);

/**
 * @brief Check if key exists in table
 * @param table Hash table
 * @param key Key to check
 * @return PCC_TRUE if key exists, PCC_FALSE otherwise
 */
PCCBool pcc_hashtable_contains(PCCHashTable* table, const char* key);

/**
 * @brief Remove key-value pair from table
 * @param table Hash table
 * @param key Key to remove
 * @param out_value Optional pointer to store removed value
 * @return PCC_SUCCESS on success, error code on failure
 */
PCCError pcc_hashtable_remove(PCCHashTable* table, const char* key, void** out_value);

/**
 * @brief Get number of entries in table
 * @param table Hash table
 * @return Number of entries
 */
static inline size_t pcc_hashtable_size(PCCHashTable* table) {
    return table ? table->size : 0;
}

/**
 * @brief Check if table is empty
 * @param table Hash table
 * @return PCC_TRUE if empty, PCC_FALSE otherwise
 */
static inline PCCBool pcc_hashtable_empty(PCCHashTable* table) {
    return (PCCBool)(table ? (table->size == 0) : PCC_TRUE);
}

/**
 * @brief Get current capacity (number of buckets)
 * @param table Hash table
 * @return Current capacity
 */
static inline size_t pcc_hashtable_capacity(PCCHashTable* table) {
    return table ? table->capacity : 0;
}

/**
 * @brief Clear all entries from table
 * @param table Hash table
 * @param free_value_func Optional function to free values
 */
void pcc_hashtable_clear(PCCHashTable* table, void (*free_value_func)(void*));

/**
 * @brief Resize hash table to new capacity
 * @param table Hash table
 * @param new_capacity New capacity
 * @return PCC_SUCCESS on success, error code on failure
 */
PCCError pcc_hashtable_resize(PCCHashTable* table, size_t new_capacity);

/**
 * @brief Get all keys in table
 * @param table Hash table
 * @param keys Array to store keys (must be pre-allocated)
 * @return Number of keys retrieved
 */
size_t pcc_hashtable_get_keys(PCCHashTable* table, PCCArray* keys);

/**
 * @brief Get all values in table
 * @param table Hash table
 * @param values Array to store values (must be pre-allocated)
 * @return Number of values retrieved
 */
size_t pcc_hashtable_get_values(PCCHashTable* table, PCCArray* values);

/**
 * @brief Get all entries (key-value pairs) in table
 * @param table Hash table
 * @param entries Array to store entries
 * @return Number of entries retrieved
 */
size_t pcc_hashtable_get_entries(PCCHashTable* table, PCCArray* entries);

/**
 * @brief Calculate load factor (size / capacity)
 * @param table Hash table
 * @return Load factor as double
 */
double pcc_hashtable_load_factor(PCCHashTable* table);

/**
 * @brief Iterator for hash table entries
 */
typedef struct {
    PCCHashTable* table;
    size_t bucket_index;
    PCCHashEntry* current_entry;
} PCCHashTableIterator;

/**
 * @brief Create iterator for hash table
 * @param table Hash table to iterate
 * @return New iterator
 */
PCCHashTableIterator pcc_hashtable_iterator_create(PCCHashTable* table);

/**
 * @brief Get next entry from iterator
 * @param iterator Iterator
 * @param out_key Pointer to store key
 * @param out_value Pointer to store value
 * @return PCC_TRUE if more entries, PCC_FALSE if done
 */
PCCBool pcc_hashtable_iterator_next(PCCHashTableIterator* iterator, char** out_key, void** out_value);

/**
 * @brief Reset iterator to beginning
 * @param iterator Iterator to reset
 */
void pcc_hashtable_iterator_reset(PCCHashTableIterator* iterator);

#endif /* PCC_HASHTABLE_H */
