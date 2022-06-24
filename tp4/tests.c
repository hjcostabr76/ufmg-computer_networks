#include "common.h"
#include "test_utils.h"

#include <stdlib.h>
#include <string.h>
#include <assert.h>

typedef struct {
    char *title;
    char *messageText;
    Message expectedResult;
    bool isVerbose;
    bool isPayloadFloat;
    bool isPayloadInt;
    bool isPayloadIntList;
    int payloadListLength;
} ExtractionTest;

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

TestResult tstMsgPatternValidation(void) {

    printf("\n");
    printf("\n>> ---------------------------------------------------------------------------- >>");
    printf("\n>> TEST: Validate protocol formatted messages --------------------------------- >>");
    printf("\n>> ---------------------------------------------------------------------------- >>");
    
    TestResult aux = { 0, 0 };
    TestResult finalResult = { 0, 0 };

    /* - New Test ------------- */
    bool isValid = true;
    
    finalResult.nTests++;
    int nPatterns = 2;
    const char *goodMessages[] = {
        "<msg><id>1<id><src>2<src><target>3<target><msg>",
        "<msg><id>1<id><src>2<src><target>3<target><payload>Loren Ipsun 123 Dolur<payload><msg>"
    };
    aux = tstMsgPatternValidationBatch(goodMessages, nPatterns, isValid, "Good");
    finalResult.nFailures += aux.nFailures;
    finalResult.nTests += aux.nTests;
    printf("\n");

    /* - New Test ------------- */
    isValid = false;
    
    finalResult.nTests++;
    nPatterns = 6;
    const char *badMessagesWrongTag[] = {
        "<message><id>1<id><src>2<src><target>3<target><payload>Loren Ipsun 123 Dolur<payload><message>",
        "<msg><id>1<id><source>2<source><target>3<target><payload>Loren Ipsun 123 Dolur<payload><msg>",
        "<msg><id>1<id><src>2<src><target>3<target><payload>Loren Ipsun 123 Dolur<payload><msg/>",
        "<msg><id>1<id><src>2<src><target>3<target><payload>Loren Ipsun 123 Dolur<payload/><msg>",
        "<msg><id>1<id><src>2<src><target>3<target/><payload>Loren Ipsun 123 Dolur<payload><msg>",
        "<msg><id>1<id><src>2<src/><target>3<target><payload>Loren Ipsun 123 Dolur<payload><msg>"
    };
    aux = tstMsgPatternValidationBatch(badMessagesWrongTag, nPatterns, isValid, "Bad: Wrong Tag names");
    finalResult.nFailures += aux.nFailures;
    finalResult.nTests += aux.nTests;
    printf("\n");

    /* - New Test ------------- */
    finalResult.nTests++;
    nPatterns = 6;
    const char *badMessagesClosingTags[] = {
        "<msg><id>1<id><src>2<src><target>3<target><payload>Loren Ipsun 123 Dolur<payload>",
        "<msg><id>1<id><src>2<src><target>3<target><payload>Loren Ipsun 123 Dolur<msg>",
        "<msg><id>1<id><src>2<src><target>3<target>Loren Ipsun 123 Dolur<payload><msg>",
        "<msg><id>1<id><src>2<src><target>3<payload>Loren Ipsun 123 Dolur<payload><msg>",
        "<msg><id>1<id><src>2<target>3<target><payload>Loren Ipsun 123 Dolur<payload><msg>",
        "<msg><id>1<src>2<src><target>3<target><payload>Loren Ipsun 123 Dolur<payload><msg>"
    };
    aux = tstMsgPatternValidationBatch(badMessagesClosingTags, nPatterns, isValid, "Bad: Missing close tags");
    finalResult.nFailures += aux.nFailures;
    finalResult.nTests += aux.nTests;
    printf("\n");

    /* - New Test ------------- */
    finalResult.nTests++;
    nPatterns = 4;
    const char *badMessagesMissingFields[] = {
        "<msg><id><id><src>2<src><target>3<target><msg>",
        "<msg><id>1<id><src><src><target>3<target><msg>",
        "<msg><id>1<id><src>2<src><target><target><msg>",
        "<msg><id>1<id><src>2<src><target>3<target><payload><payload><msg>",
    };
    aux = tstMsgPatternValidationBatch(badMessagesMissingFields, nPatterns, isValid, "Bad: Missing required fields");
    finalResult.nFailures += aux.nFailures;
    finalResult.nTests += aux.nTests;
    printf("\n");
    
    /* - New Test ------------- */
    finalResult.nTests++;
    nPatterns = 5;
    const char *badMessagesExtraChars[] = {
        "<msg><id>123<id><src>2<src><target>3<target><msg>",
        "<msg><id>1<id><src>123<src><target>3<target><msg>",
        "<msg><id>1<id><src>2<src><target>123<target><msg>",
        "<msg><id>1<id><src>2<src><target>3<target><payload>Loren Ipsun Dolur2<payload><msg>",
        "<msg><id>1<id><src>2<src><target>3<target><payload>1Loren Ipsun Dolur2<payload><msg>"
    };
    aux = tstMsgPatternValidationBatch(badMessagesExtraChars, nPatterns, isValid, "Bad: Extra characters");
    finalResult.nFailures += aux.nFailures;
    finalResult.nTests += aux.nTests;
    printf("\n");
    
    /* - New Test ------------- */
    finalResult.nTests++;
    nPatterns = 3;
    const char *badMessagesInvalidChars[] = {
        "<msg><id>1a<id><src>2<src><target>3<target><msg>",
        "<msg><id>1<id><src>2b<src><target>3<target><msg>",
        "<msg><id>1<id><src>2<src><target>3c<target><msg>"
    };
    aux = tstMsgPatternValidationBatch(badMessagesInvalidChars, nPatterns, isValid, "Bad: Invalid characters");
    finalResult.nFailures += aux.nFailures;
    finalResult.nTests += aux.nTests;
    printf("\n");

    /* - New Test ------------- */
    nPatterns = 3;
    finalResult.nTests++;
    const char *badMessagesSpaces[] = {
        "<msg><id>1 <id><src>2<src><target>3<target><msg>",
        "<msg><id>1<id><src> <src><target>3<target><msg>",
        "<msg><id>1<id><src>2<src><target>3 <target><msg>"
    };
    aux = tstMsgPatternValidationBatch(badMessagesSpaces, nPatterns, isValid, "Bad: Unexpected spaces");
    finalResult.nFailures += aux.nFailures;
    finalResult.nTests += aux.nTests;
    printf("\n");
    
    return finalResult;
}

