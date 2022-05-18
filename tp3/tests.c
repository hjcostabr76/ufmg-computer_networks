#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <regex.h>
#include <string.h>

const char CMD_PATTERN_ADD[] = "^add sensor (0[1234] ){1,4}in 0[1234]$";

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
    printf("\n\nregStatus: %d", regStatus);
    
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


void testCmd(const char* title, const char* pattern, const char testStrings[][100], const int testStringsCount, const bool isTestValid) {

    bool isSuccess = true;
    printf("\n----- New Test: %s [%s examples] ----------\n", title, isTestValid ? "valid" : "invalid");

    for (int i = 0; i < testStringsCount; i++) {
        printf("\nTesting string \"%s\"...", testStrings[i]);

        char msg[100];
        bool isMatch = comRegexMatch(pattern, testStrings[i], msg);
        isSuccess = isSuccess && ((isTestValid && isMatch) || (!isTestValid && !isMatch));

        if (isMatch)
            printf("\n\tOK...");
        else if (!strlen(msg))
            printf("\n\tNot OK!");
        else
            printf("\n\tError! \"%s\" ", msg);
    }

    if (isSuccess)
        printf("\n-------------------- TEST PASSED --------------------\n");
    else
        printf("\n-------------------- TEST FAILED --------------------\n");
}

void testCmdAdd(void) {

    // Test valid examples
    const char cmdTestAddvValid[][100] = {
        {"add sensor 01 in 02"},
        {"add sensor 01 02 in 02"},
        {"add sensor 01 02 03 in 02"},
        {"add sensor 01 02 03 04 in 02"}
    };

    testCmd("ADD", CMD_PATTERN_ADD, cmdTestAddvValid, 4, true);
}

int main() {
    testCmdAdd();
    exit(EXIT_SUCCESS);
}
