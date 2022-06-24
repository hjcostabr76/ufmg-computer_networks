#include "test_utils.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

/**
 * ------------------------------------------------
 * == FUNCTIONS ===================================
 * ------------------------------------------------
 */

TestLog* tstBool(bool v) {
    TestLogBool *log = (TestLogBool *)malloc(sizeof(TestLogBool));
    log->type = T_BOOL;
    log->value = v;
    return (TestLog *)log;
}

TestLog* tstInt(int v) {
    TestLogInt *log = (TestLogInt *)malloc(sizeof(TestLogInt));
    log->type = T_INT;
    log->value = v;
    return (TestLog *)log;
}

TestLog* tstFloat(float v) {
    TestLogFloat *log = (TestLogFloat *)malloc(sizeof(TestLogFloat));
    log->type = T_FLOAT;
    log->value = v;
    return (TestLog *)log;
}

TestLog* tstStr(char *v) {
    TestLogStr *log = (TestLogStr *)malloc(sizeof(TestLogStr));
    log->type = T_STR;
    log->value = (char *)malloc(strlen(v));
    strcpy(log->value, v);
    return (TestLog *)log;
}

void tstDebugStep(const char* msgOrTemplate, TestLog *param1, TestLog *param2, TestLog *param3) {

    if (!DEBUG_TEST_ENABLE)
		return;

    // Open space for the dummies
    const bool shouldFree1 = param1 == NULL;
    const bool shouldFree2 = param2 == NULL;
    const bool shouldFree3 = param3 == NULL;
    
    int dummiesCount = 0;
    if (shouldFree1) {
        param1 = (TestLog *)malloc(sizeof(TestLogBool));
        param1 = tstBool(false);
        dummiesCount++;
    }

    if (shouldFree2) {
        param2 = (TestLog *)malloc(sizeof(TestLogBool));
        param2 = tstBool(false);
        dummiesCount++;
    }

    if (shouldFree3) {
        param3 = (TestLog *)malloc(sizeof(TestLogBool));
        param3 = tstBool(false);
        dummiesCount++;
    }

    char dummy[] = " _(%d)";
    const int dummySize = strlen(dummy);
    char *template = (char *)malloc(strlen(msgOrTemplate) + dummiesCount*dummySize + 1);
    strcpy(template, msgOrTemplate);

    // Include dummies as placeholders
    if (dummiesCount) {
        for (int i = 0; i < dummiesCount; i++)
            strcat(template, dummy);
    }

    printf("\n[log: test] ");

    if (param1->type == T_BOOL) {

        if (param2->type == T_INT) {

            if (param3->type == T_FLOAT) {
                // T_BOOL, T_INT, T_FLOAT
                printf(template, ((TestLogBool *)param1)->value, ((TestLogInt *)param2)->value, ((TestLogFloat *)param3)->value);
        
            } else if (param3->type == T_STR) {
                // T_BOOL, T_INT, T_STR
                printf(template, ((TestLogBool *)param1)->value, ((TestLogInt *)param2)->value, ((TestLogStr *)param3)->value);
            } else 
                printf(template, ((TestLogBool *)param1)->value, ((TestLogInt *)param2)->value, ((TestLogBool *)param3)->value);
        
        } else if (param2->type == T_FLOAT) {

            if (param3->type == T_INT) {
                // T_BOOL, T_FLOAT, T_INT
                printf(template, ((TestLogBool *)param1)->value, ((TestLogFloat *)param2)->value, ((TestLogInt *)param3)->value);
        
            } else if (param3->type == T_STR) {
                // T_BOOL, T_FLOAT, T_STR
                printf(template, ((TestLogBool *)param1)->value, ((TestLogFloat *)param2)->value, ((TestLogStr *)param3)->value);
            } else
                printf(template, ((TestLogBool *)param1)->value, ((TestLogFloat *)param2)->value, ((TestLogBool *)param3)->value);

        } else if (param2->type == T_STR) {

            if (param3->type == T_INT) {
                // T_BOOL, T_STR, T_INT
                printf(template, ((TestLogBool *)param1)->value, ((TestLogStr *)param2)->value, ((TestLogInt *)param3)->value);
        
            } else if (param3->type == T_FLOAT) {
                // T_BOOL, T_STR, T_FLOAT
                printf(template, ((TestLogBool *)param1)->value, ((TestLogStr *)param2)->value, ((TestLogFloat *)param3)->value);
            } else {
                printf(template, ((TestLogBool *)param1)->value, ((TestLogStr *)param2)->value, ((TestLogBool *)param3)->value);
            }
        } else {
            printf(template, ((TestLogBool *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogBool *)param3)->value);
        }

    } else if (param1->type == T_INT) {

        if (param2->type == T_BOOL) {

            if (param3->type == T_FLOAT) {
                // T_INT, T_BOOL, T_FLOAT
                printf(template, ((TestLogInt *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogFloat *)param3)->value);
        
            } else if (param3->type == T_STR) {
                // T_INT, T_BOOL, T_STR
                printf(template, ((TestLogInt *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogStr *)param3)->value);
            } else
                printf(template, ((TestLogBool *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogBool *)param3)->value);
        
        } else if (param2->type == T_FLOAT) {

            if (param3->type == T_BOOL) {
                // T_INT, T_FLOAT, T_BOOL
                printf(template, ((TestLogInt *)param1)->value, ((TestLogFloat *)param2)->value, ((TestLogBool *)param3)->value);
        
            } else if (param3->type == T_STR) {
                // T_INT, T_FLOAT, T_STR
                printf(template, ((TestLogInt *)param1)->value, ((TestLogFloat *)param2)->value, ((TestLogStr *)param3)->value);
            } else
                printf(template, ((TestLogBool *)param1)->value, ((TestLogFloat *)param2)->value, ((TestLogBool *)param3)->value);

        } else if (param2->type == T_STR) {

            if (param3->type == T_BOOL) {
                // T_INT, T_STR, T_BOOL
                printf(template, ((TestLogInt *)param1)->value, ((TestLogStr *)param2)->value, ((TestLogBool *)param3)->value);
        
            } else if (param3->type == T_FLOAT) {
                // T_INT, T_STR, T_FLOAT
                printf(template, ((TestLogInt *)param1)->value, ((TestLogStr *)param2)->value, ((TestLogFloat *)param3)->value);
            } else {
                printf(template, ((TestLogInt *)param1)->value, ((TestLogStr *)param2)->value, ((TestLogBool *)param3)->value);
            }
        } else {
            printf(template, ((TestLogInt *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogBool *)param3)->value);
        }

    } else if (param1->type == T_FLOAT) {

        if (param2->type == T_INT) {

            if (param3->type == T_BOOL) {
                // T_FLOAT, T_INT, T_BOOL
                printf(template, ((TestLogFloat *)param1)->value, ((TestLogInt *)param2)->value, ((TestLogBool *)param3)->value);
        
            } else if (param3->type == T_STR) {
                // T_FLOAT, T_INT, T_STR
                printf(template, ((TestLogFloat *)param1)->value, ((TestLogInt *)param2)->value, ((TestLogStr *)param3)->value);
            } else
                printf(template, ((TestLogBool *)param1)->value, ((TestLogInt *)param2)->value, ((TestLogBool *)param3)->value);
        
        } else if (param2->type == T_BOOL) {

            if (param3->type == T_INT) {
                // T_FLOAT, T_BOOL, T_INT
                printf(template, ((TestLogFloat *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogInt *)param3)->value);
        
            } else if (param3->type == T_STR) {
                // T_FLOAT, T_BOOL, T_STR
                printf(template, ((TestLogFloat *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogStr *)param3)->value);
            } else
                printf(template, ((TestLogBool *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogBool *)param3)->value);

        } else if (param2->type == T_STR) {

            if (param3->type == T_INT) {
                // T_FLOAT, T_STR, T_INT
                printf(template, ((TestLogFloat *)param1)->value, ((TestLogStr *)param2)->value, ((TestLogInt *)param3)->value);
        
            } else if (param3->type == T_BOOL) {
                // T_FLOAT, T_STR, T_BOOL
                printf(template, ((TestLogFloat *)param1)->value, ((TestLogStr *)param2)->value, ((TestLogBool *)param3)->value);
            } else {
                printf(template, ((TestLogFloat *)param1)->value, ((TestLogStr *)param2)->value, ((TestLogBool *)param3)->value);
            }
        } else {
            printf(template, ((TestLogFloat *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogBool *)param3)->value);
        }

    } else if (param1->type == T_STR) {

        if (param2->type == T_INT) {

            if (param3->type == T_FLOAT) {
                // T_STR, T_INT, T_FLOAT
                printf(template, ((TestLogStr *)param1)->value, ((TestLogInt *)param2)->value, ((TestLogFloat *)param3)->value);
        
            } else if (param3->type == T_BOOL) {
                // T_STR, T_INT, T_BOOL
                printf(template, ((TestLogStr *)param1)->value, ((TestLogInt *)param2)->value, ((TestLogBool *)param3)->value);
            } else
                printf(template, ((TestLogBool *)param1)->value, ((TestLogInt *)param2)->value, ((TestLogBool *)param3)->value);
        
        } else if (param2->type == T_FLOAT) {

            if (param3->type == T_INT) {
                // T_STR, T_FLOAT, T_INT
                printf(template, ((TestLogStr *)param1)->value, ((TestLogFloat *)param2)->value, ((TestLogInt *)param3)->value);
        
            } else if (param3->type == T_BOOL) {
                // T_STR, T_FLOAT, T_BOOL
                printf(template, ((TestLogStr *)param1)->value, ((TestLogFloat *)param2)->value, ((TestLogBool *)param3)->value);
            } else
                printf(template, ((TestLogBool *)param1)->value, ((TestLogFloat *)param2)->value, ((TestLogBool *)param3)->value);

        } else if (param2->type == T_BOOL) {

            if (param3->type == T_INT) {
                // T_STR, T_BOOL, T_INT
                printf(template, ((TestLogStr *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogInt *)param3)->value);
        
            } else if (param3->type == T_FLOAT) {
                // T_STR, T_BOOL, T_FLOAT
                printf(template, ((TestLogStr *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogFloat *)param3)->value);
            } else {
                printf(template, ((TestLogStr *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogBool *)param3)->value);
            }
        } else {
            printf(template, ((TestLogStr *)param1)->value, ((TestLogStr *)param2)->value, ((TestLogBool *)param3)->value);
        }
    }

    if (shouldFree1)
        free(param1);
    if (shouldFree2)
        free(param2);
    if (shouldFree3)
        free(param3);
}

