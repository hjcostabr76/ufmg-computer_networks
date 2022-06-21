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

bool setContentTagBounds(const char* src, const char *delimiter, int *begin, int *end) {

    // Validate
    if (begin == NULL || end == NULL) {
        comDebugStep("Give something to work with (null values for begin and/or end)...");
        return false;
    }

    const int msgSize = strlen(src);
    const int delimiterSize = strlen(delimiter);

    if (!msgSize || !delimiterSize || delimiterSize > msgSize) {
        char msg[60] = "";
        sprintf(msg, "[Error!] 'msgSize' / 'delimiterSize' nonsense (%d / %d)...", msgSize, delimiterSize);
        comDebugStep(msg);
        return false;
    }

    // Seek for the message we're looking for
    *begin = -1;
    *end = -1;

    int i = 0;
    char *temp = (char *)malloc(msgSize);
    temp[0] = '\0';

    do {

        // Check for a match
        strGetSubstring(src, temp, i, msgSize);
        
        const bool isMatch = strStartsWith(temp, delimiter);
        if (!isMatch) {
            i += 1;
            continue;
        }
        
        // Update search state
        if (*begin >= 0) {
            *end = i;
            break;
        }
        
        i += delimiterSize;
        *begin = i;

    } while (i < msgSize);

    return (*begin >= 0 && *end > *begin);
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

typedef struct {
    char *message;
    char *title;
    Message expectedResult;
    bool isValid;
} ExtractionTest;


bool setMessageFromText(const char *text, Message *message) {

    int begin = 0;
    int end = 0;
    
    const int msgSize = (int)strlen(text);
    char *temp = (char *)malloc(msgSize);
    temp[0] = '\0';

    // Msg
    // if (!setContentTagBounds(text, NET_TAG_MSG, &begin, &end))
    //     return false;
    // strGetSubstring(text, temp, begin, end);
    // strcpy(temp, text);

    // Id
    if (!setContentTagBounds(text, NET_TAG_ID, &begin, &end))
        return false;
    strGetSubstring(text, temp, begin, end);
    message->id = atoi(temp);

    // Src
    if (!setContentTagBounds(text, NET_TAG_SRC, &begin, &end))
        return false;
    strGetSubstring(text, temp, begin, end);
    message->source = atoi(temp);

    // target
    if (!setContentTagBounds(text, NET_TAG_SRC, &begin, &end))
        return false;
    strGetSubstring(text, temp, begin, end);
    message->target = atoi(temp);

    // Payload
    /**
     * TODO: 2022-06-21 - How will we do this??
     */
    
    // if (!setContentTagBounds(messageText, NET_TAG_SRC, &begin, &end))
    //     return false;
    // strGetSubstring(messageText, temp, begin, end);
    // result->target = atoi(temp);

    return true;
}

int main() {

    printf("\n");
    printf("\n>> ---------------------------------------------------------------------------- >>");
    printf("\n>> Running tests... >> Running tests... >> Running tests... >> Running tests... >>");
    printf("\n>> ---------------------------------------------------------------------------- >>");

    // Run test groups
    TestResult acc = { 0, 0 };
    // TestResult aux;
    int nGroups = 0;

    nGroups++;
    const char tag[] = "<test>";
    const char message[] = "<test>Loren Ipsun Dolur<test>";

    int begin = 0;
    int end = 0;
    bool isSuccessTemp = setContentTagBounds(message, tag, &begin, &end);
    
    char content[100] = "";
    strGetSubstring(message, content, begin, end);
    printf("\nbeing: '%d', end: '%d', content: '%s'\n", begin, end, content);

    acc.nTests += 1;
    acc.nFailures += !isSuccessTemp;    

    // nGroups++;
    // aux = testMsgPatternValidation();
    // acc.nTests += aux.nTests;
    // acc.nFailures += aux.nFailures;

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