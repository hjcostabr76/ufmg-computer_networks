#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <regex.h>

/**
 * ------------------------------------------------
 * == CONSTS ======================================
 * ------------------------------------------------
 */

const char EQP_IDS[4][2] = { {"01"}, {"02"}, {"03"}, {"04"} };

/**
 * TODO: 2022-05-20 - Do we really need this?
 */
const char CMD_NAME[CMD_COUNT][15] = { {"add sensor"}, {"remove sensor"}, {"list sensors"}, {"read"}, {"kill"} };
const char CMD_PATTERN[CMD_COUNT][45] = {
    {"^add sensor (0[1234] ){1,4}in 0[1234]$"},
    {"^remove sensor (0[1234] ){1,4}in 0[1234]$"},
    {"^list sensors in 0[1234]$"},
    {"^read (0[1234] ){1,4}in 0[1234]$"},
    {"^kill$"}
};
const char CMD_PATTERN_END[] = " in ##";


/**
 * ------------------------------------------------
 * == DEBUG =======================================
 * ------------------------------------------------
 */

void comLogErrorAndDie(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

void comDebugStep(const char *text) {
	if (DEBUG_ENABLE)
		printf("%s", text);
}

/**
 * ------------------------------------------------
 * == MAIN ========================================
 * ------------------------------------------------
 */

Command getGenericCommand(void) {
    Command command;
    command.isValid = false;
    memset(command.name, '\0', sizeof(command.name));
    memset(command.equipment, '\0', sizeof(command.equipment));
    for (int i = 0; i < SENSOR_COUNT; i++)
        command.sensors[i] = false;
    return command;
}

Command getEmptyCommand(CmdCodeEnum code) {
    Command command = getGenericCommand();
    command.code = code;
    strcpy(command.name, CMD_NAME[code]);
    return command;
}

Command getCommand(const char* input) {

    Command cmd = getGenericCommand();
    
    // Identify command type
    for (int i = 0; i < CMD_COUNT; i++) {

        char regexMsg[100];
        cmd.isValid = strRegexMatch(CMD_PATTERN[i], input, regexMsg);
        if (!cmd.isValid)
            continue;

		cmd.isValid = true;
		cmd.code = i;
		strcpy(cmd.name, CMD_NAME[i]);
        break;
	}

	if (!cmd.isValid || cmd.code == CMD_CODE_KILL)
        return cmd;

    // Determine equipment
    char inputCopy[100];
    strcpy(inputCopy, input);

    int inputArgsC;
    char** inputArgs = strSplit(inputCopy, " ", 8, 100, &inputArgsC);
    strcpy(cmd.equipment, inputArgs[inputArgsC - 1]);

    if (cmd.code == CMD_CODE_LIST)
        return cmd;

    // Determine sensors
    for (int i = 0; i < SENSOR_COUNT; i++)
        cmd.sensors[i] = false;

    int firstSensorIdx = cmd.code == CMD_CODE_READ ? 1 : 2;
    for (int i = firstSensorIdx; i < inputArgsC - 2; i++) {
        int sensorCode = atoi(inputArgs[i]) - 1;
        if (cmd.sensors[sensorCode]) {
            cmd.isValid = false;
            break;
        }
        cmd.sensors[sensorCode] = true;
    }
    
    return cmd;
}

/**
 * ------------------------------------------------
 * == NETWORK =====================================
 * ------------------------------------------------
 */

short int netIsValidAddrFamily(const int addrFamily) {
	return (addrFamily == AF_INET || addrFamily == AF_INET6);
}

int netListen(const int port, const struct timeval *timeout, const int maxConnections) {

	// Validate params
	if (!port)
		comLogErrorAndDie("Invalid listening socket port");

	if (!maxConnections)
		comLogErrorAndDie("Listening socket must accept at least one connection");

	// Create socket
	const int sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock == -1)
        comLogErrorAndDie("Failure as creating listening socket [1]");

    // Avoid that port used in an execution become deactivated for 02 min after conclustion
    int enableAddrReuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enableAddrReuse, sizeof(int)) != 0)
        comLogErrorAndDie("Failure as creating listening socket [2]");

	// Sets linstening timeout
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, timeout, sizeof(*timeout)) != 0)
		comLogErrorAndDie("Failure as creating listening socket [3]");

	// Enable ipv4 connections in ipv6 socket
	int no = 0;
	if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&no, sizeof(no)) != 0)
		comLogErrorAndDie("Failure as trying to enable ipv4 clients to ipv6 server");

	// Bind socket
	struct sockaddr_storage storage;
	memset(&storage, 0, sizeof(storage));

	struct sockaddr_in6 *addr = (struct sockaddr_in6 *)&storage;
	memset(addr, 0, sizeof(*addr));

	addr->sin6_family = AF_INET6;
	addr->sin6_addr = in6addr_any; // Set bindings to occur in any host available address
	addr->sin6_port = htons(port); // Host to network short

	if (bind(sock, (struct sockaddr *)addr, sizeof(*addr)) != 0)
		comLogErrorAndDie("Failure as biding listening socket");
    
	// Start listening
	if (listen(sock, maxConnections) != 0)
		comLogErrorAndDie("Failure as starting to listen");
	
	return sock;
}

