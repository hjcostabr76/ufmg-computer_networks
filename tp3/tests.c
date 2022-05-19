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
const char CMD_PATTERN_END[] = "in ##";

/**
 * ------------------------------------------------
 * == ABSTRACTS ===================================
 * ------------------------------------------------
 */

typedef struct {
    bool isValid;
    CmmdCodeEnum code;
    char* name; // TODO: Is it really necessary?
    char equipment[2] ;
    bool sensors[SENSOR_COUNT];
} Command;


typedef struct {
    char inputTxt[100];
    Command cmd;
} CmdTest;

/**
 * ------------------------------------------------
 * == HEADERS =====================================
 * ------------------------------------------------
 */

bool comRegexMatch(const char* pattern, const char* str, char errorMsg[100]);
bool comStrStartsWith(const char *target, const char *prefix);
char* comStrGetSubstring(const char *src, char *dst, size_t start, size_t end);
void strSplit(const char* source, char** dest, const char delimiter, int *tokensCount);
Command getCommand();

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
        cmd.isValid = comRegexMatch(CMD_PATTERN[i], input, regexMsg);
        if (!cmd.isValid)
            continue;

		cmd.isValid = true;
		cmd.code = i;
		strcpy(cmd.name, CMD_NAME[i]);
        break;
	}

    // Check if is there anything else to parse
	if (!cmd.isValid || strcmp(cmd.name, CMD_NAME[CMD_CODE_KILL]) == 0)
        return cmd;

    // Determine equipment
    int eqIdLength = 2;
    int inputLength = strlen(input);
    comStrGetSubstring(input, cmd.equipment, inputLength - eqIdLength, inputLength);

    // Check if is there sensors to identify
	if (strcmp(cmd.name, CMD_NAME[CMD_CODE_LIST]) == 0)
        return cmd;
    
    // Determine Sensors
    char sensorsListStr[inputLength];
    int sensorsChar1 = strlen(cmd.name);
    int sensorsCharLast = inputLength - strlen(CMD_PATTERN_END);
    comStrGetSubstring(input, sensorsListStr, sensorsChar1, sensorsCharLast);

    int inputSensorsCount;
    char sensorIds[4][2];
    strSplit(sensorsListStr, sensorIds, " ", &inputSensorsCount);
    
    for (int i = 0; i < SENSOR_COUNT; i++)
        cmd.sensors[i] = false;

    for (int i = 0; i < inputSensorsCount; i++) {
        int sensorCode = atoi(sensorIds[i]);
        cmd.sensors[sensorCode] = true;
    }
    
    return cmd;
}

bool comRegexMatch(const char* pattern, const char* str, char errorMsg[100]) {

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

bool comStrStartsWith(const char *target, const char *prefix) {
   return strncmp(target, prefix, strlen(prefix)) == 0;
}

char* comStrGetSubstring(const char *src, char *dst, size_t start, size_t end) {
    return strncpy(dst, src + start, end);
}


void strSplit(const char* source, char** dest, const char delimiter, int *tokensCount) {
    *tokensCount = 0;
    
    while (true) {
        char *token = strtok(source, delimiter);
        if (token == NULL)
            break;
        *tokensCount++;
        strcpy(dest[*tokensCount], token);
    }
}

/**
 * ------------------------------------------------
 * == TEST ========================================
 * ------------------------------------------------
 */

// void testPattern(const char* title, const char* pattern, const char testStrings[][100], const int strCount, const bool isTestValid) {

//     bool isSuccess = true;
//     printf("\n----- New Test: %s [%s examples] ----------\n", title, isTestValid ? "valid" : "invalid");

//     for (int i = 0; i < strCount; i++) {
//         printf("\nTesting string \"%s\"...", testStrings[i]);

//         char msg[100];
//         bool isMatch = comRegexMatch(pattern, testStrings[i], msg);
//         isSuccess = isSuccess && ((isTestValid && isMatch) || (!isTestValid && !isMatch));

//         if (isMatch)
//             printf("\n\tOK...");
//         else if (!strlen(msg))
//             printf("\n\tNot OK!");
//         else
//             printf("\n\tError! \"%s\" ", msg);
//     }

//     if (isSuccess)
//         printf("\n-------------------- TEST PASSED --------------------\n");
//     else
//         printf("\n-------------------- TEST FAILED --------------------\n");
// }

void testCommandAdd(void) {

    // Test valid examples
    // const char cmdTestAddvValid[][100] = {
    //     {"add sensor 01 in 01"},
    //     {"add sensor 01 02 in 02"},
    //     {"add sensor 01 02 03 in 03"},
    //     {"add sensor 01 02 03 04 in 04"}
    // };

    int testsCount = 1;
    CmdTest validTests[testsCount];
    strcpy(validTests[0].inputTxt, "add sensor 01 in 01");
    
    validTests[0].cmd.code = CMD_CODE_ADD;
    strcpy(validTests[0].cmd.equipment, "01");

    for (int i = 0; i < SENSOR_COUNT; i++)
        validTests[0].cmd.sensors[i] = false;
    validTests[0].cmd.sensors[0] = true;

    for (int i = 0; i < testsCount; i++) {
        
        CmdTest test = validTests[i];
        test.cmd.isValid = true;
        test.cmd.name = CMD_NAME[i];
        strcpy(test.cmd.name, CMD_NAME[i]);

        Command cmd = getCommand(test.inputTxt);

        bool passed = (
            cmd.isValid
            && cmd.code == test.cmd.code
            && strcmp(cmd.equipment, test.cmd.equipment) == 0
            && strcmp(cmd.name, test.cmd.name) == 0
            && cmd.sensors[0] == test.cmd.sensors[0]
            && cmd.sensors[1] == test.cmd.sensors[1]
            && cmd.sensors[2] == test.cmd.sensors[2]
            && cmd.sensors[3] == test.cmd.sensors[3]
        );

        if (passed)
            printf("\n-------------------- TEST PASSED --------------------\n");
        else
            printf("\n-------------------- TEST FAILED --------------------\n");
    }

    // int strCount = sizeof(cmdTestAddvValid) / sizeof(cmdTestAddvValid[0]);
    // testPattern("ADD", CMD_PATTERN[CMD_CODE_ADD], cmdTestAddvValid, strCount, true);

    // // Test invalid examples
    // const char cmdTestAddvInvalid[][100] = {
    //     {"add sensor 05 in 02"},
    //     {"add sensor 01 in 05"},
    //     {"add sensor 01 01 in 02"},
    //     {"add sensor 01 02 03 04 04 in 02"},
    //     {"add sensor 01 02 in 01 02"},
    //     {"add sensor in 01"},
    //     {"add sensor 01"},
    //     {"add 01 in 01"},
    //     {"sensor in 01"}
    // };

    // strCount = sizeof(cmdTestAddvInvalid) / sizeof(cmdTestAddvInvalid[0]);
    // testPattern("ADD", CMD_PATTERN[CMD_CODE_ADD], cmdTestAddvInvalid, strCount, false);
}

int main() {
    testCommandAdd();
    exit(EXIT_SUCCESS);
}
