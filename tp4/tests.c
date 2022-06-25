#include "common.h"
#include "test_utils.h"
#include "test_runners.h"

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

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

TestResult tstMsgExtractionGood(void) {

    printf("\n");
    printf("\n>> ---------------------------------------------------------------------------- >>");
    printf("\n>> TEST: Extract message from text (happy cases) ------------------------------ >>");
    printf("\n>> ---------------------------------------------------------------------------- >>");
    
    TestResult aux = { 0, 0 };
    TestResult finalResult = { 0, 0 };

    bool isVerbose = false;
    
    ExtractionTest tests[10];
    int i = 0;
    int j = -1;

    /* ================================================================ */
    /* ---  GOOD Tests ------------------------------------------------ */

    /* - New Test ----------------------- */
    tests[i].title = "Valid MSG_REQ_ADD";
    tests[i].isVerbose = isVerbose;
    tests[i].messageText = "<msg><id>1<id>><msg>";
    
    tests[i].expectedResult.id = MSG_REQ_ADD;
    tests[i].expectedResult.isValid = true;
    tests[i].expectedResult.source = 0;
    tests[i].expectedResult.target = 0;

    tests[i].expectedResult.payloadText = NULL;
    tests[i].expectedResult.payload = NULL;
    tests[i].isPayloadInt = false;
    tests[i].isPayloadFloat = false;
    tests[i].isPayloadIntList = false;
    tests[i].payloadListLength = 0;
    i++;

    /* - New Test ----------------------- */
    tests[i].title = "Valid MSG_REQ_RM";
    tests[i].isVerbose = isVerbose;
    tests[i].messageText = "<msg><id>2<id><src>3<src><msg>";
    
    tests[i].expectedResult.id = MSG_REQ_RM;
    tests[i].expectedResult.isValid = true;
    tests[i].expectedResult.source = 3;
    tests[i].expectedResult.target = 0;

    tests[i].expectedResult.payloadText = NULL;
    tests[i].expectedResult.payload = NULL;
    tests[i].isPayloadInt = false;
    tests[i].isPayloadFloat = false;
    tests[i].isPayloadIntList = false;
    tests[i].payloadListLength = 0;
    i++;

    /* - New Test ----------------------- */
    tests[i].title = "Valid MSG_RES_ADD";
    tests[i].isVerbose = isVerbose;
    tests[i].messageText = "<msg><id>3<id><payload>10<payload><msg>";
    
    tests[i].expectedResult.id = MSG_RES_ADD;
    tests[i].expectedResult.isValid = true;
    tests[i].expectedResult.source = 0;
    tests[i].expectedResult.target = 0;

    tests[i].expectedResult.payloadText = "10";
    tests[i].expectedResult.payload = (int *)malloc(sizeof(int));
    *(int *)tests[i].expectedResult.payload = 10;
    
    tests[i].isPayloadInt = true;
    tests[i].isPayloadFloat = false;
    tests[i].isPayloadIntList = false;
    tests[i].payloadListLength = 0;
    i++;

    /* - New Test ----------------------- */
    tests[i].title = "Valid MSG_RES_LIST";
    tests[i].isVerbose = isVerbose;
    tests[i].messageText = "<msg><id>4<id><payload>10,1,12,11,5<payload><msg>";
    
    tests[i].expectedResult.id = MSG_RES_LIST;
    tests[i].expectedResult.isValid = true;
    tests[i].expectedResult.source = 0;
    tests[i].expectedResult.target = 0;
    
    j = -1;
    tests[i].expectedResult.payloadText = "10,1,12,11,5";
    tests[i].expectedResult.payload = (int *)malloc(5 * sizeof(int));
    ((int *)tests[i].expectedResult.payload)[++j] = 10;
    ((int *)tests[i].expectedResult.payload)[++j] = 1;
    ((int *)tests[i].expectedResult.payload)[++j] = 12;
    ((int *)tests[i].expectedResult.payload)[++j] = 11;
    ((int *)tests[i].expectedResult.payload)[++j] = 5;

    tests[i].isPayloadInt = false;
    tests[i].isPayloadFloat = false;
    tests[i].isPayloadIntList = true;
    tests[i].payloadListLength = j + 1;
    i++;

    /* - New Test ----------------------- */
    tests[i].title = "Valid MSG_REQ_INF";
    tests[i].isVerbose = isVerbose;
    tests[i].messageText = "<msg><id>5<id><src>3<src><target>8<target><msg>";
    
    tests[i].expectedResult.id = MSG_REQ_INF;
    tests[i].expectedResult.isValid = true;
    tests[i].expectedResult.source = 3;
    tests[i].expectedResult.target = 8;

    tests[i].expectedResult.payloadText = NULL;
    tests[i].expectedResult.payload = NULL;
    tests[i].isPayloadInt = false;
    tests[i].isPayloadFloat = false;
    tests[i].isPayloadIntList = false;
    tests[i].payloadListLength = 0;
    i++;

    /* - New Test ----------------------- */
    tests[i].title = "Valid MSG_RES_INF";
    tests[i].isVerbose = isVerbose;
    tests[i].messageText = "<msg><id>6<id><src>2<src><target>7<target><payload>120.52<payload><msg>";
    
    tests[i].expectedResult.id = MSG_RES_INF;
    tests[i].expectedResult.isValid = true;
    tests[i].expectedResult.source = 2;
    tests[i].expectedResult.target = 7;

    tests[i].expectedResult.payloadText = "120.52";
    tests[i].expectedResult.payload = (float *)malloc(sizeof(float));
    *(float *)tests[i].expectedResult.payload = 120.52;
    
    tests[i].isPayloadInt = false;
    tests[i].isPayloadFloat = true;
    tests[i].isPayloadIntList = false;
    tests[i].payloadListLength = 0;
    i++;

    /* - New Test ----------------------- */
    tests[i].title = "Valid MSG_ERR";
    tests[i].isVerbose = isVerbose;
    tests[i].messageText = "<msg><id>7<id><target>1<target><payload>4<payload><msg>";
    
    tests[i].expectedResult.id = MSG_ERR;
    tests[i].expectedResult.isValid = true;
    tests[i].expectedResult.source = 0;
    tests[i].expectedResult.target = 1;

    tests[i].expectedResult.payloadText = "4";
    tests[i].expectedResult.payload = (ErrorCodeEnum *)malloc(sizeof(ErrorCodeEnum));
    *(ErrorCodeEnum *)tests[i].expectedResult.payload = ERR_MAX_EQUIP;
    
    tests[i].isPayloadInt = true;
    tests[i].isPayloadFloat = false;
    tests[i].isPayloadIntList = false;
    tests[i].payloadListLength = 0;
    i++;

    /* - New Test ----------------------- */
    tests[i].title = "Valid MSG_OK";
    tests[i].isVerbose = isVerbose;
    tests[i].messageText = "<msg><id>8<id><target>11<target><payload>1<payload><msg>";
    
    tests[i].expectedResult.id = MSG_OK;
    tests[i].expectedResult.isValid = true;
    tests[i].expectedResult.source = 0;
    tests[i].expectedResult.target = 11;

    tests[i].expectedResult.payloadText = "1";
    tests[i].expectedResult.payload = (OkMessageCodeEnum *)malloc(sizeof(OkMessageCodeEnum));
    *(OkMessageCodeEnum *)tests[i].expectedResult.payload = OK_RM;
    
    tests[i].isPayloadInt = true;
    tests[i].isPayloadFloat = false;
    tests[i].isPayloadIntList = false;
    tests[i].payloadListLength = 0;
    i++;

    /* >> Test em'all! ------------------------>> */
    aux = tstMsgExtractionBatch(tests, i, "Good");
    finalResult.nFailures += aux.nFailures;
    finalResult.nTests += aux.nTests;
    printf("\n");

    return finalResult;
}

