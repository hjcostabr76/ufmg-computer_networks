#pragma once

#include <stdbool.h>
#include <sys/select.h>

#include <stdio.h>

/**
 * ------------------------------------------------
 * == CONSTS ======================================
 * ------------------------------------------------
 */

#define DEBUG_ENABLE 1
#define BUF_SIZE 500

#define MAX_CONNECTIONS 1
#define TIMEOUT_CONN_SECS 15
#define TIMEOUT_TRANSFER_SECS 15

#define CMD_COUNT 5
#define SENSOR_COUNT 4

/**
 * TODO: 2022-05-20 - Check if we need it all
 */

#define ASCII_NUMBER_FIRST 48
#define ASCII_NUMBER_LAST 57
#define ASCII_LC_LETTER_LAST 122
#define ASCII_LC_LETTER_FIRST 97

extern const char EQP_IDS[4][2];
extern const char CMD_NAME[5][15];
extern const char CMD_PATTERN[CMD_COUNT][45];

typedef enum { CMD_CODE_ADD, CMD_CODE_RM, CMD_CODE_LIST, CMD_CODE_READ, CMD_CODE_KILL } CmdCodeEnum;

/**
 * ------------------------------------------------
 * == ABSTRACTS ===================================
 * ------------------------------------------------
 */

typedef struct {
    bool isValid;
    CmdCodeEnum code;
    char name[15]; // TODO: Is it really necessary?
    char equipment[3];
    bool sensors[SENSOR_COUNT];
} Command;

typedef struct{
    char id[3];
    bool sensors[SENSOR_COUNT];
} Equipment;


/**
 * ------------------------------------------------
 * == HEADERS =====================================
 * ------------------------------------------------
 */

/** -- DEBUG ------------ */

void comDebugStep(const char *text);
void comLogErrorAndDie(const char *msg);

/** -- MAIN ------------- */

Command getGenericCommand(void);
Command getEmptyCommand(CmdCodeEnum code);
Command getCommand(const char* input);

/** -- NETWORK ---------- */

int netListen(const int port, const int timeoutSecs, const int maxConnections);
int netConnect(const int port, const char *addrStr, const int timeoutSecs);
bool netSend(const int socket, const char *buffer, const unsigned bytesToSend);
int netAccept(const int servSocket);
ssize_t netRecv(const int cliSocket, char *buffer, const int timeoutSecs);
int netGetIpType(const char *ipTypeStr);
bool netSetSocketAddressString(int socket, char *boundAddr);

/** -- STRING ----------- */

// int comValidateLCaseString(const char *string, const int strLength);
bool strReadFromStdIn(char *buffer, size_t buffLength);
bool strRegexMatch(const char* pattern, const char* str, char errorMsg[100]);
bool strStartsWith(const char *a, const char *b);
bool strStartsWith(const char *target, const char *prefix);
bool strValidateNumeric(const char *string, const int strLength);
char** strSplit(char* source, const char delimiter[1], const int maxTokens, const int maxLength, int *tokensCount);
void strGetSubstring(const char *src, char *dst, size_t start, size_t end);