TestResult tstMsgExtractionBatch(const ExtractionTest tests[], const int nTests, const bool shouldFail, const char *title) {
    printf("\n\n----- New Test: %s ------------------------------", title);
    
    int nFailures = 0;
    for (int i = 0; i < nTests; i++) {
        
        const ExtractionTest test = tests[i];
        printf("\nTesting pattern: \"%s\"\n\t\"%s\"...", test.title, test.messageText);
        
        // Run extraction
        Message message;
        setMessageFromText(test.messageText, &message);
        tstDebugStep("testMsgExtractionBatch [%d] 1", &i, NULL, NULL, NULL, NULL);
        
        // Validate result payload
        const bool isValidPayloadTxt = (
            (test.expectedResult.payloadText == NULL && message.payloadText == NULL)
            || (
                test.expectedResult.payloadText != NULL
                && message.payloadText != NULL
                && strcmp(message.payloadText, test.expectedResult.payloadText) == 0
            )
        );
        tstDebugStep("testMsgExtractionBatch [%d] 2 | %d", &i, tstBool(isValidPayloadTxt), NULL, NULL, NULL);
        
        bool isValidEmptyPayload = false;
        bool isValidPayloadFloat = false;
        bool isValidPayloadInt = false;
        bool isValidPayloadIntList = false;

        if (message.payload == NULL) // Test for is there anything to validate
            isValidEmptyPayload = test.expectedResult.payload == NULL;
        
        else if (test.isPayloadFloat) // Test for Float
            isValidPayloadFloat = *(float *)message.payload == *(float *)test.expectedResult.payload;
        
        else if (test.isPayloadInt) // Test for Integer
            isValidPayloadInt = *(int *)message.payload == *(int *)test.expectedResult.payload;
        
        else if (test.isPayloadIntList) { // Test for List of integers
            for (int i = 0; i < test.payloadListLength; i++) {
                isValidPayloadIntList = ((int *)message.payload)[i] == ((int *)test.expectedResult.payload)[i];
                if (!isValidPayloadIntList)
                    break;
            }
        }

        const bool isValidPayload = (
            isValidPayloadTxt
            && (
                isValidEmptyPayload
                || isValidPayloadFloat
                || isValidPayloadInt
                || isValidPayloadIntList
            )
        );

        // Validate the whole thing
        const bool isSuccess = (
            (!message.isValid && shouldFail)
            || (
                message.isValid
                && !shouldFail
                && message.id == test.expectedResult.id
                && message.source == test.expectedResult.source
                && message.target == test.expectedResult.target
                && isValidPayload
            )
        );
        tstDebugStep(
            "testMsgExtractionBatch [%d] 3.1 | isValidEmptyPayload: '%d', isValidPayloadFloat: '%d', isValidPayloadInt: '%d', isValidPayloadIntList: '%d'",
            &i, tstBool(isValidEmptyPayload), tstBool(isValidPayloadFloat), tstBool(isValidPayloadInt), tstBool(isValidPayloadIntList)
        );
        tstDebugStep(
            "testMsgExtractionBatch [%d] 3.2 | isValidPayload: '%d', isValidPayloadTxt: '%d'",
            &i, tstBool(isValidPayload), tstBool(isValidPayloadTxt), NULL, NULL
        );

        // Exhibit result
        char resultMsg[10];
        strcpy(resultMsg, isSuccess ? "ok" : "FAILED!");
        printf("\n[test %s]", resultMsg);

        const bool showDetails = test.isVerbose || !isSuccess;
        if (showDetails) {
            tstDebugStep("testMsgExtractionBatch [%d] 4 showDetails: '%d'", &i, tstBool(showDetails), NULL, NULL, NULL);
            
            // Print actual header
            printf("\n\n\t--- What came: -------------------");
            printf("\n\tmessage.isValid: '%d'", message.isValid);
            printf("\n\tmessage.id: '%d'", message.id);
            printf("\n\tmessage.source: '%d'", message.source);
            printf("\n\tmessage.target: '%d'", message.target);
            
            tstDebugStep("testMsgExtractionBatch [%d] 5", &i, NULL, NULL, NULL, NULL);
            
            if (message.payloadText == NULL)
                    printf("\n\tmessage.payloadText: 'NULL'");
            else {
                printf("\n\tmessage.payloadText: '%s'", message.payloadText);
            }
            tstDebugStep("testMsgExtractionBatch [%d] 6", &i, NULL, NULL, NULL, NULL);

            // Print actual payload
            if (message.payload == NULL) // Empty
                printf("\n\tmessage.payload: 'NULL'");

            else if (test.isPayloadFloat) // Float
                printf("\n\tmessage.payload: '%.2f'", *(float *)message.payload);

            else if (test.isPayloadInt) // Integer
                printf("\n\tmessage.payload: '%d'", *(int *)message.payload);

            else if (test.isPayloadIntList) { // List of integers
                printf("\n\tmessage.payload: ");
                for (int i = 0; i < test.payloadListLength; i++)
                    printf("'%d'; ", ((int *)message.payload)[i]);
            } else {
                printf("\n\tmessage.payload: 'BAD STUFF'");
            }
            tstDebugStep("testMsgExtractionBatch [%d] 7", &i, NULL, NULL, NULL, NULL);

            // Print expected header
            printf("\n\n\t--- What was supposed to come: ---");
            printf("\n\ttest.expectedResult.isValid: '%d'", test.expectedResult.isValid);
            printf("\n\ttest.expectedResult.id: '%d'", test.expectedResult.id);
            printf("\n\ttest.expectedResult.source: '%d'", test.expectedResult.source);
            printf("\n\ttest.expectedResult.target: '%d'", test.expectedResult.target);

            tstDebugStep("testMsgExtractionBatch [%d] 8", &i, NULL, NULL, NULL, NULL);

            if (test.expectedResult.payloadText == NULL)
                printf("\n\ttest.expectedResult.payloadText: 'NULL'");
            else {
                printf("\n\ttest.expectedResult.payloadText: '%s'", test.expectedResult.payloadText);
            }
            tstDebugStep("testMsgExtractionBatch [%d] 9", &i, NULL, NULL, NULL, NULL);

            // Print expected payload
            if (test.expectedResult.payload == NULL) // Empty
                printf("\n\ttest.expectedResult.payload: 'NULL'");
            
            else if (test.isPayloadFloat) // Float
                printf("\n\ttest.expectedResult.payload: '%.2f'", *(float *)test.expectedResult.payload);

            else if (test.isPayloadInt) // Integer
                printf("\n\ttest.expectedResult.payload: '%d'", *(int *)test.expectedResult.payload);

            else if (test.isPayloadIntList) { // List of integers
                printf("\n\ttest.expectedResult.payload: ");
                for (int i = 0; i < test.payloadListLength; i++)
                    printf("'%d'; ", ((int *)test.expectedResult.payload)[i]);
            }
            tstDebugStep("testMsgExtractionBatch [%d] 10", &i, NULL, NULL, NULL, NULL);

            // Explain how payload is wrong
            if (!isValidPayload) {
                tstDebugStep("testMsgExtractionBatch [%d] 11 (!isValidPayload...)", &i, NULL, NULL, NULL, NULL);

                printf("\n\n\t--- How did payload went bad: ----");
                printf("\n\tisValidPayload: %d", isValidPayload);
                printf("\n\tisValidPayloadTxt: %d", isValidPayloadTxt);
                printf("\n\tisValidEmptyPayload: %d", isValidEmptyPayload);
                printf("\n\tisValidPayloadFloat: %d", isValidPayloadFloat);
                printf("\n\tisValidPayloadInt: %d", isValidPayloadInt);
                printf("\n\tisValidPayloadIntList: %d", isValidPayloadIntList);
            }
            tstDebugStep("testMsgExtractionBatch [%d] 12", &i, NULL, NULL, NULL, NULL);

            // Compute result
            printf("\n");
            if (!isSuccess)
                nFailures++;
        }
    }
    tstDebugStep("testMsgExtractionBatch -- the end --", NULL, NULL, NULL, NULL, NULL);
    
    printf("\n");

    if (!nFailures)
        printf("-------------------- ALL TESTS PASSED! ------------------\n");
    else
        printf("-------------------- %d TEST(S) FAILED --------------------\n", nFailures);
    
    const TestResult result = { nTests, nFailures };
    return result;
}

