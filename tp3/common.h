#pragma once

// #include <stdio.h>
// #include <stdlib.h>
#include <stdbool.h>

// #include <sys/socket.h>
// #include <arpa/inet.h>
#include <sys/select.h>
// #include <unistd.h>

// #include <string.h>
// #include <time.h>
// #include <errno.h>
// #include <pthread.h>
// #include <inttypes.h>

#define DEBUG_ENABLE 1
#define MAX_CONNECTIONS 1
#define TIMEOUT_CONN_SECS 15

/**
 * TODO: 2022-05-13 - Delete unused consts...
 */
#define TIMEOUT_TRANSFER_SECS 2

#define BUF_SIZE 500
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


/**
 * TODO: 2022-05-13 - ADD Descricao
 */
// int posixConnect(const int port, const char *addrStr, const struct timeval *timeout);

/**
 * TODO: 2022-05-13 - ADD Descricao
 */

int posixListen(const int port, const struct timeval *timeout, const int maxConnections);

/**
 * TODO: 2022-05-13 - ADD Descricao
 */
bool posixSetSocketAddressString(int socket, char *boundAddr);