TestResult tstMsgExtractionBadTags(void) {

    printf("\n");
    printf("\n>> ---------------------------------------------------------------------------- >>");
    printf("\n>> TEST: Extract message from text (bad tags) --------------------------------- >>");
    printf("\n>> ---------------------------------------------------------------------------- >>");
    
    TestResult aux = { 0, 0 };
    TestResult finalResult = { 0, 0 };

    ExtractionTest tests[10];
    bool isVerbose = false;
    
    int i = 0;
    int j = -1;

    /* - New Test ----------------------- */
    
    // NOTE: Without the message delimiter we don't know what to read...
    
    tests[i].title = "Missing <msg> tag";
    tests[i].isVerbose = isVerbose;
    tests[i].messageText = "<id>6<id><src>6<src><target>14<target><payload>1,12<payload>";
    
    tests[i].expectedResult.id = 0;    // NOTE: Without ID there's we cant't evaluate the others
    tests[i].expectedResult.isValid = false;
    tests[i].expectedResult.source = 0;
    tests[i].expectedResult.target = 0;
    tests[i].expectedResult.payloadText = NULL;
    tests[i].expectedResult.payload = NULL;
    
    tests[i].isPayloadInt = false;
    tests[i].isPayloadFloat = false;
    tests[i].isPayloadIntList = false;
    tests[i].payloadListLength = 0;
    i++;

    /* - New Test ----------------------- */
    
    // NOTE: Without the message delimiter we don't know what to read...
    
    tests[i].title = "Broken <msg> tag";
    tests[i].isVerbose = isVerbose;
    tests[i].messageText = "<msg><id>6<id><src>6<src><target>14<target><payload>1,12<payload>";
    
    tests[i].expectedResult.id = 0;    // NOTE: Without ID there's we cant't evaluate the others
    tests[i].expectedResult.isValid = false;
    tests[i].expectedResult.source = 0;
    tests[i].expectedResult.target = 0;
    tests[i].expectedResult.payloadText = NULL;
    tests[i].expectedResult.payload = NULL;
    
    tests[i].isPayloadInt = false;
    tests[i].isPayloadFloat = false;
    tests[i].isPayloadIntList = false;
    tests[i].payloadListLength = 0;
    i++;

    /* - New Test ----------------------- */
    
    // NOTE: Without the message delimiter we don't know what to read...
    
    tests[i].title = "Unrecognized tag <message>";
    tests[i].isVerbose = isVerbose;
    tests[i].messageText = "<message><id>6<id><src>6<src><target>14<target><payload>1,12<payload><message>";
    
    tests[i].expectedResult.id = 0;
    tests[i].expectedResult.isValid = false;
    tests[i].expectedResult.source = 0;
    tests[i].expectedResult.target = 0;
    tests[i].expectedResult.payloadText = NULL;
    tests[i].expectedResult.payload = NULL;
    
    tests[i].isPayloadInt = false;
    tests[i].isPayloadFloat = false;
    tests[i].isPayloadIntList = false;
    tests[i].payloadListLength = 0;
    i++;

    /* - New Test ----------------------- */

    // NOTE: Without a vakud ID there's we cant't evaluate the others
    
    tests[i].title = "Missing <id> tag";
    tests[i].isVerbose = isVerbose;
    tests[i].messageText = "<msg><src>6<src><target>14<target><payload>1,12<payload><msg>";
    
    tests[i].expectedResult.id = 0;
    tests[i].expectedResult.isValid = false;
    tests[i].expectedResult.source = 0;
    tests[i].expectedResult.target = 0;

    tests[i].expectedResult.payload = NULL;
    tests[i].expectedResult.payloadText = "1,12";

    tests[i].isPayloadInt = false;
    tests[i].isPayloadFloat = false;
    tests[i].isPayloadIntList = false;
    tests[i].payloadListLength = j + 1;
    i++;

    /* - New Test ----------------------- */
    tests[i].title = "Unrecognized tag <source>";
    tests[i].isVerbose = isVerbose;
    tests[i].messageText = "<msg><id>6<id><source>6<source><target>14<target><payload>12.32<payload><msg>";
    
    tests[i].expectedResult.id = MSG_RES_INF;
    tests[i].expectedResult.isValid = false;
    tests[i].expectedResult.source = 0;
    tests[i].expectedResult.target = 14;
    tests[i].expectedResult.payloadText = "12.32";
    
    tests[i].expectedResult.payload = (float *)malloc(sizeof(float));
    *(float *)tests[i].expectedResult.payload = 12.32;
    
    tests[i].isPayloadInt = false;
    tests[i].isPayloadFloat = false;
    tests[i].isPayloadIntList = true;
    tests[i].payloadListLength = j + 1;
    i++;

    /* - New Test ----------------------- */
    tests[i].title = "Mismatching tag";
    tests[i].isVerbose = isVerbose;
    tests[i].messageText = "<msg><id>6<src><id>6<src><target>14<target><payload>12.32<payload><msg>";
    
    tests[i].expectedResult.id = 0;    // NOTE: Without ID there's we cant't evaluate the others
    tests[i].expectedResult.isValid = false;
    tests[i].expectedResult.source = 0;
    tests[i].expectedResult.target = 14;
    
    tests[i].expectedResult.payloadText = "12,32";
    tests[i].expectedResult.payload = NULL;
    
    tests[i].isPayloadInt = false;
    tests[i].isPayloadFloat = false;
    tests[i].isPayloadIntList = true;
    tests[i].payloadListLength = j + 1;
    i++;

    /* - New Test ----------------------- */
    tests[i].title = "Mismatching tag";
    tests[i].isVerbose = isVerbose;
    tests[i].messageText = "<msg><id>6<src><id>6<src><target>14<target><payload>12.32<payload><msg>";
    
    tests[i].expectedResult.id = 0;    // NOTE: Without ID there's we cant't evaluate the others
    tests[i].expectedResult.isValid = false;
    tests[i].expectedResult.source = 0;
    tests[i].expectedResult.target = 14;
    
    j = -1;
    tests[i].expectedResult.payloadText = "12.32";
    tests[i].expectedResult.payload = NULL;
    
    tests[i].isPayloadInt = false;
    tests[i].isPayloadFloat = false;
    tests[i].isPayloadIntList = true;
    tests[i].payloadListLength = j + 1;
    i++;

    /* >> Test em'all! ------------------------>> */
    aux = tstMsgExtractionBatch(tests, i, "Bad (tag problems)");
    finalResult.nFailures += aux.nFailures;
    finalResult.nTests += aux.nTests;
    printf("\n");

    return finalResult;
}

