#pragma once

#define DEBUG_ENABLE 0

#define TIMEOUT_CONN_SECS 15
#define TIMEOUT_TRANSFER_SECS 2

#define BUF_SIZE 1024
#define SIZE_NUMBER_STR 10

#define ASCII_NUMBER_FIRST 48
#define ASCII_NUMBER_LAST 57
#define ASCII_LC_LETTER_LAST 122
#define ASCII_LC_LETTER_FIRST 97

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
void comLogErrorAndDie(const char *msg);

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
void comDebugStep(const char *text);

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
int comValidateLCaseString(const char *string, const int strLength);

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
int comValidateNumericString(const char *string, const int strLength);
