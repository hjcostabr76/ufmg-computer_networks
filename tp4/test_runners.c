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
        tstDebugStep("tst: extraction [%d] 2 | %d", &i, tstBool(payloadFlags.isValidTxt), NULL, NULL, NULL);
        
        if (result.payload == NULL)
            payloadFlags.isValidEmpty = test.expectedResult.payload == NULL;
        
        else if (test.isPayloadFloat)
            payloadFlags.isValidFloat = *(float *)result.payload == *(float *)test.expectedResult.payload;
        
        else if (test.isPayloadInt)
            payloadFlags.isValidInt = *(int *)result.payload == *(int *)test.expectedResult.payload;
        
        else if (test.isPayloadIntList) {
            for (int i = 0; i < test.payloadListLength; i++) {
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
            &i, tstBool(payloadFlags.isOk), tstBool(payloadFlags.isValidTxt), NULL, NULL
        );
        tstDebugStep(
            "tst: extraction [%d] 3.2 | payload.isValidEmpty: '%d', payload.isValidFloat: '%d', payload.isValidInt: '%d', payloadFlags.isValidIntList: '%d'",
            &i, tstBool(payloadFlags.isValidEmpty), tstBool(payloadFlags.isValidFloat), tstBool(payloadFlags.isValidInt), tstBool(payloadFlags.isValidIntList)
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
    printf("\n\tmessage.isValid: '%s'", tstBool(result.isValid));
    printf("\n\tmessage.id: '%d'", result.id);
    printf("\n\tmessage.source: '%d'", result.source);
    printf("\n\tmessage.target: '%d'", result.target);
    
    tstDebugStep("tst: extraction [%d] 5", &i, NULL, NULL, NULL, NULL);
    
    if (result.payloadText == NULL)
            printf("\n\tmessage.payloadText: 'NULL'");
    else {
        printf("\n\tmessage.payloadText: '%s'", result.payloadText);
    }
    tstDebugStep("tst: extraction [%d] 6", &i, NULL, NULL, NULL, NULL);

    // Print actual payload
    if (result.payload == NULL) // Empty
        printf("\n\tmessage.payload: 'NULL'");

    else if (test.isPayloadFloat) // Float
        printf("\n\tmessage.payload: '%.2f'", *(float *)result.payload);

    else if (test.isPayloadInt) // Integer
        printf("\n\tmessage.payload: '%d'", *(int *)result.payload);

    else if (test.isPayloadIntList) { // List of integers
        printf("\n\tmessage.payload: ");
        for (int i = 0; i < test.payloadListLength; i++)
            printf("'%d'; ", ((int *)result.payload)[i]);
    } else {
        printf("\n\tmessage.payload: 'BAD STUFF'");
    }
    tstDebugStep("tst: extraction [%d] 7", &i, NULL, NULL, NULL, NULL);

    // Print expected header
    printf("\n\n\t--- What was supposed to come: ---");
    printf("\n\texpectedResult.isValid: '%s'", tstBool(test.expectedResult.isValid));
    printf("\n\texpectedResult.id: '%d'", test.expectedResult.id);
    printf("\n\texpectedResult.source: '%d'", test.expectedResult.source);
    printf("\n\texpectedResult.target: '%d'", test.expectedResult.target);

    tstDebugStep("tst: extraction [%d] 8", &i, NULL, NULL, NULL, NULL);

    if (test.expectedResult.payloadText == NULL)
        printf("\n\texpectedResult.payloadText: 'NULL'");
    else {
        printf("\n\texpectedResult.payloadText: '%s'", test.expectedResult.payloadText);
    }
    tstDebugStep("tst: extraction [%d] 9", &i, NULL, NULL, NULL, NULL);

    // Print expected payload
    if (test.expectedResult.payload == NULL) // Empty
        printf("\n\texpectedResult.payload: 'NULL'");
    
    else if (test.isPayloadFloat) // Float
        printf("\n\texpectedResult.payload: '%.2f'", *(float *)test.expectedResult.payload);

    else if (test.isPayloadInt) // Integer
        printf("\n\texpectedResult.payload: '%d'", *(int *)test.expectedResult.payload);

    else if (test.isPayloadIntList) { // List of integers
        printf("\n\texpectedResult.payload: ");
        for (int j = 0; j < test.payloadListLength; j++)
            printf("'%d'; ", ((int *)test.expectedResult.payload)[j]);
    }
    tstDebugStep("tst: extraction [%d] 10", &i, NULL, NULL, NULL, NULL);

    // Explain how payload is wrong
    if (!payloadFlags.isOk) {
        tstDebugStep("tst: extraction [%d] 11 (!isValidPayload...)", &i, NULL, NULL, NULL, NULL);

        printf("\n\n\t--- How did payload went bad: ----");
        printf("\n\tisValidPayloadTxt: '%s'", tstBool(payloadFlags.isValidTxt));
        printf("\n\tisValidEmptyPayload: '%s'", tstBool(payloadFlags.isValidEmpty));
        printf("\n\tisValidPayloadFloat: '%s'", tstBool(payloadFlags.isValidFloat));
        printf("\n\tisValidPayloadInt: '%s'", tstBool(payloadFlags.isValidInt));
        printf("\n\tisValidPayloadIntList: '%s'", tstBool(payloadFlags.isValidIntList));
    }
    
    tstDebugStep("tst: extraction [%d] 12", &i, NULL, NULL, NULL, NULL);
    printf("\n");
}