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

#define BUF_SIZE 1024

#define TIMEOUT_CONN_SECS 15
#define TIMEOUT_TRANSFER_SECS 15

#define ASCII_NUM_FIRST 48
#define ASCII_NUM_LAST 57
#define ASCII_CHAR_LC_FIRST 97
#define ASCII_CHAR_LC_LAST 122
#define ASCII_CHAR_UC_FIRST 65
#define ASCII_CHAR_UC_LAST 90

#define CMD_COUNT 3
#define PORT_DEFAULT 51511

#define MAX_PAYLOAD_SIZE 1024 - 1 - 2 - 2 // [buffer size] - [id_msg] - [id_src] - [id_target]
#define MAX_CONNECTIONS 15

extern const char* EQUIP_IDS[MAX_CONNECTIONS];
extern const char* CMD_NAME[CMD_COUNT];

extern const char* NET_TAG_MSG;
extern const char* NET_TAG_ID;
extern const char* NET_TAG_SRC;
extern const char* NET_TAG_TARGET;
extern const char* NET_TAG_PAYLOAD;

typedef enum { CMD_CODE_KILL, CMD_CODE_LIST, CMD_CODE_INFO } CommandCodeEnum;

typedef enum { OK_RM = 1 } OkMessageCodeEnum;

typedef enum {
    ERR_NOT_FOUND = 1,
    ERR_NOT_FOUND_SRC,
    ERR_NOT_FOUND_TARGET,
    ERR_MAX_EQUIP,
} ErrorCodeEnum;

typedef enum {
    MSG_REQ_ADD = 1,
    MSG_REQ_RM,
    MSG_RES_ADD,
    MSG_RES_LIST,
    MSG_REQ_INF,
    MSG_RES_INF,
    MSG_ERR,
    MSG_OK
    // MSG_REQ_LIST,
    // MSG_RES_RM,
} MessageIdEnum;

/**
 * ------------------------------------------------
 * == ABSTRACTS ===================================
 * ------------------------------------------------
 */

typedef struct {
    MessageIdEnum id;
    int source;
    int target;
    void *payload;
    char *payloadText;
    bool isValid;
} Message;

/**
 * ------------------------------------------------
 * == HEADERS =====================================
 * ------------------------------------------------
 */

/** -- DEBUG ------------ */

void comDebugStep(const char *text);
void comLogErrorAndDie(char *msg);

/** -- MAIN ------------- */

bool isValidReceivedMsg(const char *message);
void setMessageFromText(const char *text, Message *message);
// Equipment getEmptyEquipment(void);
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
bool netSetSocketAddrString(const int sock, char *addrStr);

/** -- STRING ----------- */

// int comValidateLCaseString(const char *string, const int strLength);
bool strReadFromStdIn(char *buffer, size_t buffLength);
bool strRegexMatch(const char* pattern, const char* str, char errorMsg[100]);
bool strEndsWith(const char *target, const char *suffix);
bool strStartsWith(const char *target, const char *prefix);
bool strIsNumeric(const char *string);
bool strIsAlphaNumericChar(const char c);
// char* strTrim(const char *string);
char** strSplit(const char* source, const char delimiter, const int maxTokens, const int maxLength, int *tokensCount);
void strSubstring(const char *src, char *dst, size_t start, size_t end);
bool strSetDelimitedTextBounds(const char* src, const char *delimiter, int *begin, int *end);
