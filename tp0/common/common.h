#pragma once

#define DEBUG_ENABLE 1
#define TIMEOUT_SECS 15
#define SIZE_BUFFER 1024

#define ASCII_NUMBER_FIRST 48
#define ASCII_NUMBER_LAST 57
#define ASCII_LC_LETTER_LAST 122
#define ASCII_LC_LETTER_FIRST 97

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
void commonLogErrorAndDie(const char *msg);

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
void commonDebugStep(const char *text);