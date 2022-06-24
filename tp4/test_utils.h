#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>

#define DEBUG_TEST_ENABLE true

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

typedef enum { T_BOOL, T_INT, T_FLOAT, T_STR } VarTypeEnum;

typedef struct { VarTypeEnum type; } TestLog;
typedef struct { VarTypeEnum type; bool value; } TestLogBool;
typedef struct { VarTypeEnum type; int value; } TestLogInt;
typedef struct { VarTypeEnum type; float value; } TestLogFloat;
typedef struct { VarTypeEnum type; char *value; } TestLogStr;

/**
 * ------------------------------------------------
 * == FUNCTIONS ===================================
 * ------------------------------------------------
 */

TestLog* tstInt(int v);
TestLog* tstStr(char *v);
TestLog* tstBool(bool v);
TestLog* tstFloat(float v);

void tstDebugStep(const char* msgOrTemplate, TestLog *param1, TestLog *param2, TestLog *param3);


////// T_BOOL, T_INT, T_FLOAT, T_STR
    
// T_BOOL, T_INT, T_FLOAT
// T_BOOL, T_INT, T_STR
// T_BOOL, T_FLOAT, T_INT
// T_BOOL, T_FLOAT, T_STR
// T_BOOL, T_STR, T_INT
// T_BOOL, T_STR, T_FLOAT

// T_INT, T_BOOL, T_FLOAT
// T_INT, T_BOOL, T_STR
// T_INT, T_FLOAT, T_BOOL
// T_INT, T_FLOAT, T_STR
// T_INT, T_STR, T_BOOL
// T_INT, T_STR, T_FLOAT

// T_FLOAT, T_INT, T_BOOL
// T_FLOAT, T_INT, T_STR
// T_FLOAT, T_BOOL, T_INT
// T_FLOAT, T_BOOL, T_STR
// T_FLOAT, T_STR, T_INT
// T_FLOAT, T_STR, T_BOOL

// T_STR, T_INT, T_FLOAT
// T_STR, T_INT, T_BOOL
// T_STR, T_FLOAT, T_INT
// T_STR, T_FLOAT, T_BOOL
// T_STR, T_BOOL, T_INT
// T_STR, T_BOOL, T_FLOAT

// // T_BOOL, T_INT, T_FLOAT
// printf(msgOrTemplate, ((TestLogBool *)param1)->value, ((TestLogInt *)param2)->value, ((TestLogFloat *)param3)->value);
// // T_BOOL, T_INT, T_STR
// printf(msgOrTemplate, ((TestLogBool *)param1)->value, ((TestLogInt *)param2)->value, ((TestLogStr *)param3)->value);
// // T_BOOL, T_FLOAT, T_INT
// printf(msgOrTemplate, ((TestLogBool *)param1)->value, ((TestLogFloat *)param2)->value, ((TestLogInt *)param3)->value);
// // T_BOOL, T_FLOAT, T_STR
// printf(msgOrTemplate, ((TestLogBool *)param1)->value, ((TestLogFloat *)param2)->value, ((TestLogStr *)param3)->value);
// // T_BOOL, T_STR, T_INT
// printf(msgOrTemplate, ((TestLogBool *)param1)->value, ((TestLogStr *)param2)->value, ((TestLogInt *)param3)->value);
// // T_BOOL, T_STR, T_FLOAT
// printf(msgOrTemplate, ((TestLogBool *)param1)->value, ((TestLogStr *)param2)->value, ((TestLogFloat *)param3)->value);

// // T_INT, T_BOOL, T_FLOAT
// printf(msgOrTemplate, ((TestLogInt *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogFloat *)param3)->value);
// // T_INT, T_BOOL, T_STR
// printf(msgOrTemplate, ((TestLogInt *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogStr *)param3)->value);
// // T_INT, T_FLOAT, T_BOOL
// printf(msgOrTemplate, ((TestLogInt *)param1)->value, ((TestLogFloat *)param2)->value, ((TestLogBool *)param3)->value);
// // T_INT, T_FLOAT, T_STR
// printf(msgOrTemplate, ((TestLogInt *)param1)->value, ((TestLogFloat *)param2)->value, ((TestLogStr *)param3)->value);
// // T_INT, T_STR, T_BOOL
// printf(msgOrTemplate, ((TestLogInt *)param1)->value, ((TestLogStr *)param2)->value, ((TestLogBool *)param3)->value);
// // T_INT, T_STR, T_FLOAT
// printf(msgOrTemplate, ((TestLogInt *)param1)->value, ((TestLogStr *)param2)->value, ((TestLogFloat *)param3)->value);

// // T_FLOAT, T_INT, T_BOOL
// printf(msgOrTemplate, ((TestLogFloat *)param1)->value, ((TestLogInt *)param2)->value, ((TestLogBool *)param3)->value);
// // T_FLOAT, T_INT, T_STR
// printf(msgOrTemplate, ((TestLogFloat *)param1)->value, ((TestLogInt *)param2)->value, ((TestLogStr *)param3)->value);
// // T_FLOAT, T_BOOL, T_INT
// printf(msgOrTemplate, ((TestLogFloat *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogInt *)param3)->value);
// // T_FLOAT, T_BOOL, T_STR
// printf(msgOrTemplate, ((TestLogFloat *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogStr *)param3)->value);
// // T_FLOAT, T_STR, T_INT
// printf(msgOrTemplate, ((TestLogFloat *)param1)->value, ((TestLogStr *)param2)->value, ((TestLogInt *)param3)->value);
// // T_FLOAT, T_STR, T_BOOL
// printf(msgOrTemplate, ((TestLogFloat *)param1)->value, ((TestLogStr *)param2)->value, ((TestLogBool *)param3)->value);

// // T_STR, T_INT, T_FLOAT
// printf(msgOrTemplate, ((TestLogStr *)param1)->value, ((TestLogInt *)param2)->value, ((TestLogFloat *)param3)->value);
// // T_STR, T_INT, T_BOOL
// printf(msgOrTemplate, ((TestLogStr *)param1)->value, ((TestLogInt *)param2)->value, ((TestLogBool *)param3)->value);
// // T_STR, T_FLOAT, T_INT
// printf(msgOrTemplate, ((TestLogStr *)param1)->value, ((TestLogFloat *)param2)->value, ((TestLogInt *)param3)->value);
// // T_STR, T_FLOAT, T_BOOL
// printf(msgOrTemplate, ((TestLogStr *)param1)->value, ((TestLogFloat *)param2)->value, ((TestLogBool *)param3)->value);
// // T_STR, T_BOOL, T_INT
// printf(msgOrTemplate, ((TestLogStr *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogInt *)param3)->value);
// // T_STR, T_BOOL, T_FLOAT
// printf(msgOrTemplate, ((TestLogStr *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogFloat *)param3)->value);