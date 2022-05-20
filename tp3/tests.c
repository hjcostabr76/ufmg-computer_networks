#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <regex.h>
#include <string.h>

/**
 * ------------------------------------------------
 * == CONSTS ======================================
 * ------------------------------------------------
 */

#define CMD_COUNT 5
#define BUF_SIZE 500
#define SENSOR_COUNT 4

const char EQP_IDS[4][2] = { {"01"}, {"02"}, {"03"}, {"04"} };

typedef enum { CMD_CODE_ADD, CMD_CODE_RM, CMD_CODE_LIST, CMD_CODE_READ, CMD_CODE_KILL } CmmdCodeEnum;
const char CMD_NAME[CMD_COUNT][15] = { {"add sensor"}, {"remove sensor"}, {"list sensors"}, {"read"}, {"kill"} }; // TODO: Do we really need this?
const char CMD_PATTERN[CMD_COUNT][45] = {
    {"^add sensor (0[1234] ){1,4}in 0[1234]$"},
    {"^remove sensor (0[1234] ){1,4}in 0[1234]$"},
    {"^list sensors in 0[1234]$"},
    {"^read (0[1234] ){1,4}in 0[1234]$"},
    {"^kill$"}
};
const char CMD_PATTERN_END[] = " in ##";

/**
 * ------------------------------------------------
 * == ABSTRACTS ===================================
 * ------------------------------------------------
 */

typedef struct {
    bool isValid;
    CmmdCodeEnum code;
    char name[15]; // TODO: Is it really necessary?
    char equipment[3];
    bool sensors[SENSOR_COUNT];
} Command;


typedef struct {
    char inputTxt[100];
    Command cmd;
    bool isVerbose;
} CmdTest;

/**
 * ------------------------------------------------
 * == HEADERS =====================================
 * ------------------------------------------------
 */

bool regexMatch(const char* pattern, const char* str, char errorMsg[100]);
bool strStartsWith(const char *target, const char *prefix);
void strGetSubstring(const char *src, char *dst, size_t start, size_t end);
char** strSplit(char* source, const char delimiter[1], const int maxTokens, const int maxLength, int *tokensCount);
Command getGenericCommand(void);
Command getEmptyCommand(CmmdCodeEnum code);

bool runBulkTest(const char* title, CmdTest tests[], const int nTests, const bool isValid);
bool runOneTest(CmdTest test);
void testCommandAdd(void);

/**
 * ------------------------------------------------
 * == MAIN ========================================
 * ------------------------------------------------
 */

Command getCommand(const char* input) {

    Command cmd;
    cmd.isValid = false;
    
    // Identify command type
    for (int i = 0; i < CMD_COUNT; i++) {

        char regexMsg[100];
        cmd.isValid = regexMatch(CMD_PATTERN[i], input, regexMsg);
        if (!cmd.isValid)
            continue;

		cmd.isValid = true;
		cmd.code = i;
		strcpy(cmd.name, CMD_NAME[i]);
        break;
	}

	if (!cmd.isValid || cmd.code == CMD_CODE_KILL)
        return cmd;

    // Determine equipment

    char inputCopy[100];
    strcpy(inputCopy, input);

    int inputArgsC;
    char** inputArgs = strSplit(inputCopy, " ", 8, 8, &inputArgsC);
    strcpy(cmd.equipment, inputArgs[inputArgsC - 1]);

    if (cmd.code == CMD_CODE_LIST)
        return cmd;

    // Determine sensors    
    for (int i = 0; i < SENSOR_COUNT; i++)
        cmd.sensors[i] = false;

    int firstSensorIdx = cmd.code == CMD_CODE_READ ? 1 : 2;
    for (int i = firstSensorIdx; i < inputArgsC - 2; i++) {
        int sensorCode = atoi(inputArgs[i]) - 1;
        if (cmd.sensors[sensorCode]) {
            cmd.isValid = false;
            break;
        }
        cmd.sensors[sensorCode] = true;
    }
    
    return cmd;
}

bool regexMatch(const char* pattern, const char* str, char errorMsg[100]) {

    regex_t regex;
    memset(errorMsg, 0, 100);
    
    // Compile
    int regStatus = regcomp(&regex, pattern, REG_EXTENDED);
    if (regStatus != 0) {
        sprintf(errorMsg, "Compiling error");
        return false;
    }

    // Execute
    regStatus = regexec(&regex, str, 0, NULL, 0);
    
    bool isSuccess = regStatus == 0;
    if (!isSuccess && regStatus != REG_NOMATCH) { // Error
        char aux[100];
        regerror(regStatus, &regex, aux, 100);
        sprintf(errorMsg, "Match error: %s\n", aux);
    }

    // Free memory allocated to the pattern buffer by regcomp()
    regfree(&regex);
    return isSuccess;
}

bool strStartsWith(const char *target, const char *prefix) {
   return strncmp(target, prefix, strlen(prefix)) == 0;
}

void strGetSubstring(const char *src, char *dst, size_t start, size_t end) {
    strncpy(dst, src + start, end - start);
}

char** strSplit(char* source, const char delimiter[1], const int maxTokens, const int maxLength, int *tokensCount) {
    
    *tokensCount = 0;
    char** tokens = malloc(maxTokens * sizeof(char*));
    
    char *token = strtok(source, delimiter);
    if (token == NULL)
        return tokens;

    while (token != NULL && *tokensCount < maxTokens) {

        tokens[*tokensCount] = malloc(maxLength * sizeof(char));
        memset(tokens[*tokensCount], '\n', maxLength * sizeof(char));
        strcpy(tokens[*tokensCount], token);
        
        *tokensCount = *tokensCount + 1;
        token = strtok(NULL, delimiter);
    }

    return tokens;
}

/**
 * ------------------------------------------------
 * == TEST ========================================
 * ------------------------------------------------
 */

Command getGenericCommand(void) {
    Command command;
    command.isValid = false;
    memset(command.name, '\0', sizeof(command.name));
    memset(command.equipment, '\0', sizeof(command.equipment));
    for (int i = 0; i < SENSOR_COUNT; i++)
        command.sensors[i] = false;
    return command;
}

Command getEmptyCommand(CmmdCodeEnum code) {
    Command command = getGenericCommand();
    command.code = code;
    strcpy(command.name, CMD_NAME[code]);
    return command;
}


void testCommandAdd(void) {

    int n = -1;
    int testsCount = 4;
    bool isVerbose = false;
    CmdTest validTests[testsCount];

    /**
     * VALID Tests...
     */

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

    /**
     * INVALID Tests...
     */

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

int main() {
    testCommandAdd();
    exit(EXIT_SUCCESS);
}