bool netSetSocketAddressString(int socket, char *addrStr) {

	struct sockaddr_storage storage;
	memset(&storage, 0, sizeof(storage));
	
	struct sockaddr *addr = (struct sockaddr *)&storage;
	memset(addr, 0, sizeof(*addr));

	socklen_t socketLength = sizeof(addr);
	getsockname(socket, (struct sockaddr*)addr, &socketLength);

	if (((struct sockaddr *)addr)->sa_family == AF_INET) {
		inet_ntop(AF_INET, &(((struct sockaddr_in *)addr)->sin_addr), addrStr, INET_ADDRSTRLEN + 1);
	} else if (((struct sockaddr *)addr)->sa_family == AF_INET6) {
		inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)addr)->sin6_addr), addrStr, INET6_ADDRSTRLEN + 1);
	} else
		return false;

	return true;
}

/**
 * ------------------------------------------------
 * == STRING ======================================
 * ------------------------------------------------
 */

bool strValidateNumeric(const char *string, const int strLength) {
	for (int i; i < strLength; i++) {
		if ((int)string[i] < ASCII_NUMBER_FIRST || (int)string[i] > ASCII_NUMBER_LAST)
			return false;
	}
	return true;
}

bool strReadFromStdIn(char *buffer, size_t buffLength) {
    if (!fgets(buffer, buffLength, stdin))
		return false; // Error...
	int lastCharIdx = strcspn(buffer, "\n");
	buffer[lastCharIdx] = '\0';
	return true;
}

bool strStartsWith(const char *target, const char *prefix) {
   return strncmp(target, prefix, strlen(prefix)) == 0;
}

void strGetSubstring(const char *src, char *dst, size_t start, size_t end) {
    strncpy(dst, src + start, end - start);
}

char** strSplit(char* source, const char delimiter[1], const int maxTokens, const int maxLength, int *tokensCount) {
    
    *tokensCount = 0;
    char** tokens = malloc(maxTokens * sizeof(char*));
    
    char *token = strtok(source, delimiter);
    if (token == NULL)
        return tokens;

    while (token != NULL && *tokensCount < maxTokens) {

        tokens[*tokensCount] = malloc(maxLength * sizeof(char));
        memset(tokens[*tokensCount], '\n', maxLength * sizeof(char));
        strcpy(tokens[*tokensCount], token);
        
        *tokensCount = *tokensCount + 1;
        token = strtok(NULL, delimiter);
    }

    return tokens;
}

bool strRegexMatch(const char* pattern, const char* str, char errorMsg[100]) {

    regex_t regex;
    memset(errorMsg, 0, 100);
    
    // Compile
    int regStatus = regcomp(&regex, pattern, REG_EXTENDED);
    if (regStatus != 0) {
        sprintf(errorMsg, "Compiling error");
        return false;
    }

    // Execute
    regStatus = regexec(&regex, str, 0, NULL, 0);
    
    bool isSuccess = regStatus == 0;
    if (!isSuccess && regStatus != REG_NOMATCH) { // Error
        char aux[100];
        regerror(regStatus, &regex, aux, 100);
        sprintf(errorMsg, "Match error: %s\n", aux);
    }

    // Free memory allocated to the pattern buffer by regcomp()
    regfree(&regex);
    return isSuccess;
}
