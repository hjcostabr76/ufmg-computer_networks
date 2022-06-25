#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define DEBUG_TEST_ENABLE false

/**
 * ------------------------------------------------
 * == ABSTRACTS ===================================
 * ------------------------------------------------
 */

typedef struct {
    int nTests;
    int nFailures;
} TestResult;

typedef struct {
    int nTests;
    int nFailures;
    TestResult tests[20];
} TestBatchResult;

/**
 * ------------------------------------------------
 * == FUNCTIONS ===================================
 * ------------------------------------------------
 */

void tstDebugStep(const char* msgOrTemplate, const void *param1, const void *param2, const void *param3, const void *param4, const void *param5);
char* tstBool(bool v);