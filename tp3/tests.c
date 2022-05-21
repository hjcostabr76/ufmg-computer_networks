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
    
    bool isSuccess = cmd.isValid == test.cmd.isValid;
    if (isSuccess && test.cmd.isValid) {
        isSuccess = (
            cmd.code == test.cmd.code
            && cmd.equipCode == test.cmd.equipCode
            && cmd.sensors[0] == test.cmd.sensors[0]
            && cmd.sensors[1] == test.cmd.sensors[1]
            && cmd.sensors[2] == test.cmd.sensors[2]
            && cmd.sensors[3] == test.cmd.sensors[3]
        );
    }

    if (test.isVerbose || !isSuccess) {

        printf("\n\t-- What came: -------------------");
        printf("\n\tcmd.isValid: %d", cmd.isValid);
        printf("\n\tcmd.code: %d", cmd.code);
        printf("\n\tcmd.equipment: '%d'", cmd.equipCode);
        printf("\n\tcmd.sensors[0]: '%d'", cmd.sensors[0]);
        printf("\n\tcmd.sensors[1]: '%d'", cmd.sensors[1]);
        printf("\n\tcmd.sensors[2]: '%d'", cmd.sensors[2]);
        printf("\n\tcmd.sensors[3]: '%d'", cmd.sensors[3]);

        printf("\n\t-- What was supposed to come: ---");
        printf("\n\ttest.cmd.isValid: %d", test.cmd.isValid);
        printf("\n\ttest.cmd.code: %d", test.cmd.code);
        printf("\n\ttest.cmd.equipment: '%d'", test.cmd.equipCode);
        printf("\n\ttest.cmd.sensors[0]: '%d'", test.cmd.sensors[0]);
        printf("\n\ttest.cmd.sensors[1]: '%d'", test.cmd.sensors[1]);
        printf("\n\ttest.cmd.sensors[2]: '%d'", test.cmd.sensors[2]);
        printf("\n\ttest.cmd.sensors[3]: '%d'", test.cmd.sensors[3]);
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

void testCommandAdd() {

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
    validTests[n].cmd.equipCode = 0;
    validTests[n].cmd.sensors[0] = true;

    // New test...
    n++;
    validTests[n].isVerbose = isVerbose;
    strcpy(validTests[n].inputTxt, "add sensor 01 02 in 02");

    validTests[n].cmd = getEmptyCommand(CMD_CODE_ADD);
    validTests[n].cmd.equipCode = 1;
    validTests[n].cmd.sensors[0] = true;
    validTests[n].cmd.sensors[1] = true;

    // New test...
    n++;
    validTests[n].isVerbose = isVerbose;
    strcpy(validTests[n].inputTxt, "add sensor 01 02 03 in 03");

    validTests[n].cmd = getEmptyCommand(CMD_CODE_ADD);
    validTests[n].cmd.equipCode = 2;
    validTests[n].cmd.sensors[0] = true;
    validTests[n].cmd.sensors[1] = true;
    validTests[n].cmd.sensors[2] = true;

    // New test...
    n++;
    validTests[n].isVerbose = isVerbose;
    strcpy(validTests[n].inputTxt, "add sensor 01 02 03 04 in 04");

    validTests[n].cmd = getEmptyCommand(CMD_CODE_ADD);
    validTests[n].cmd.equipCode = 3;
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

void testCommandRemove() {

    /** == VALID Tests ========== */

    int n = -1;
    int testsCount = 4;
    bool isVerbose = false;
    CmdTest validTests[testsCount];

    // New test...
    n++;
    validTests[n].isVerbose = isVerbose;
    strcpy(validTests[n].inputTxt, "remove sensor 01 in 01");

    validTests[n].cmd = getEmptyCommand(CMD_CODE_RM);
    validTests[n].cmd.equipCode = 0;
    validTests[n].cmd.sensors[0] = true;

    // New test...
    n++;
    validTests[n].isVerbose = isVerbose;
    strcpy(validTests[n].inputTxt, "remove sensor 01 02 in 02");

    validTests[n].cmd = getEmptyCommand(CMD_CODE_RM);
    validTests[n].cmd.equipCode = 1;
    validTests[n].cmd.sensors[0] = true;
    validTests[n].cmd.sensors[1] = true;

    // New test...
    n++;
    validTests[n].isVerbose = isVerbose;
    strcpy(validTests[n].inputTxt, "remove sensor 01 02 03 in 03");

    validTests[n].cmd = getEmptyCommand(CMD_CODE_RM);
    validTests[n].cmd.equipCode = 2;
    validTests[n].cmd.sensors[0] = true;
    validTests[n].cmd.sensors[1] = true;
    validTests[n].cmd.sensors[2] = true;

    // New test...
    n++;
    validTests[n].isVerbose = isVerbose;
    strcpy(validTests[n].inputTxt, "remove sensor 01 02 03 04 in 04");

    validTests[n].cmd = getEmptyCommand(CMD_CODE_RM);
    validTests[n].cmd.equipCode = 3;
    validTests[n].cmd.sensors[0] = true;
    validTests[n].cmd.sensors[1] = true;
    validTests[n].cmd.sensors[2] = true;
    validTests[n].cmd.sensors[3] = true;

    runBulkTest("REMOVE", validTests, testsCount, true);

    /** == INVALID Tests ======== */

    n = -1;
    testsCount = 13;
    // isVerbose = true;
    CmdTest invalidTests[testsCount];

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "remove sensor 00 in 01");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_RM);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "remove sensor 05 in 01");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_RM);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "remove sensor 01 in 00");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_RM);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "remove sensor 01 in 05");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_RM);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "remove sensor 01 01 in 02");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_RM);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "remove sensor 02 02 03 in 03");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_RM);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "remove sensor 03 04 04 in 04");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_RM);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "remove sensor 01 02 03 02 in 03");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_RM);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "remove 01 in 01");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_RM);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "rmove sensor 02 in 03");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_RM);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "remove sesnor 03 in 04");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_RM);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "removesensor 04 in 02");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_RM);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "remove sensor 01 02 03 04 in 03 01");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_RM);

    runBulkTest("REMOVE", invalidTests, testsCount, false);
}

