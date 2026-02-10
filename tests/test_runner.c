/**
 * @file test_runner.c
 * @brief Main test runner for PCC compiler
 */

#include <stdio.h>

/* External test functions */
extern void run_array_tests(void);

int main(void) {
    printf("PCC Compiler Test Suite\n");
    printf("========================\n");

    /* Run all test suites */
    run_array_tests();

    printf("\n========================\n");
    printf("All tests completed.\n");

    return 0;
}
