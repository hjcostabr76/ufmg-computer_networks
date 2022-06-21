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
} ExtractionTest;


bool setMessageFromText(const char *text, Message *message) {

    int begin = 0;
    int end = 0;
    
    const int msgSize = (int)strlen(text);
    char *temp = (char *)malloc(msgSize);
    temp[0] = '\0';

    // Msg
    // if (!strSetDelimitedTextBounds(text, NET_TAG_MSG, &begin, &end))
    //     return false;
    // strGetSubstring(text, temp, begin, end);
    // strcpy(temp, text);

    // Id
    if (!strSetDelimitedTextBounds(text, NET_TAG_ID, &begin, &end))
        return false;
    strSubstring(text, temp, begin, end);
    message->id = atoi(temp);

    // Src
    if (!strSetDelimitedTextBounds(text, NET_TAG_SRC, &begin, &end))
        return false;
    strSubstring(text, temp, begin, end);
    message->source = atoi(temp);

    // target
    if (!strSetDelimitedTextBounds(text, NET_TAG_TARGET, &begin, &end))
        return false;
    strSubstring(text, temp, begin, end);
    message->target = atoi(temp);

    // Payload
    // TODO: 2022-06-21 - How will we do this??
    if (!strSetDelimitedTextBounds(text, NET_TAG_PAYLOAD, &begin, &end))
        return false;
    strSubstring(text, temp, begin, end);

    message->payload = (char *)malloc(strlen(temp));
    strcpy(message->payload, temp);

    return true;
}


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
        
        // Test result
        const bool isSuccess = (
            (!isParsingOk && !isValidCase)
            || (
                isParsingOk
                && isValidCase
                && message.id == test.expectedResult.id
                && message.source == test.expectedResult.source
                && message.target == test.expectedResult.target
                && (
                    (message.payload == NULL && test.expectedResult.payload == NULL)
                    || (
                        message.payload != NULL
                        && test.expectedResult.payload != NULL
                        && strcmp(message.payload, test.expectedResult.payload) == 0
                    )
                )
            )
        );
        
        if (!isSuccess) {
            nFailures++;
        }

        // Exhibit result
        char resultMsg[10];
        strcpy(resultMsg, isSuccess ? "ok" : "FAILED!");
        printf("\n[test %s]", resultMsg);

        const bool showDetails = test.isVerbose || !isSuccess;
        if (showDetails) {
            
            printf("\n\n\t--- What came: -------------------");
            printf("\n\tmessage.id: '%d'", message.id);
            printf("\n\tmessage.source: '%d'", message.source);
            printf("\n\tmessage.target: '%d'", message.target);

            if (message.payload == NULL)
                printf("\n\tmessage.payload: 'NULL'");
            else
                printf("\n\tmessage.payload: '%s'", (char *)message.payload);
            
            printf("\n\n\t--- What was supposed to come: ---");
            printf("\n\ttest.expectedResult.id: '%d'", test.expectedResult.id);
            printf("\n\ttest.expectedResult.source: '%d'", test.expectedResult.source);
            printf("\n\ttest.expectedResult.target: '%d'", test.expectedResult.target);
            
            if (test.expectedResult.payload == NULL)
                printf("\n\ttest.expectedResult.payload: 'NULL'");
            else
                printf("\n\ttest.expectedResult.payload: '%s'", (char *)test.expectedResult.payload);
            printf("\n");
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

    /* - New Test ------------- */
    bool isValid = true;
    bool isVerbose = true;
    
    ExtractionTest goodTests[10];
    int i = 0;

    goodTests[i].isVerbose = isVerbose;
    goodTests[i].messageText = "<msg><id>1<id><src>2<src><target>3<target><msg>";
    goodTests[i].expectedResult.id = 1;
    goodTests[i].expectedResult.source = 2;
    goodTests[i].expectedResult.target = 3;
    goodTests[i].expectedResult.payload = NULL;
    i++;

    /* - New Test ------------- */
    goodTests[i].isVerbose = isVerbose;
    goodTests[i].messageText = "<msg><id>1<id><src>2<src><target>3<target><payload>Loren Ipsun 123 Dolur<payload><msg>";
    goodTests[i].expectedResult.id = 1;
    goodTests[i].expectedResult.source = 2;
    goodTests[i].expectedResult.target = 3;
    goodTests[i].expectedResult.payload = "Loren Ipsun 123 Dolur";
    i++;

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