TestResult tstMsgExtraction(void) {

    printf("\n");
    printf("\n>> ---------------------------------------------------------------------------- >>");
    printf("\n>> TEST: Extract message from text -------------------------------------------- >>");
    printf("\n>> ---------------------------------------------------------------------------- >>");
    
    TestResult aux = { 0, 0 };
    TestResult finalResult = { 0, 0 };

    bool isValid = true;
    bool isVerbose = false;
    
    ExtractionTest goodTests[10];
    int i = 0;
    int j = -1;

    /* ================================================================ */
    /* ---  GOOD Tests ------------------------------------------------ */

    /* - New Test ----------------------- */
    goodTests[i].title = "Valid MSG_REQ_ADD";
    goodTests[i].isVerbose = isVerbose;
    goodTests[i].messageText = "<msg><id>1<id>><msg>";
    
    goodTests[i].expectedResult.id = MSG_REQ_ADD;
    goodTests[i].expectedResult.source = 0;
    goodTests[i].expectedResult.target = 0;

    goodTests[i].expectedResult.payloadText = NULL;
    goodTests[i].expectedResult.payload = NULL;
    goodTests[i].isPayloadInt = false;
    goodTests[i].isPayloadFloat = false;
    goodTests[i].isPayloadIntList = false;
    goodTests[i].payloadListLength = 0;
    i++;

    /* - New Test ----------------------- */
    goodTests[i].title = "Valid MSG_REQ_RM";
    goodTests[i].isVerbose = isVerbose;
    goodTests[i].messageText = "<msg><id>2<id><src>3<src><msg>";
    
    goodTests[i].expectedResult.id = MSG_REQ_RM;
    goodTests[i].expectedResult.source = 3;
    goodTests[i].expectedResult.target = 0;

    goodTests[i].expectedResult.payloadText = NULL;
    goodTests[i].expectedResult.payload = NULL;
    goodTests[i].isPayloadInt = false;
    goodTests[i].isPayloadFloat = false;
    goodTests[i].isPayloadIntList = false;
    goodTests[i].payloadListLength = 0;

    /* - New Test ----------------------- */
    goodTests[i].title = "Valid MSG_RES_ADD";
    goodTests[i].isVerbose = isVerbose;
    goodTests[i].messageText = "<msg><id>3<id><payload>10<payload><msg>";
    
    goodTests[i].expectedResult.id = MSG_RES_ADD;
    goodTests[i].expectedResult.source = 0;
    goodTests[i].expectedResult.target = 0;

    goodTests[i].expectedResult.payloadText = "10";
    goodTests[i].expectedResult.payload = (int *)malloc(sizeof(int));
    *(int *)goodTests[i].expectedResult.payload = 10;
    
    goodTests[i].isPayloadInt = true;
    goodTests[i].isPayloadFloat = false;
    goodTests[i].isPayloadIntList = false;
    goodTests[i].payloadListLength = 0;
    i++;

    /* - New Test ----------------------- */
    goodTests[i].title = "Valid MSG_RES_LIST";
    goodTests[i].isVerbose = isVerbose;
    goodTests[i].messageText = "<msg><id>4<id><payload>10,1,12,11,5<payload><msg>";
    
    goodTests[i].expectedResult.id = MSG_RES_LIST;
    goodTests[i].expectedResult.source = 15;
    goodTests[i].expectedResult.target = 9;
    
    j = -1;
    goodTests[i].expectedResult.payloadText = "10,1,12,11,5";
    goodTests[i].expectedResult.payload = (int *)malloc(5 * sizeof(int));
    ((int *)goodTests[i].expectedResult.payload)[++j] = 10;
    ((int *)goodTests[i].expectedResult.payload)[++j] = 1;
    ((int *)goodTests[i].expectedResult.payload)[++j] = 12;
    ((int *)goodTests[i].expectedResult.payload)[++j] = 11;
    ((int *)goodTests[i].expectedResult.payload)[++j] = 5;

    goodTests[i].isPayloadInt = false;
    goodTests[i].isPayloadFloat = false;
    goodTests[i].isPayloadIntList = true;
    goodTests[i].payloadListLength = j + 1;
    i++;

    /* - New Test ----------------------- */
    goodTests[i].title = "Valid MSG_REQ_INF";
    goodTests[i].isVerbose = isVerbose;
    goodTests[i].messageText = "<msg><id>5<id><src>3<src><target>8<target><msg>";
    
    goodTests[i].expectedResult.id = MSG_REQ_INF;
    goodTests[i].expectedResult.source = 3;
    goodTests[i].expectedResult.target = 8;

    goodTests[i].expectedResult.payloadText = NULL;
    goodTests[i].expectedResult.payload = NULL;
    goodTests[i].isPayloadInt = false;
    goodTests[i].isPayloadFloat = false;
    goodTests[i].isPayloadIntList = false;
    goodTests[i].payloadListLength = 0;
    i++;

    /* - New Test ----------------------- */
    goodTests[i].title = "Valid MSG_RES_INF";
    goodTests[i].isVerbose = isVerbose;
    goodTests[i].messageText = "<msg><id>6<id><src>2<src><target>7<target><payload>120.52<payload><msg>";
    
    goodTests[i].expectedResult.id = MSG_RES_INF;
    goodTests[i].expectedResult.source = 2;
    goodTests[i].expectedResult.target = 7;

    goodTests[i].expectedResult.payloadText = "120.52";
    goodTests[i].expectedResult.payload = (float *)malloc(sizeof(float));
    *(float *)goodTests[i].expectedResult.payload = 120.52;
    
    goodTests[i].isPayloadInt = false;
    goodTests[i].isPayloadFloat = true;
    goodTests[i].isPayloadIntList = false;
    goodTests[i].payloadListLength = 0;
    i++;

    /* - New Test ----------------------- */
    goodTests[i].title = "Valid MSG_ERR";
    goodTests[i].isVerbose = isVerbose;
    goodTests[i].messageText = "<msg><id>7<id><target>1<target><payload>4<payload><msg>";
    
    goodTests[i].expectedResult.id = MSG_ERR;
    goodTests[i].expectedResult.source = 0;
    goodTests[i].expectedResult.target = 1;

    goodTests[i].expectedResult.payloadText = "4";
    goodTests[i].expectedResult.payload = (ErrorCodeEnum *)malloc(sizeof(ErrorCodeEnum));
    *(ErrorCodeEnum *)goodTests[i].expectedResult.payload = ERR_MAX_EQUIP;
    
    goodTests[i].isPayloadInt = true;
    goodTests[i].isPayloadFloat = false;
    goodTests[i].isPayloadIntList = false;
    goodTests[i].payloadListLength = 0;
    i++;

    /* - New Test ----------------------- */
    goodTests[i].title = "Valid MSG_OK";
    goodTests[i].isVerbose = isVerbose;
    goodTests[i].messageText = "<msg><src>13<src><target>11<target><msg>";
    
    goodTests[i].expectedResult.id = MSG_OK;
    goodTests[i].expectedResult.source = 13;
    goodTests[i].expectedResult.target = 11;

    goodTests[i].expectedResult.payloadText = "1";
    goodTests[i].expectedResult.payload = (OkMessageCodeEnum *)malloc(sizeof(OkMessageCodeEnum));
    *(OkMessageCodeEnum *)goodTests[i].expectedResult.payload = OK_RM;
    
    goodTests[i].isPayloadInt = true;
    goodTests[i].isPayloadFloat = false;
    goodTests[i].isPayloadIntList = false;
    goodTests[i].payloadListLength = 0;
    i++;

    /* >> Test em'all! ------------------------>> */
    aux = tstMsgExtractionBatch(goodTests, i, isValid, "Good");
    finalResult.nFailures += aux.nFailures;
    finalResult.nTests += aux.nTests;
    printf("\n");

    /* ================================================================ */
    /* ---  BAD Tests: General ---------------------------------------- */

    ExtractionTest badGeneral[10];

    i = 0;
    isValid = false;
    isVerbose = true;

    /* - New Test ----------------------- */
    badGeneral[i].title = "Invalid Message ID";
    badGeneral[i].isVerbose = isVerbose;
    badGeneral[i].messageText = "<msg><id>9<id><src>2<src><target>3<target><msg>";
    
    badGeneral[i].expectedResult.id = 0;
    badGeneral[i].expectedResult.source = 2;
    badGeneral[i].expectedResult.target = 3;
    badGeneral[i].expectedResult.payloadText = NULL;
    badGeneral[i].expectedResult.payload = NULL;
    
    badGeneral[i].isPayloadInt = false;
    badGeneral[i].isPayloadFloat = false;
    badGeneral[i].isPayloadIntList = false;
    badGeneral[i].payloadListLength = 0;
    i++;

    /* - New Test ----------------------- */
    badGeneral[i].title = "Invalid Message ID";
    badGeneral[i].isVerbose = isVerbose;
    badGeneral[i].messageText = "<msg><id>0<id><src>2<src><target>3<target><msg>";
    
    badGeneral[i].expectedResult.id = 0;
    badGeneral[i].expectedResult.source = 2;
    badGeneral[i].expectedResult.target = 3;
    badGeneral[i].expectedResult.payloadText = NULL;
    badGeneral[i].expectedResult.payload = NULL;
    
    badGeneral[i].isPayloadInt = false;
    badGeneral[i].isPayloadFloat = false;
    badGeneral[i].isPayloadIntList = false;
    badGeneral[i].payloadListLength = 0;
    i++;

    /* - New Test ----------------------- */
    badGeneral[i].title = "Invalid source equipment";
    badGeneral[i].isVerbose = isVerbose;
    badGeneral[i].messageText = "<msg><id>3<id><src>0<src><target>3<target><payload>1<payload><msg>";
    
    badGeneral[i].expectedResult.id = MSG_RES_ADD;
    badGeneral[i].expectedResult.source = 0;
    badGeneral[i].expectedResult.target = 3;
    
    badGeneral[i].expectedResult.payloadText = "1";
    badGeneral[i].expectedResult.payload = (int *)malloc(sizeof(int));
    *(int *)badGeneral[i].expectedResult.payload = 1;
    
    badGeneral[i].isPayloadInt = true;
    badGeneral[i].isPayloadFloat = false;
    badGeneral[i].isPayloadIntList = false;
    badGeneral[i].payloadListLength = 0;
    i++;

    /* - New Test ----------------------- */
    badGeneral[i].title = "Invalid target equipment";
    badGeneral[i].isVerbose = isVerbose;
    badGeneral[i].messageText = "<msg><id>8<id><src>6<src><target>0<target><payload>1.12<payload><msg>";
    
    badGeneral[i].expectedResult.id = MSG_OK;
    badGeneral[i].expectedResult.source = 6;
    badGeneral[i].expectedResult.target = 0;
    
    badGeneral[i].expectedResult.payloadText = "1.12";
    badGeneral[i].expectedResult.payload = (float *)malloc(sizeof(float));
    *(float *)badGeneral[i].expectedResult.payload = 1.12;
    
    badGeneral[i].isPayloadInt = false;
    badGeneral[i].isPayloadFloat = true;
    badGeneral[i].isPayloadIntList = false;
    badGeneral[i].payloadListLength = 0;
    i++;

    /* - New Test ----------------------- */
    badGeneral[i].title = "Missing <msg> tag";
    badGeneral[i].isVerbose = isVerbose;
    badGeneral[i].messageText = "<id>6<id><src>6<src><target>14<target><payload>1,12<payload>";
    
    badGeneral[i].expectedResult.id = MSG_RES_LIST;
    badGeneral[i].expectedResult.source = 6;
    badGeneral[i].expectedResult.target = 14;
    badGeneral[i].expectedResult.payloadText = "1,12";
    
    j = -1;
    badGeneral[i].expectedResult.payload = (int *)malloc(2 * sizeof(int));
    ((int *)badGeneral[i].expectedResult.payload)[++j] = 10;
    ((int *)badGeneral[i].expectedResult.payload)[++j] = 1;
    
    badGeneral[i].isPayloadInt = false;
    badGeneral[i].isPayloadFloat = false;
    badGeneral[i].isPayloadIntList = true;
    badGeneral[i].payloadListLength = 0;
    i++;

    /* - New Test ----------------------- */
    badGeneral[i].title = "Missing <id> tag";
    badGeneral[i].isVerbose = isVerbose;
    badGeneral[i].messageText = "<msg><src>6<src><target>14<target><payload>1,12<payload><msg>";
    
    badGeneral[i].expectedResult.id = MSG_RES_LIST;
    badGeneral[i].expectedResult.source = 6;
    badGeneral[i].expectedResult.target = 14;
    badGeneral[i].expectedResult.payloadText = "1,12";

    j = -1;
    badGeneral[i].expectedResult.payload = (int *)malloc(2 * sizeof(int));
    ((int *)badGeneral[i].expectedResult.payload)[++j] = 10;
    ((int *)badGeneral[i].expectedResult.payload)[++j] = 1;

    badGeneral[i].isPayloadInt = false;
    badGeneral[i].isPayloadFloat = false;
    badGeneral[i].isPayloadIntList = true;
    badGeneral[i].payloadListLength = 0;
    i++;

    /* - New Test ----------------------- */
    badGeneral[i].title = "Nonsense payload";
    badGeneral[i].isVerbose = isVerbose;
    badGeneral[i].messageText = "<msg><id>6<id><src>2<src><target>3<target><payload>Loren Ipsun 123 Dolur<payload><msg>";
    
    badGeneral[i].expectedResult.id = MSG_RES_LIST;
    badGeneral[i].expectedResult.source = 2;
    badGeneral[i].expectedResult.target = 3;
    badGeneral[i].expectedResult.payloadText = "Loren Ipsun 123 Dolur";
    badGeneral[i].expectedResult.payload = NULL;
    
    badGeneral[i].isPayloadInt = false;
    badGeneral[i].isPayloadFloat = false;
    badGeneral[i].isPayloadIntList = false;
    badGeneral[i].payloadListLength = 0;
    i++;

    /* >> Test em'all! ------------------------>> */
    aux = tstMsgExtractionBatch(badGeneral, i, isValid, "Bad (general)");
    finalResult.nFailures += aux.nFailures;
    finalResult.nTests += aux.nTests;
    printf("\n");

    /* ================================================================ */
    /* ---  BAD Tests: MSG_REQ_ADD ------------------------------------ */

    // ExtractionTest badAdd[10];

    // i = 0;
    // isValid = false;
    // isVerbose = true;

    // /* - New Test: Invalid MSG_REQ_ADD ---------- */
    // badAdd[i].isVerbose = isVerbose;
    // badAdd[i].messageText = "<msg><id>1<id><src>12<src><target>13<target><payload>1.12<payload><msg>";
    
    // badAdd[i].expectedResult.id = 0;
    // badAdd[i].expectedResult.source = 0;
    // badAdd[i].expectedResult.target = 0;
    // badAdd[i].expectedResult.payloadText = NULL;
    // badAdd[i].expectedResult.payload = NULL;
    
    // badAdd[i].isPayloadInt = false;
    // badAdd[i].isPayloadFloat = false;
    // badAdd[i].isPayloadIntList = false;
    // badAdd[i].payloadListLength = 0;
    // i++;

    // /* - New Test: Invalid MSG_REQ_ADD ---------- */
    // badAdd[i].isVerbose = isVerbose;
    // badAdd[i].messageText = "<msg><id>1<id><src>12<src><msg>";
    
    // badAdd[i].expectedResult.id = 0;
    // badAdd[i].expectedResult.source = 0;
    // badAdd[i].expectedResult.target = 0;
    // badAdd[i].expectedResult.payloadText = NULL;
    // badAdd[i].expectedResult.payload = NULL;
    
    // badAdd[i].isPayloadInt = false;
    // badAdd[i].isPayloadFloat = false;
    // badAdd[i].isPayloadIntList = false;
    // badAdd[i].payloadListLength = 0;
    // i++;

    // /* - New Test: Invalid MSG_REQ_ADD ---------- */
    // badAdd[i].isVerbose = isVerbose;
    // badAdd[i].messageText = "<msg><id>1<id><target>13<target><msg>";
    
    // badAdd[i].expectedResult.id = 0;
    // badAdd[i].expectedResult.source = 0;
    // badAdd[i].expectedResult.target = 0;
    // badAdd[i].expectedResult.payloadText = NULL;
    // badAdd[i].expectedResult.payload = NULL;
    
    // badAdd[i].isPayloadInt = false;
    // badAdd[i].isPayloadFloat = false;
    // badAdd[i].isPayloadIntList = false;
    // badAdd[i].payloadListLength = 0;
    // i++;

    // /* - New Test: Invalid MSG_REQ_ADD ---------- */
    // badAdd[i].isVerbose = isVerbose;
    // badAdd[i].messageText = "<msg><id>1<id><payload>1.12<payload><msg>";
    
    // badAdd[i].expectedResult.id = 0;
    // badAdd[i].expectedResult.source = 0;
    // badAdd[i].expectedResult.target = 0;
    // badAdd[i].expectedResult.payloadText = NULL;
    // badAdd[i].expectedResult.payload = NULL;
    
    // badAdd[i].isPayloadInt = false;
    // badAdd[i].isPayloadFloat = false;
    // badAdd[i].isPayloadIntList = false;
    // badAdd[i].payloadListLength = 0;
    // i++;

    // /* >> Test em'all! ------------------------>> */
    // // aux = tstMsgExtractionBatch(badAdd, i, isValid, "Bad");
    // finalResult.nFailures += aux.nFailures;
    // finalResult.nTests += aux.nTests;
    // printf("\n");

    return finalResult;
}