// void ___tstDebugStep(const char* msgOrTemplate, TestLog *param1, TestLog *param2) {

//     if (!DEBUG_TEST_ENABLE)
// 		return;

//     // Open space for the dummies
//     const bool shouldFree1 = param1 == NULL;
//     const bool shouldFree2 = param2 == NULL;

//     int dummiesCount = 0;
//     if (shouldFree1) {
//         param1 = (TestLog *)malloc(sizeof(TestLogBool));
//         param1 = tstBool(false);
//         dummiesCount++;
//     }

//     if (shouldFree2) {
//         param2 = (TestLog *)malloc(sizeof(TestLogBool));
//         param2 = tstBool(false);
//         dummiesCount++;
//     }

//     char dummy[] = " _(%d)";
//     const int dummySize = strlen(dummy);
//     char *template = (char *)malloc(strlen(msgOrTemplate) + dummiesCount*dummySize + 1);
//     strcpy(template, msgOrTemplate);

//     // Include dummies as placeholders
//     if (dummiesCount) {
//         for (int i = 0; i < dummiesCount; i++)
//             strcat(template, dummy);
//     }

//     printf("\n[log: test] ");

//     if (param1->type == T_BOOL) {

//         if (param2->type == T_INT) {
//             // T_BOOL, T_INT, T_FLOAT
//             printf(template, ((TestLogBool *)param1)->value, ((TestLogInt *)param2)->value);
        