void testCommandRead() {

    /** == VALID Tests ========== */

    int n = -1;
    int testsCount = 4;
    bool isVerbose = false;
    CmdTest validTests[testsCount];

    // New test...
    n++;
    validTests[n].isVerbose = isVerbose;
    strcpy(validTests[n].inputTxt, "read 01 in 01");

    validTests[n].cmd = getEmptyCommand(CMD_CODE_READ);
    validTests[n].cmd.equipCode = 0;
    validTests[n].cmd.sensors[0] = true;

    // New test...
    n++;
    validTests[n].isVerbose = isVerbose;
    strcpy(validTests[n].inputTxt, "read 01 02 in 02");

    validTests[n].cmd = getEmptyCommand(CMD_CODE_READ);
    validTests[n].cmd.equipCode = 1;
    validTests[n].cmd.sensors[0] = true;
    validTests[n].cmd.sensors[1] = true;

    // New test...
    n++;
    validTests[n].isVerbose = isVerbose;
    strcpy(validTests[n].inputTxt, "read 01 02 03 in 03");

    validTests[n].cmd = getEmptyCommand(CMD_CODE_READ);
    validTests[n].cmd.equipCode = 2;
    validTests[n].cmd.sensors[0] = true;
    validTests[n].cmd.sensors[1] = true;
    validTests[n].cmd.sensors[2] = true;

    // New test...
    n++;
    validTests[n].isVerbose = isVerbose;
    strcpy(validTests[n].inputTxt, "read 01 02 03 04 in 04");

    validTests[n].cmd = getEmptyCommand(CMD_CODE_READ);
    validTests[n].cmd.equipCode = 3;
    validTests[n].cmd.sensors[0] = true;
    validTests[n].cmd.sensors[1] = true;
    validTests[n].cmd.sensors[2] = true;
    validTests[n].cmd.sensors[3] = true;

    runBulkTest("READ", validTests, testsCount, true);

    /** == INVALID Tests ======== */

    n = -1;
    testsCount = 13;
    // isVerbose = true;
    CmdTest invalidTests[testsCount];

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "read 00 in 01");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_READ);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "read 05 in 01");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_READ);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "read 01 in 00");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_READ);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "read 01 in 05");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_READ);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "read 01 01 in 02");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_READ);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "read 02 02 03 in 03");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_READ);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "read 03 04 04 in 04");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_READ);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "read 01 02 03 02 in 03");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_READ);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "read sensor 01 in 01");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_READ);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "rad sensor 02 in 03");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_READ);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "read sesnor 03 in 04");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_READ);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "readsensor 04 in 02");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_READ);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "read 01 02 03 04 in 03 01");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_READ);

    runBulkTest("READ", invalidTests, testsCount, false);
}

