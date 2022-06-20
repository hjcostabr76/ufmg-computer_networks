#include "common.h"

// #include <stdio.h>
#include <stdlib.h>
// #include <pthread.h>
// #include <sys/socket.h>
// #include <inttypes.h>
// #include <arpa/inet.h>
// #include <sys/time.h>
#include <string.h>
// #include <unistd.h>
// #include <errno.h>

typedef struct {
    int nTests;
    int nSuccesses;
} TestResult;

typedef struct {
    int nTests;
    int nSuccesses;
    TestResult tests[20];
} TestBatchResult;

// bool isBatchSuccess(TestBatchResult result) {
//     int accSuccess = 0;
//     for (int i = 0; i < result.nTests; i++) {
//         if (result.tests[i] != NULL)
//             accSuccess += result.tests[i].nSuccesses;
//     }
//     return accSuccess == result.nTests;
// }

TestResult testBatch(const char **messages, const int nTests, const bool isValidMessage) {

    char testType[15] = "";
    strcpy(testType, isValidMessage ? "Good\0" : "Bad\0");
    
    int nSuccesses = 0;
    for (int i = 0; i < nTests; i++) {
        
        printf("\nTesting %s pattern:\n\t\"%s\"...", testType, messages[i]);
        const bool passedValidation = validateReceivedMsg(messages[i]);
        const bool isSuccess = (passedValidation && isValidMessage) || (!passedValidation && !isValidMessage);
        if (isSuccess) {
            nSuccesses++;
        }
        
        char resultMsg[10];
        strcpy(resultMsg, isSuccess ? "OK" : "FAILED!");
        printf("\n\tTest %s", resultMsg);
    }

    printf("\n");
    
    // const TestResult result = { nTests, nSuccesses };
    const TestResult result = { nTests, nSuccesses, NULL };
    // TestResult result;
    // result.nTests = nTests;
    // result.nSuccesses = nSuccesses;
    // result.tests = NULL;
    return result;
}

TestBatchResult testRename(void) {

    printf("\n\n>>>>>>>>>> Tests for regex >>>>>>>>>>\n\n");
    
    // int resultSize = 20 * sizeof(TestResult);
    // TestResult* allResults = (TestResult *)malloc(resultSize);
    // memset(allResults, NULL, resultSize);

    int aux = 20;
    TestResult resultAux = { 0, 0 };
    TestResult partialResults[20] = { 0, 0 };

    TestBatchResult finalResult = { 0, -1, { 0, 0 } };
    // finalResult.nSuccesses = 0;
    // finalResult.nTests = -1;
    // finalResult.tests = { 0, 0 };

    

    /* ---------------- Good ----------------  */
    bool isValid = true;
    
    finalResult.nTests++;
    int nPatterns = 2;
    const char *goodMessages[] = {
        "<msg><id>1<id><src>2<src><target>3<target><msg>",
        "<msg><id>1<id><src>2<src><target>3<target><payload>Loren Ipsun 123 Dolur<payload><msg>"
    };
    resultAux = testBatch(goodMessages, nPatterns, isValid);
    finalResult.nSuccesses += (resultAux.nTests == resultAux.nSuccesses);
    partialResults[finalResult.nTests] = resultAux;
    printf("\n");

    /* - Bad: Wrong Tag names ---------------  */
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
    resultAux = testBatch(badMessagesWrongTag, nPatterns, isValid);
    finalResult.nSuccesses += (resultAux.nTests == resultAux.nSuccesses);
    partialResults[finalResult.nTests] = resultAux;
    printf("\n");

    /* - Bad: Missing close tags ------------  */
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
    resultAux = testBatch(badMessagesClosingTags, nPatterns, isValid);
    finalResult.nSuccesses += (resultAux.nTests == resultAux.nSuccesses);
    partialResults[finalResult.nTests] = resultAux;
    printf("\n");

    /* - Bad: Missing required fields -------  */
    finalResult.nTests++;
    nPatterns = 4;
    const char *badMessagesMissingFields[] = {
        "<msg><id><id><src>2<src><target>3<target><msg>",
        "<msg><id>1<id><src><src><target>3<target><msg>",
        "<msg><id>1<id><src>2<src><target><target><msg>",
        "<msg><id>1<id><src>2<src><target>3<target><payload><payload><msg>",
    };
    resultAux = testBatch(badMessagesMissingFields, nPatterns, isValid);
    finalResult.nSuccesses += (resultAux.nTests == resultAux.nSuccesses);
    partialResults[finalResult.nTests] = resultAux;
    printf("\n");
    
    /* - Bad: Extra characters --------------  */
    finalResult.nTests++;
    nPatterns = 5;
    const char *badMessagesExtraChars[] = {
        "<msg><id>123<id><src>2<src><target>3<target><msg>",
        "<msg><id>1<id><src>123<src><target>3<target><msg>",
        "<msg><id>1<id><src>2<src><target>123<target><msg>",
        "<msg><id>1<id><src>2<src><target>3<target><payload>Loren Ipsun Dolur2<payload><msg>",
        "<msg><id>1<id><src>2<src><target>3<target><payload>1Loren Ipsun Dolur2<payload><msg>"
    };
    resultAux = testBatch(badMessagesMissingFields, nPatterns, isValid);
    finalResult.nSuccesses += (resultAux.nTests == resultAux.nSuccesses);
    partialResults[finalResult.nTests] = resultAux;
    printf("\n");
    
    /* - Bad: Invalid characters ------------  */
    finalResult.nTests++;
    nPatterns = 3;
    const char *badMessagesInvalidChars[] = {
        "<msg><id>1a<id><src>2<src><target>3<target><msg>",
        "<msg><id>1<id><src>2b<src><target>3<target><msg>",
        "<msg><id>1<id><src>2<src><target>3c<target><msg>"
    };
    resultAux = testBatch(badMessagesInvalidChars, nPatterns, isValid);
    finalResult.nSuccesses += (resultAux.nTests == resultAux.nSuccesses);
    partialResults[finalResult.nTests] = resultAux;
    printf("\n");

    /* - Bad: Unexpected spaces -------------  */
    nPatterns = 3;
    finalResult.nTests++;
    const char *badMessagesSpaces[] = {
        "<msg><id>1 <id><src>2<src><target>3<target><msg>",
        "<msg><id>1<id><src> <src><target>3<target><msg>",
        "<msg><id>1<id><src>2<src><target>3 <target><msg>"
    };
    resultAux = testBatch(badMessagesSpaces, nPatterns, isValid);
    finalResult.nSuccesses += (resultAux.nTests == resultAux.nSuccesses);
    partialResults[finalResult.nTests] = resultAux;
    printf("\n");
    
    return finalResult;
}

int main() {

    bool isSuccess = true;

    TestBatchResult foo = testRename();
    isSuccess = isSuccess && (foo.nTests == foo.nSuccesses);
    
    return isSuccess ? EXIT_SUCCESS : EXIT_FAILURE;
}