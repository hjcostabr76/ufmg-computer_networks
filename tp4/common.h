#pragma once

#include <stdbool.h>
#include <sys/select.h>
#include <stdio.h>

/**
 * ------------------------------------------------
 * == CONSTS ======================================
 * ------------------------------------------------
 */

#define DEBUG_ENABLE true
#define BUF_SIZE 500
#define PORT_DEFAULT 51511

#define MAX_CONNECTIONS 15
#define TIMEOUT_CONN_SECS 15
#define TIMEOUT_TRANSFER_SECS 15

#define CMD_COUNT 5
// #define MAX_SENSORS 15

// #define ASCII_NUM_FIRST 48
// #define ASCII_NUM_LAST 57
// #define ASCII_CHAR_LC_FIRST 97
// #define ASCII_CHAR_LC_LAST 122
// #define ASCII_CHAR_UC_FIRST 65
// #define ASCII_CHAR_UC_LAST 90

extern const char* EQUIP_IDS[MAX_CONNECTIONS];

// extern const char* CMD_NAME[5];
// extern const char* CMD_PATTERN[CMD_COUNT];

// typedef enum { CMD_CODE_ADD, CMD_CODE_RM, CMD_CODE_LIST, CMD_CODE_READ, CMD_CODE_KILL } CmdCodeEnum;
// typedef enum { ERR_CMD_INVALID = 1, ERR_EQUIP_INVALID, ERR_SENSOR_INVALID, ERR_SENSOR_REPEATED, ERR_SENSOR_LIMIT } ErrCodeEnum;

/**
 * ------------------------------------------------
 * == ABSTRACTS ===================================
 * ------------------------------------------------
 */

// typedef struct {
//     ErrCodeEnum error;
//     CmdCodeEnum code;
//     int equipCode;
//     bool sensors[SENSOR_COUNT];
// } Command;

typedef struct {
    int id;
} Equipment;


/**
 * ------------------------------------------------
 * == HEADERS =====================================
 * ------------------------------------------------
 */

/** -- DEBUG ------------ */

void comDebugStep(const char *text);
void comLogErrorAndDie(char *msg);

/** -- MAIN ------------- */

Equipment getEmptyEquipment(void);
// Command getGenericCommand(void);
// Command getEmptyCommand(CmdCodeEnum code);
// Command getCommand(const char* input);

/** -- NETWORK ---------- */

int netListen(const char *portStr, const int timeoutSecs, const int maxConnections, const int *ipVersion);
int netConnect(const char *portStr, const char *addrStr, const int timeoutSecs, const int *ipVersion);
bool netSend(const int socket, const char *msg);
int netAccept(const int servSocket);
ssize_t netRecv(const int cliSocket, char *buffer, const int timeoutSecs);
int netGetIpType(const char *ipTypeStr);
bool netSetSocketAddressString(int socket, char *boundAddr);

/** -- STRING ----------- */

// int comValidateLCaseString(const char *string, const int strLength);
// bool strReadFromStdIn(char *buffer, size_t buffLength);
// bool strRegexMatch(const char* pattern, const char* str, char errorMsg[100]);
bool strEndsWith(const char *target, const char *suffix);
// bool strStartsWith(const char *target, const char *prefix);
bool strIsNumeric(const char *string);
// bool strIsAlphaNumericChar(const char c);
// char* strTrim(const char *string);
// char** strSplit(char* source, const char delimiter[1], const int maxTokens, const int maxLength, int *tokensCount);
// void strGetSubstring(const char *src, char *dst, size_t start, size_t end);
