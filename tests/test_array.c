/**
 * @file test_array.c
 * @brief Unit tests for array data structure
 */

#include "../include/array.h"
#include <stdio.h>
#include <assert.h>

static int test_count = 0;
static int test_passed = 0;

#define TEST_START(name) \
    printf("Running test: %s... ", name); \
    test_count++;

#define TEST_END() \
    test_passed++; \
    printf("PASSED\n");

#define TEST_FAIL(msg) \
    printf("FAILED: %s\n", msg);

/* Test array creation */
void test_array_create(void) {
    TEST_START("array_create");
    PCCArray* array = pcc_array_create(16, sizeof(int));
    assert(array != NULL);
    assert(pcc_array_size(array) == 0);
    assert(pcc_array_capacity(array) == 16);
    pcc_array_free(array, NULL);
    TEST_END();
}

/* Test array push */
void test_array_push(void) {
    TEST_START("array_push");
    PCCArray* array = pcc_array_create(16, sizeof(int));
    int value = 42;
    PCCError err = pcc_array_push(array, &value);
    assert(err == PCC_SUCCESS);
    assert(pcc_array_size(array) == 1);
    pcc_array_free(array, NULL);
    TEST_END();
}

/* Test array get */
void test_array_get(void) {
    TEST_START("array_get");
    PCCArray* array = pcc_array_create(16, sizeof(int));
    int value = 42;
    pcc_array_push(array, &value);
    int* result = (int*)pcc_array_get(array, 0);
    assert(result != NULL);
    assert(*result == 42);
    pcc_array_free(array, NULL);
    TEST_END();
}

/* Test array resize */
void test_array_resize(void) {
    TEST_START("array_resize");
    PCCArray* array = pcc_array_create(4, sizeof(int));
    PCCError err = pcc_array_resize(array, 16);
    assert(err == PCC_SUCCESS);
    assert(pcc_array_capacity(array) == 16);
    pcc_array_free(array, NULL);
    TEST_END();
}

/* Test array clear */
void test_array_clear(void) {
    TEST_START("array_clear");
    PCCArray* array = pcc_array_create(16, sizeof(int));
    int value = 42;
    pcc_array_push(array, &value);
    pcc_array_clear(array, NULL);
    assert(pcc_array_size(array) == 0);
    pcc_array_free(array, NULL);
    TEST_END();
}

/* Run all array tests */
void run_array_tests(void) {
    printf("\n=== Array Tests ===\n");
    test_array_create();
    test_array_push();
    test_array_get();
    test_array_resize();
    test_array_clear();
    printf("\nArray tests: %d/%d passed\n", test_passed, test_count);
}