//         } else if (param2->type == T_FLOAT) {
//             // T_BOOL, T_FLOAT
//             printf(template, ((TestLogBool *)param1)->value, ((TestLogFloat *)param2)->value);

//         } else if (param2->type == T_STR) {
//             // T_BOOL, T_STR
//             printf(template, ((TestLogBool *)param1)->value, ((TestLogStr *)param2)->value);
//         } else {
//             printf(template, ((TestLogBool *)param1)->value, ((TestLogBool *)param2)->value);
//         }

//     } else if (param1->type == T_INT) {

//         if (param2->type == T_BOOL) {
//             // T_INT, T_BOOL
//             printf(template, ((TestLogInt *)param1)->value, ((TestLogBool *)param2)->value);
        
//         } else if (param2->type == T_FLOAT) {
//             // T_INT, T_FLOAT
//             printf(template, ((TestLogInt *)param1)->value, ((TestLogFloat *)param2)->value);

//         } else if (param2->type == T_STR) {
//             // T_INT, T_STR
//             printf(template, ((TestLogInt *)param1)->value, ((TestLogStr *)param2)->value);
//         } else {
//             printf(template, ((TestLogInt *)param1)->value, ((TestLogInt *)param2)->value);
//         }

//     } else if (param1->type == T_FLOAT) {

//         if (param2->type == T_INT) {
//             // T_FLOAT, T_INT
//             printf(template, ((TestLogFloat *)param1)->value, ((TestLogInt *)param2)->value);
        
//         } else if (param2->type == T_BOOL) {
//             // T_FLOAT, T_BOOL
//             printf(template, ((TestLogFloat *)param1)->value, ((TestLogBool *)param2)->value);

//         } else if (param2->type == T_STR) {
//             // T_FLOAT, T_STR
//             printf(template, ((TestLogFloat *)param1)->value, ((TestLogStr *)param2)->value);
//         } else {
//             printf(template, ((TestLogFloat *)param1)->value, ((TestLogFloat *)param2)->value);
//         }

//     } else if (param1->type == T_STR) {

//         if (param2->type == T_INT) {
//             // T_STR, T_INT
//             printf(template, ((TestLogStr *)param1)->value, ((TestLogInt *)param2)->value);
        