void testCommandList() {

    /** == VALID Tests ========== */

    int n = -1;
    int testsCount = 4;
    bool isVerbose = false;
    CmdTest validTests[testsCount];

    // New test...
    n++;
    validTests[n].isVerbose = isVerbose;
    strcpy(validTests[n].inputTxt, "list sensors in 01");

    validTests[n].cmd = getEmptyCommand(CMD_CODE_LIST);
    validTests[n].cmd.equipCode = 0;

    // New test...
    n++;
    validTests[n].isVerbose = isVerbose;
    strcpy(validTests[n].inputTxt, "list sensors in 02");

    validTests[n].cmd = getEmptyCommand(CMD_CODE_LIST);
    validTests[n].cmd.equipCode = 1;

    // New test...
    n++;
    validTests[n].isVerbose = isVerbose;
    strcpy(validTests[n].inputTxt, "list sensors in 03");

    validTests[n].cmd = getEmptyCommand(CMD_CODE_LIST);
    validTests[n].cmd.equipCode = 2;

    // New test...
    n++;
    validTests[n].isVerbose = isVerbose;
    strcpy(validTests[n].inputTxt, "list sensors in 04");

    validTests[n].cmd = getEmptyCommand(CMD_CODE_LIST);
    validTests[n].cmd.equipCode = 3;

    runBulkTest("LIST", validTests, testsCount, true);

    /** == INVALID Tests ======== */

    n = -1;
    testsCount = 10;
    // isVerbose = true;
    CmdTest invalidTests[testsCount];

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "list sensors in 00");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_LIST);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "list sensors in 05");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_LIST);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "list in 01");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_LIST);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "sensors in 02");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_LIST);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "list sensor in 03");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_LIST);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "lst sensors in 04");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_LIST);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "list sensors 01 in 01");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_LIST);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "list sensors 01 02 in 02");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_LIST);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "list sensors 01 02 03 in 03");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_LIST);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "list sensors 01 02 03 04 in 04");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_LIST);

    runBulkTest("LIST", invalidTests, testsCount, false);
}

void testCommandKill() {

    /** == VALID Tests ========== */

    int n = -1;
    int testsCount = 1;
    bool isVerbose = false;
    CmdTest validTests[testsCount];

    // New test...
    n++;
    validTests[n].isVerbose = isVerbose;
    strcpy(validTests[n].inputTxt, "kill");
    validTests[n].cmd = getEmptyCommand(CMD_CODE_KILL);

    runBulkTest("KILL", validTests, testsCount, true);

    /** == INVALID Tests ======== */

    n = -1;
    testsCount = 9;
    // isVerbose = true;
    CmdTest invalidTests[testsCount];

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "kil");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_KILL);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "kill sensor 01 in 01");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_KILL);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "kill sensor 01 02 in 02");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_KILL);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "kill sensor 01 02 03 in 03");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_KILL);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "kill sensor 01 02 03 04 in 04");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_KILL);
    
    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "kill sensors 01 in 01");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_KILL);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "kill sensors 01 02 in 02");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_KILL);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "kill sensors 01 02 03 in 03");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_KILL);

    // New test...
    n++;
    invalidTests[n].isVerbose = isVerbose;
    strcpy(invalidTests[n].inputTxt, "kill sensors 01 02 03 04 in 04");
    invalidTests[n].cmd = getEmptyCommand(CMD_CODE_KILL);

    runBulkTest("KILL", invalidTests, testsCount, false);
}

int main() {
    testCommandAdd();
    testCommandRead();
    testCommandRemove();
    testCommandList();
    testCommandKill();
    exit(EXIT_SUCCESS);
}
