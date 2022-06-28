#include "common.h"
#include "test_utils.h"
#include "test_runners.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    bool isOk;
    bool isValidTxt;
    bool isValidEmpty;
    bool isValidFloat;
    bool isValidInt;
    bool isValidIntList;
} PayloadFlags;

/* -------------------------------------------- */
/* -- Headers --------------------------------- */

void showExtractionResult(const int i, const Message result, const ExtractionTest test, const PayloadFlags payloadFlags);

/* -------------------------------------------- */
/* -- Functions ------------------------------- */

TestResult tstMsgPatternValidationBatch(const char **messages, const int nTests, const bool isValidMessage, const char *title) {

    char testType[15] = "";
    strcpy(testType, isValidMessage ? "Good\0" : "Bad\0");
    printf("\n\n----- New Test: %s ------------------------------", title);
    
    int nFailures = 0;
    for (int i = 0; i < nTests; i++) {
        
        printf("\nTesting %s pattern:\n\t\"%s\"...", testType, messages[i]);
        const bool passedValidation = isValidReceivedMsg(messages[i]);
        const bool isSuccess = (passedValidation && isValidMessage) || (!passedValidation && !isValidMessage);
        if (!isSuccess) {
            nFailures++;
        }
        
        char resultMsg[10];
        strcpy(resultMsg, isSuccess ? "OK" : "FAILED!");
        printf("\n\tTest %s", resultMsg);
    }

    printf("\n");

    if (!nFailures)
        printf("-------------------- ALL TESTS PASSED! ------------------\n");
    else
        printf("-------------------- %d TEST(S) FAILED --------------------\n", nFailures);
    
    const TestResult result = { nTests, nFailures };
    return result;
}

TestResult tstMsgExtractionBatch(const ExtractionTest tests[], const int nTests, const char *title) {
    printf("\n\n----- New Batch [%d tests]: %s ------------------------------", nTests, title);
    
    int nFailures = 0;
    for (int i = 0; i < nTests; i++) {
        
        const ExtractionTest test = tests[i];
        printf("\nTesting pattern: \"%s\"\n\t\"%s\"...", test.title, test.messageText);
        
        // Run extraction
        Message result;
        setMessageFromText(test.messageText, &result);
        tstDebugStep("tst: extraction [%d] 1", &i, NULL, NULL, NULL, NULL);
        
        // Validate result payload
        PayloadFlags payloadFlags = { false, false, false, false, false, false };

        payloadFlags.isValidTxt = (
            (test.expectedResult.payloadText == NULL && result.payloadText == NULL)
            || (
                test.expectedResult.payloadText != NULL
                && result.payloadText != NULL
                && strcmp(result.payloadText, test.expectedResult.payloadText) == 0
            )
        );
        tstDebugStep("tst: extraction [%d] 2 | %d", &i, comDbgBool(payloadFlags.isValidTxt), NULL, NULL, NULL);
        
        if (result.payload == NULL)
            payloadFlags.isValidEmpty = test.expectedResult.payload == NULL;
        
        else if (test.payloadDesc.isFloat)
            payloadFlags.isValidFloat = *(float *)result.payload == *(float *)test.expectedResult.payload;
        
        else if (test.payloadDesc.isInt)
            payloadFlags.isValidInt = *(int *)result.payload == *(int *)test.expectedResult.payload;
        
        else if (test.payloadDesc.isIntList) {
            for (int i = 0; i < test.payloadDesc.listLength; i++) {
                payloadFlags.isValidIntList = ((int *)result.payload)[i] == ((int *)test.expectedResult.payload)[i];
                if (!payloadFlags.isValidIntList)
                    break;
            }
        }
        
        payloadFlags.isOk = (
            payloadFlags.isValidTxt
            && (
                payloadFlags.isValidEmpty
                || payloadFlags.isValidFloat
                || payloadFlags.isValidInt
                || payloadFlags.isValidIntList
            )
        );

        // Validate the whole thing
        const bool isSuccess = (
            payloadFlags.isOk
            && result.isValid == test.expectedResult.isValid
            && result.id == test.expectedResult.id
            && result.source == test.expectedResult.source
            && result.target == test.expectedResult.target
        );
        if (!isSuccess)
            nFailures++;

        tstDebugStep(
            "tst: extraction [%d] 3.1 | payload.isOk: '%d', payload.isValidTxt: '%d'",
            &i, comDbgBool(payloadFlags.isOk), comDbgBool(payloadFlags.isValidTxt), NULL, NULL
        );
        tstDebugStep(
            "tst: extraction [%d] 3.2 | payload.isValidEmpty: '%d', payload.isValidFloat: '%d', payload.isValidInt: '%d', payloadFlags.isValidIntList: '%d'",
            &i, comDbgBool(payloadFlags.isValidEmpty), comDbgBool(payloadFlags.isValidFloat), comDbgBool(payloadFlags.isValidInt), comDbgBool(payloadFlags.isValidIntList)
        );

        // Exhibit result
        printf("\n[test %s]", isSuccess ? "ok" : "FAILED!");;
        const bool showDetails = test.isVerbose || !isSuccess;
        if (showDetails)
            showExtractionResult(i, result, test, payloadFlags);
    }
    tstDebugStep("tst: extraction -- the end --", NULL, NULL, NULL, NULL, NULL);
    
    printf("\n");

    if (!nFailures)
        printf("-------------------- ALL TESTS PASSED! ------------------\n");
    else
        printf("-------------------- %d TEST(S) FAILED --------------------\n", nFailures);
    
    const TestResult result = { nTests, nFailures };
    return result;
}

void showExtractionResult(const int i, const Message result, const ExtractionTest test, const PayloadFlags payloadFlags) {

    printf("\n\n\t--- What came: -------------------");
    printf("\n\tmessage.isValid: '%s'", comDbgBool(result.isValid));
    printf("\n\tmessage.id: '%d'", result.id);
    printf("\n\tmessage.source: '%d'", result.source);
    printf("\n\tmessage.target: '%d'", result.target);
    
    tstDebugStep("tst: extraction [%d] 5", &i, NULL, NULL, NULL, NULL);
    comDebugMessage(result, &test.payloadDesc);
    tstDebugStep("tst: extraction [%d] 6", &i, NULL, NULL, NULL, NULL);

    // Print expected header
    printf("\n\n\t--- What was supposed to come: ---");
    comDebugMessage(test.expectedResult, &test.payloadDesc);
    tstDebugStep("tst: extraction [%d] 7", &i, NULL, NULL, NULL, NULL);

    // Explain how payload is wrong
    if (!payloadFlags.isOk) {
        tstDebugStep("tst: extraction [%d] 8 (!isValidPayload...)", &i, NULL, NULL, NULL, NULL);

        printf("\n\n\t--- How did payload went bad: ----");
        printf("\n\tisValidPayloadTxt: '%s'", comDbgBool(payloadFlags.isValidTxt));
        printf("\n\tisValidEmptyPayload: '%s'", comDbgBool(payloadFlags.isValidEmpty));
        printf("\n\tisValidPayloadFloat: '%s'", comDbgBool(payloadFlags.isValidFloat));
        printf("\n\tisValidPayloadInt: '%s'", comDbgBool(payloadFlags.isValidInt));
        printf("\n\tisValidPayloadIntList: '%s'", comDbgBool(payloadFlags.isValidIntList));
    }
    
    tstDebugStep("tst: extraction [%d] 12", &i, NULL, NULL, NULL, NULL);
    printf("\n");
}
