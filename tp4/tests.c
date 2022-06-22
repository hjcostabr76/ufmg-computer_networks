#include "common.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    int nTests;
    int nFailures;
} TestResult;

typedef struct {
    int nTests;
    int nFailures;
    TestResult tests[20];
} TestBatchResult;

typedef struct {
    char *messageText;
    Message expectedResult;
    bool isVerbose;
    bool isPayloadFloat;
    bool isPayloadInt;
    bool isPayloadIntList;
    int payloadListLength;
} ExtractionTest;

TestResult testMsgPatternValidationBatch(const char **messages, const int nTests, const bool isValidMessage, const char *title) {

    char testType[15] = "";
    strcpy(testType, isValidMessage ? "Good\0" : "Bad\0");
    printf("\n\n----- New Test: %s ------------------------------", title);
    
    int nFailures = 0;
    for (int i = 0; i < nTests; i++) {
        
        printf("\nTesting %s pattern:\n\t\"%s\"...", testType, messages[i]);
        const bool passedValidation = validateReceivedMsg(messages[i]);
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

TestResult testMsgPatternValidation(void) {

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
    aux = testMsgPatternValidationBatch(goodMessages, nPatterns, isValid, "Good");
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
    aux = testMsgPatternValidationBatch(badMessagesWrongTag, nPatterns, isValid, "Bad: Wrong Tag names");
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
    aux = testMsgPatternValidationBatch(badMessagesClosingTags, nPatterns, isValid, "Bad: Missing close tags");
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
    aux = testMsgPatternValidationBatch(badMessagesMissingFields, nPatterns, isValid, "Bad: Missing required fields");
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
    aux = testMsgPatternValidationBatch(badMessagesExtraChars, nPatterns, isValid, "Bad: Extra characters");
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
    aux = testMsgPatternValidationBatch(badMessagesInvalidChars, nPatterns, isValid, "Bad: Invalid characters");
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
    aux = testMsgPatternValidationBatch(badMessagesSpaces, nPatterns, isValid, "Bad: Unexpected spaces");
    finalResult.nFailures += aux.nFailures;
    finalResult.nTests += aux.nTests;
    printf("\n");
    
    return finalResult;
}

TestResult testMsgExtractionBatch(const ExtractionTest tests[], const int nTests, const bool isValidCase, const char *title) {

    char testType[15] = "";
    strcpy(testType, isValidCase ? "Good\0" : "Bad\0");
    printf("\n\n----- New Test: %s ------------------------------", title);
    
    int nFailures = 0;
    for (int i = 0; i < nTests; i++) {
        
        const ExtractionTest test = tests[i];
        printf("\nTesting %s pattern:\n\"%s\"...", testType, test.messageText);
        
        // Run extraction
        Message message;
        bool isParsingOk = setMessageFromText(test.messageText, &message);
        
        // Validate result payload
        const bool isValidPayloadTxt = (
            (test.expectedResult.payloadText == NULL && message.payloadText == NULL)
            || (
                test.expectedResult.payloadText != NULL
                && message.payloadText != NULL
                && strcmp(message.payloadText, test.expectedResult.payloadText) == 0
            )
        );

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
            (!isParsingOk && !isValidCase)
            || (
                isParsingOk
                && isValidCase
                && message.id == test.expectedResult.id
                && message.source == test.expectedResult.source
                && message.target == test.expectedResult.target
                && isValidPayload
            )
        );

        // Exhibit result
        char resultMsg[10];
        strcpy(resultMsg, isSuccess ? "ok" : "FAILED!");
        printf("\n[test %s]", resultMsg);

        const bool showDetails = test.isVerbose || !isSuccess;
        if (showDetails) {
            
            // Print actual header
            printf("\n\n\t--- What came: -------------------");
            printf("\n\tmessage.id: '%d'", message.id);
            printf("\n\tmessage.source: '%d'", message.source);
            printf("\n\tmessage.target: '%d'", message.target);
            
            if (message.payloadText == NULL)
                    printf("\n\tmessage.payloadText: 'NULL'");
                else
                    printf("\n\tmessage.payloadText: '%s'", message.payloadText);

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
            } else
                printf("\n\tmessage.payload: 'BAD STUFF'");

            // Print expected header
            printf("\n\n\t--- What was supposed to come: ---");
            printf("\n\ttest.expectedResult.id: '%d'", test.expectedResult.id);
            printf("\n\ttest.expectedResult.source: '%d'", test.expectedResult.source);
            printf("\n\ttest.expectedResult.target: '%d'", test.expectedResult.target);

            if (test.expectedResult.payloadText == NULL)
                printf("\n\ttest.expectedResult.payloadText: 'NULL'");
            else
                printf("\n\ttest.expectedResult.payloadText: '%s'", test.expectedResult.payloadText);
            
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

            // Explain how payload is wrong
            if (!isValidPayload) {
                printf("\n\n\t--- How did payload went bad: ----");
                printf("\n\tisValidPayload: %d", isValidPayload);
                printf("\n\tisValidPayloadTxt: %d", isValidPayloadTxt);
                printf("\n\tisValidEmptyPayload: %d", isValidEmptyPayload);
                printf("\n\tisValidPayloadFloat: %d", isValidPayloadFloat);
                printf("\n\tisValidPayloadInt: %d", isValidPayloadInt);
                printf("\n\tisValidPayloadIntList: %d", isValidPayloadIntList);
            }

            // Compute result
            printf("\n");
            if (!isSuccess)
                nFailures++;
        }
    }

    printf("\n");

    if (!nFailures)
        printf("-------------------- ALL TESTS PASSED! ------------------\n");
    else
        printf("-------------------- %d TEST(S) FAILED --------------------\n", nFailures);
    
    const TestResult result = { nTests, nFailures };
    return result;
}