//         } else if (param2->type == T_FLOAT) {
//             // T_STR, T_FLOAT
//             printf(template, ((TestLogStr *)param1)->value, ((TestLogFloat *)param2)->value);

//         } else if (param2->type == T_BOOL) {
//             // T_STR, T_BOOL, T_INT
//             printf(template, ((TestLogStr *)param1)->value, ((TestLogBool *)param2)->value);
//         } else {
//             printf(template, ((TestLogStr *)param1)->value, ((TestLogStr *)param2)->value);
//         }
//     }

//     if (shouldFree1)
//         free(param1);
//     if (shouldFree2)
//         free(param2);
// }



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
// printf(template, ((TestLogBool *)param1)->value, ((TestLogInt *)param2)->value, ((TestLogFloat *)param3)->value);
// // T_BOOL, T_INT, T_STR
// printf(template, ((TestLogBool *)param1)->value, ((TestLogInt *)param2)->value, ((TestLogStr *)param3)->value);
// // T_BOOL, T_FLOAT, T_INT
// printf(template, ((TestLogBool *)param1)->value, ((TestLogFloat *)param2)->value, ((TestLogInt *)param3)->value);
// // T_BOOL, T_FLOAT, T_STR
// printf(template, ((TestLogBool *)param1)->value, ((TestLogFloat *)param2)->value, ((TestLogStr *)param3)->value);
// // T_BOOL, T_STR, T_INT
// printf(template, ((TestLogBool *)param1)->value, ((TestLogStr *)param2)->value, ((TestLogInt *)param3)->value);
// // T_BOOL, T_STR, T_FLOAT
// printf(template, ((TestLogBool *)param1)->value, ((TestLogStr *)param2)->value, ((TestLogFloat *)param3)->value);

// // T_INT, T_BOOL, T_FLOAT
// printf(template, ((TestLogInt *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogFloat *)param3)->value);
// // T_INT, T_BOOL, T_STR
// printf(template, ((TestLogInt *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogStr *)param3)->value);
// // T_INT, T_FLOAT, T_BOOL
// printf(template, ((TestLogInt *)param1)->value, ((TestLogFloat *)param2)->value, ((TestLogBool *)param3)->value);
// // T_INT, T_FLOAT, T_STR
// printf(template, ((TestLogInt *)param1)->value, ((TestLogFloat *)param2)->value, ((TestLogStr *)param3)->value);
// // T_INT, T_STR, T_BOOL
// printf(template, ((TestLogInt *)param1)->value, ((TestLogStr *)param2)->value, ((TestLogBool *)param3)->value);
// // T_INT, T_STR, T_FLOAT
// printf(template, ((TestLogInt *)param1)->value, ((TestLogStr *)param2)->value, ((TestLogFloat *)param3)->value);

// // T_FLOAT, T_INT, T_BOOL
// printf(template, ((TestLogFloat *)param1)->value, ((TestLogInt *)param2)->value, ((TestLogBool *)param3)->value);
// // T_FLOAT, T_INT, T_STR
// printf(template, ((TestLogFloat *)param1)->value, ((TestLogInt *)param2)->value, ((TestLogStr *)param3)->value);
// // T_FLOAT, T_BOOL, T_INT
// printf(template, ((TestLogFloat *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogInt *)param3)->value);
// // T_FLOAT, T_BOOL, T_STR
// printf(template, ((TestLogFloat *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogStr *)param3)->value);
// // T_FLOAT, T_STR, T_INT
// printf(template, ((TestLogFloat *)param1)->value, ((TestLogStr *)param2)->value, ((TestLogInt *)param3)->value);
// // T_FLOAT, T_STR, T_BOOL
// printf(template, ((TestLogFloat *)param1)->value, ((TestLogStr *)param2)->value, ((TestLogBool *)param3)->value);

// // T_STR, T_INT, T_FLOAT
// printf(template, ((TestLogStr *)param1)->value, ((TestLogInt *)param2)->value, ((TestLogFloat *)param3)->value);
// // T_STR, T_INT, T_BOOL
// printf(template, ((TestLogStr *)param1)->value, ((TestLogInt *)param2)->value, ((TestLogBool *)param3)->value);
// // T_STR, T_FLOAT, T_INT
// printf(template, ((TestLogStr *)param1)->value, ((TestLogFloat *)param2)->value, ((TestLogInt *)param3)->value);
// // T_STR, T_FLOAT, T_BOOL
// printf(template, ((TestLogStr *)param1)->value, ((TestLogFloat *)param2)->value, ((TestLogBool *)param3)->value);
// // T_STR, T_BOOL, T_INT
// printf(template, ((TestLogStr *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogInt *)param3)->value);
// // T_STR, T_BOOL, T_FLOAT
// printf(template, ((TestLogStr *)param1)->value, ((TestLogBool *)param2)->value, ((TestLogFloat *)param3)->value);