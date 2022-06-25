#pragma once

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

TestResult tstMsgPatternValidationBatch(const char **messages, const int nTests, const bool isValidMessage, const char *title);
TestResult tstMsgExtractionBatch(const ExtractionTest tests[], const int nTests, const char *title);