TestResult testMsgExtraction(void) {

    printf("\n");
    printf("\n>> ---------------------------------------------------------------------------- >>");
    printf("\n>> TEST: Extract message from text -------------------------------------------- >>");
    printf("\n>> ---------------------------------------------------------------------------- >>");
    
    TestResult aux = { 0, 0 };
    TestResult finalResult = { 0, 0 };

    bool isValid = true;
    bool isVerbose = true;
    
    ExtractionTest goodTests[10];
    int i = 0;
    int j = -1;

    /* - New Test: MSG_REQ_ADD --- */
    goodTests[i].isVerbose = isVerbose;
    goodTests[i].messageText = "<msg><id>1<id><src>2<src><target>3<target><msg>";
    
    goodTests[i].expectedResult.id = MSG_REQ_ADD;
    goodTests[i].expectedResult.source = 2;
    goodTests[i].expectedResult.target = 3;

    goodTests[i].expectedResult.payloadText = NULL;
    goodTests[i].expectedResult.payload = NULL;
    goodTests[i].isPayloadInt = false;
    goodTests[i].isPayloadFloat = false;
    goodTests[i].isPayloadIntList = false;
    goodTests[i].payloadListLength = 0;
    i++;

    /* - New Test: MSG_REQ_RM ---- */
    goodTests[i].isVerbose = isVerbose;
    goodTests[i].messageText = "<msg><id>2<id><src>3<src><target>4<target><msg>";
    
    goodTests[i].expectedResult.id = MSG_REQ_RM;
    goodTests[i].expectedResult.source = 3;
    goodTests[i].expectedResult.target = 4;

    goodTests[i].expectedResult.payloadText = NULL;
    goodTests[i].expectedResult.payload = NULL;
    goodTests[i].isPayloadInt = false;
    goodTests[i].isPayloadFloat = false;
    goodTests[i].isPayloadIntList = false;
    goodTests[i].payloadListLength = 0;

    /* - New Test: MSG_RES_ADD --- */
    goodTests[i].isVerbose = isVerbose;
    goodTests[i].messageText = "<msg><id>3<id><src>5<src><target>14<target><payload>10<payload><msg>";
    
    goodTests[i].expectedResult.id = MSG_RES_ADD;
    goodTests[i].expectedResult.source = 5;
    goodTests[i].expectedResult.target = 14;

    goodTests[i].expectedResult.payloadText = "10";
    goodTests[i].expectedResult.payload = (int *)malloc(sizeof(int));
    *(int *)goodTests[i].expectedResult.payload = 10;
    
    goodTests[i].isPayloadInt = true;
    goodTests[i].isPayloadFloat = false;
    goodTests[i].isPayloadIntList = false;
    goodTests[i].payloadListLength = 0;
    i++;

    /* - New Test: MSG_RES_LIST -- */
    j = -1;

    goodTests[i].isVerbose = isVerbose;
    goodTests[i].messageText = "<msg><id>4<id><src>15<src><target>9<target><payload>10,1,12,11,5<payload><msg>";
    
    goodTests[i].expectedResult.id = MSG_RES_LIST;
    goodTests[i].expectedResult.source = 15;
    goodTests[i].expectedResult.target = 9;
    
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

    /* - New Test: MSG_REQ_INF --- */
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

    /* - New Test: MSG_RES_INF --- */
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

    /* - New Test: MSG_ERR ------- */
    goodTests[i].isVerbose = isVerbose;
    goodTests[i].messageText = "<msg><id>7<id><src>12<src><target>1<target><payload>4<payload><msg>";
    
    goodTests[i].expectedResult.id = MSG_ERR;
    goodTests[i].expectedResult.source = 12;
    goodTests[i].expectedResult.target = 1;

    goodTests[i].expectedResult.payloadText = "4";
    goodTests[i].expectedResult.payload = (ErrorCodeEnum *)malloc(sizeof(ErrorCodeEnum));
    *(ErrorCodeEnum *)goodTests[i].expectedResult.payload = ERR_MAX_EQUIP;
    
    goodTests[i].isPayloadInt = true;
    goodTests[i].isPayloadFloat = false;
    goodTests[i].isPayloadIntList = false;
    goodTests[i].payloadListLength = 0;
    i++;

    /* - New Test: MSG_OK -------- */
    goodTests[i].isVerbose = isVerbose;
    goodTests[i].messageText = "<msg><id>8<id><src>13<src><target>11<target><payload>1<payload><msg>";
    
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

    /* - New Test ------------- */
    // goodTests[i].isVerbose = isVerbose;
    // goodTests[i].messageText = "<msg><id>1<id><src>2<src><target>3<target><payload>Loren Ipsun 123 Dolur<payload><msg>";
    
    // goodTests[i].expectedResult.id = 1;
    // goodTests[i].expectedResult.source = 2;
    // goodTests[i].expectedResult.target = 3;
    
    // goodTests[i].expectedResult.payload = "Loren Ipsun 123 Dolur";
    // goodTests[i].expectedResult.payloadText = "Loren Ipsun 123 Dolur";
    // goodTests[i].isPayloadInt = false;
    // goodTests[i].isPayloadFloat = false;
    // goodTests[i].isPayloadIntList = false;
    // goodTests[i].payloadListLength = 0;
    // i++;

    /* - Test em'all! --------- */
    aux = testMsgExtractionBatch(goodTests, i, isValid, "Good");
    finalResult.nFailures += aux.nFailures;
    finalResult.nTests += aux.nTests;
    printf("\n");
    
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
    // aux = testMsgPatternValidation();
    // acc.nTests += aux.nTests;
    // acc.nFailures += aux.nFailures;

    nGroups++;
    aux = testMsgExtraction();
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