TestResult tstMsgExtractionBadContents(void) {

    printf("\n");
    printf("\n>> ---------------------------------------------------------------------------- >>");
    printf("\n>> TEST: Extract message from text (bad MSG_REQ_ADD requests) ----------------- >>");
    printf("\n>> ---------------------------------------------------------------------------- >>");
    
    TestResult aux = { 0, 0 };
    TestResult finalResult = { 0, 0 };

    ExtractionTest tests[10];
    bool isVerbose = false;
    
    int i = 0;

    /* - New Test ----------------------- */
    tests[i].title = "Bad MSG_REQ_ADD: Unexpected target";
    tests[i].isVerbose = isVerbose;
    tests[i].messageText = "<msg><id>1<id><target>2<target><msg>";
    
    tests[i].expectedResult.id = MSG_REQ_ADD;
    tests[i].expectedResult.isValid = false;
    tests[i].expectedResult.source = 0;
    tests[i].expectedResult.target = 0;

    tests[i].expectedResult.payloadText = NULL;
    tests[i].expectedResult.payload = NULL;
    tests[i].isPayloadInt = false;
    tests[i].isPayloadFloat = false;
    tests[i].isPayloadIntList = false;
    tests[i].payloadListLength = 0;
    i++;

    /* - New Test ----------------------- */
    tests[i].title = "Bad MSG_REQ_ADD: Unexpected source";
    tests[i].isVerbose = isVerbose;
    tests[i].messageText = "<msg><id>1<id><src>2<src><msg>";
    
    tests[i].expectedResult.id = MSG_REQ_ADD;
    tests[i].expectedResult.isValid = false;
    tests[i].expectedResult.source = 0;
    tests[i].expectedResult.target = 0;

    tests[i].expectedResult.payloadText = NULL;
    tests[i].expectedResult.payload = NULL;
    tests[i].isPayloadInt = false;
    tests[i].isPayloadFloat = false;
    tests[i].isPayloadIntList = false;
    tests[i].payloadListLength = 0;
    i++;

    /* - New Test ----------------------- */
    tests[i].title = "Bad MSG_REQ_ADD: Unexpected payload";
    tests[i].isVerbose = isVerbose;
    tests[i].messageText = "<msg><id>1<id><payload>2<payload><msg>";
    
    tests[i].expectedResult.id = MSG_REQ_ADD;
    tests[i].expectedResult.isValid = false;
    tests[i].expectedResult.source = 0;
    tests[i].expectedResult.target = 0;

    tests[i].expectedResult.payloadText = "2";
    tests[i].expectedResult.payload = NULL;
    tests[i].isPayloadInt = false;
    tests[i].isPayloadFloat = false;
    tests[i].isPayloadIntList = false;
    tests[i].payloadListLength = 0;
    i++;

    /* >> Test em'all! ------------------------>> */
    aux = tstMsgExtractionBatch(tests, i, "Bad add request");
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

    nGroups++;
    aux = tstMsgPatternValidation();
    acc.nTests += aux.nTests;
    acc.nFailures += aux.nFailures;

    nGroups++;
    aux = tstMsgExtractionGood();
    acc.nTests += aux.nTests;
    acc.nFailures += aux.nFailures;

    nGroups++;
    aux = tstMsgExtractionBadTags();
    acc.nTests += aux.nTests;
    acc.nFailures += aux.nFailures;

    nGroups++;
    aux = tstMsgExtractionBadContents();
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