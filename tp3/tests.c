#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <regex.h>
#include <string.h>

#include "common.h"

typedef struct {
    char inputTxt[100];
    Command cmd;
    bool isVerbose;
} CmdTest;

/**
 * ------------------------------------------------
 * == HELPERS =====================================
 * ------------------------------------------------
 */

bool runOneTest(CmdTest test) {

    printf("\n\n Testing command: '%s'...", test.inputTxt);

    Command cmd = getCommand(test.inputTxt);
    
    if (test.isVerbose) {
        
        printf("\n\t-- What came: -------------------");
        printf("\n\tcmd.isValid: %d", cmd.isValid);
        printf("\n\tcmd.code: %d", cmd.code);
        printf("\n\tcmd.equipment: '%s'", cmd.equipment);
        printf("\n\tcmd.name: '%s'", cmd.name);
        printf("\n\tcmd.sensors[0]: '%d'", cmd.sensors[0]);
        printf("\n\tcmd.sensors[1]: '%d'", cmd.sensors[1]);
        printf("\n\tcmd.sensors[2]: '%d'", cmd.sensors[2]);
        printf("\n\tcmd.sensors[3]: '%d'", cmd.sensors[3]);

        printf("\n\t-- What was supposed to come: ---");
        printf("\n\ttest.cmd.isValid: %d", test.cmd.isValid);
        printf("\n\ttest.cmd.code: %d", test.cmd.code);
        printf("\n\ttest.cmd.equipment: '%s'", test.cmd.equipment);
        printf("\n\ttest.cmd.name: '%s'", test.cmd.name);
        printf("\n\ttest.cmd.sensors[0]: '%d'", test.cmd.sensors[0]);
        printf("\n\ttest.cmd.sensors[1]: '%d'", test.cmd.sensors[1]);
        printf("\n\ttest.cmd.sensors[2]: '%d'", test.cmd.sensors[2]);
        printf("\n\ttest.cmd.sensors[3]: '%d'", test.cmd.sensors[3]);
    }

    bool isSuccess = cmd.isValid == test.cmd.isValid;
    if (isSuccess && test.cmd.isValid) {
        isSuccess = (
            cmd.code == test.cmd.code
            && strcmp(cmd.equipment, test.cmd.equipment) == 0
            && strcmp(cmd.name, test.cmd.name) == 0
            && cmd.sensors[0] == test.cmd.sensors[0]
            && cmd.sensors[1] == test.cmd.sensors[1]
            && cmd.sensors[2] == test.cmd.sensors[2]
            && cmd.sensors[3] == test.cmd.sensors[3]
        );
    }

    if (isSuccess)
        printf("\n> OK!");
    else
        printf("\n> Not OK!");

    return isSuccess;
}

bool runBulkTest(const char* title, CmdTest tests[], const int nTests, const bool isValid) {

    int failureCount = 0;
    printf("\n\n----- New Test: %s [%s examples] ------------", title, isValid ? "valid" : "invalid");

    for (int i = 0; i < nTests; i++) {
        
        CmdTest test = tests[i];
        test.cmd.isValid = isValid;
        bool isSuccess = runOneTest(test);
        if (!isSuccess)
            failureCount++;
    }

    if (!failureCount)
        printf("\n-------------------- ALL TESTS PASSED! --------------\n");
    else
        printf("\n-------------------- %d TESTS FAILED ---------------\n", failureCount);

    return !failureCount;
}

/**
 * ------------------------------------------------
 * == TEST ========================================
 * ------------------------------------------------
 */

void testCommandAdd(void) {

    /** == VALID Tests ========== */

    int n = -1;
    int testsCount = 4;
    bool isVerbose = false;
    CmdTest validTests[testsCount];


    // New test...
    n++;
    validTests[n].isVerbose = isVerbose;
    strcpy(validTests[n].inputTxt, "add sensor 01 in 01");

    validTests[n].cmd = getEmptyCommand(CMD_CODE_ADD);
    strcpy(validTests[n].cmd.equipment, "01");
    validTests[n].cmd.sensors[0] = true;

    // New test...
    n++;
    validTests[n].isVerbose = isVerbose;
    strcpy(validTests[n].inputTxt, "add sensor 01 02 in 02");

    validTests[n].cmd = getEmptyCommand(CMD_CODE_ADD);
    strcpy(validTests[n].cmd.equipment, "02");
    validTests[n].cmd.sensors[0] = true;
    validTests[n].cmd.sensors[1] = true;

    // New test...
    n++;
    validTests[n].isVerbose = isVerbose;
    strcpy(validTests[n].inputTxt, "add sensor 01 02 03 in 03");

    validTests[n].cmd = getEmptyCommand(CMD_CODE_ADD);
    strcpy(validTests[n].cmd.equipment, "03");
    validTests[n].cmd.sensors[0] = true;
    validTests[n].cmd.sensors[1] = true;
    validTests[n].cmd.sensors[2] = true;

    // New test...
    n++;
    validTests[n].isVerbose = isVerbose;
    strcpy(validTests[n].inputTxt, "add sensor 01 02 03 04 in 04");

    validTests[n].cmd = getEmptyCommand(CMD_CODE_ADD);
    strcpy(validTests[n].cmd.equipment, "04");
    validTests[n].cmd.sensors[0] = true;
    validTests[n].cmd.sensors[1] = true;
    validTests[n].cmd.sensors[2] = true;
    validTests[n].cmd.sensors[3] = true;

    runBulkTest("ADD", validTests, testsCount, true);

    /** == INVALID Tests ======== */

    n = -1;
    testsCount = 13;
    // isVerbose = true;
    CmdTest invalidTests[testsCount];

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "add sensor 00 in 01");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_ADD);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "add sensor 05 in 01");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_ADD);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "add sensor 01 in 00");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_ADD);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "add sensor 01 in 05");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_ADD);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "add sensor 01 01 in 02");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_ADD);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "add sensor 02 02 03 in 03");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_ADD);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "add sensor 03 04 04 in 04");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_ADD);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "add sensor 01 02 03 02 in 03");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_ADD);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "add 01 in 01");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_ADD);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "ad sensor 02 in 03");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_ADD);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "add sesnor 03 in 04");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_ADD);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "addsensor 04 in 02");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_ADD);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "add sensor 01 02 03 04 in 03 01");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_ADD);

    runBulkTest("ADD", invalidTests, testsCount, false);
}

// void testCommandAdd(void);
// void testCommandRemove(void);
// void testCommandList(void);
// void testCommandRead(void);
// void testCommandKill(void);

int main() {
    testCommandAdd();
    exit(EXIT_SUCCESS);
}
