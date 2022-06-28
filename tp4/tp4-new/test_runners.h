#pragma once

#include "common.h"
#include "test_utils.h"

typedef struct {
    char *title;
    char *messageText;
    Message expectedResult;
    bool isVerbose;
    PayloadDescription payloadDesc;
} ExtractionTest;

TestResult tstMsgPatternValidationBatch(const char **messages, const int nTests, const bool isValidMessage, const char *title);
TestResult tstMsgExtractionBatch(const ExtractionTest tests[], const int nTests, const char *title);