int main() {

    printf("\n");
    printf("\n>> ---------------------------------------------------------------------------- >>");
    printf("\n>> Running tests... >> Running tests... >> Running tests... >> Running tests... >>");
    printf("\n>> ---------------------------------------------------------------------------- >>");

    // Run test groups
    TestResult acc = { 0, 0 };
    TestResult aux;
    int nGroups = 0;

    // nGroups++;
    // const char tag[] = "<test>";
    // const char message[] = "<test>Loren Ipsun Dolur<test>";

    // int begin = 0;
    // int end = 0;
    // bool isSuccessTemp = strSetDelimitedTextBounds(message, tag, &begin, &end);
    
    // char content[100] = "";
    // strGetSubstring(message, content, begin, end);
    // printf("\nbeing: '%d', end: '%d', content: '%s'\n", begin, end, content);

    // acc.nTests += 1;
    // acc.nFailures += !isSuccessTemp;    

    // nGroups++;
    // aux = tstMsgPatternValidation();
    // acc.nTests += aux.nTests;
    // acc.nFailures += aux.nFailures;

    nGroups++;
    aux = tstMsgExtraction();
    acc.nTests += aux.nTests;
    acc.nFailures += aux.nFailures;

    // Notify end result
    bool isSuccess = acc.nFailures == 0;
    printf("\n<< ---------------------------------------------------------------------------- <<\n");

    if (isSuccess)
        printf("<<<< All tests passed! >>> <<<< All tests passed!! >>>> <<< All tests passed! >>>>\n");
    else
        printf("<<<<--- TESTS FAILED --->>> <<<--- TESTS FAILED --->>> <<<--- TESTS FAILED --->>>>\n");
    
    printf("<< ---------------------------------------------------------------------------- <<\n\n");
    printf(">> %d test group(s);\n", nGroups);
    printf(">> %d unit test(s);\n", acc.nTests);
    printf(">> %d error(s);\n", acc.nFailures);
    printf("\n");

    return EXIT_SUCCESS;
}