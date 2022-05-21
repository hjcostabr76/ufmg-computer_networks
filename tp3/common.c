#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <regex.h>
#include <errno.h>

/**
 * ------------------------------------------------
 * == CONSTS ======================================
 * ------------------------------------------------
 */

const char* NET_END_SEQ = "#_#";
const char* SENSOR_IDS[4] = { "01", "02", "03", "04" };
const char* EQUIP_IDS[4] = { "01", "02", "03", "04" };

/**
 * TODO: 2022-05-20 - Do we really need this?
 */
const char* CMD_NAME[CMD_COUNT] = { "add sensor", "remove sensor", "list sensors", "read", "kill" };
const char* CMD_PATTERN[CMD_COUNT] = {
    "^add sensor (0[1234] ){1,4}in 0[1234]$",
    "^remove sensor (0[1234] ){1,4}in 0[1234]$",
    "^list sensors in 0[1234]$",
    "^read (0[1234] ){1,4}in 0[1234]$",
    "^kill$"
};

typedef enum { SOCK_ACTION_RD = 10, SOCK_ACTION_WT } SocketActionEnum;

/**
 * ------------------------------------------------
 * == Local Headers ===============================
 * ------------------------------------------------
 */

bool netIsActionAvailable(int socket, const SocketActionEnum action, struct timeval *timeout);

/**
 * ------------------------------------------------
 * == DEBUG =======================================
 * ------------------------------------------------
 */

void comLogErrorAndDie(char *msg) {

	if (DEBUG_ENABLE) {
		if (errno) {
			strcat(msg, "\nDESC");
			perror(msg);
		} else
			printf("\n%s\n", msg);
	}

	exit(EXIT_FAILURE);
}

void comDebugStep(const char *text) {
	if (DEBUG_ENABLE)
		printf("%s\n", text);
}

/**
 * ------------------------------------------------
 * == MAIN ========================================
 * ------------------------------------------------
 */

int getEquipmentCodeById(const char* id) {
	for (int i = 0; i < EQUIP_COUNT; i++) {
		if (strcmp(EQUIP_IDS[i], id) == 0)
			return i;
	}
	return -1;
}

Equipment getEmptyEquipment(const EquipCodeEnum code) {
    Equipment equip;
    equip.code = code;
    equip.sensors[0] = false;
    equip.sensors[1] = false;
    equip.sensors[2] = false;
    equip.sensors[3] = false;
    return equip;
}

Command getGenericCommand(void) {
    Command command;
    command.isValid = false;
    command.equipCode = getEquipmentCodeById("");
    for (int i = 0; i < SENSOR_COUNT; i++)
        command.sensors[i] = false;
    return command;
}

Command getEmptyCommand(CmdCodeEnum code) {
    Command command = getGenericCommand();
    command.code = code;
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
        break;
	}

	if (!cmd.isValid || cmd.code == CMD_CODE_KILL)
        return cmd;

    // Determine equipment
    char inputCopy[100];
    strcpy(inputCopy, input);

    int inputArgsC;
    char** inputArgs = strSplit(inputCopy, " ", 8, 100, &inputArgsC);
    cmd.equipCode = getEquipmentCodeById(inputArgs[inputArgsC - 1]);

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

int netListen(const int port, const int timeoutSecs, const int maxConnections) {

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
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enableAddrReuse, sizeof(int)) != 0) {
        comLogErrorAndDie("Failure as creating listening socket [2]");
	}

	// Sets linstening timeout
	struct timeval timeout;
    timeout.tv_sec = timeoutSecs;
    timeout.tv_usec = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0)
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

int netConnect(const int port, const char *addrStr, const int timeoutSecs) {

	// Valida endereco
	if (!port)
		comLogErrorAndDie("invalid connection port");

    // Determina protocolo
	struct sockaddr_storage addrStorage;

    struct in_addr inaddr4; // 32-bit IP address
	const int isIpv4 = inet_pton(AF_INET, addrStr, &inaddr4);

	struct in6_addr inaddr6; // 128-bit IPv6 address
	const int isIpv6 = !isIpv4 ? inet_pton(AF_INET6, addrStr, &inaddr6) : 0;

	if (!isIpv4 && !isIpv6)
		comLogErrorAndDie("Invalid address family to create connection socket");

	// Cria socket
	int sock = socket(isIpv4 ? AF_INET : AF_INET6, SOCK_STREAM, 0);
	if (sock == -1)
		comLogErrorAndDie("Failure as creating connection socket [1]");

	struct timeval timeout;
    timeout.tv_sec = timeoutSecs;
    timeout.tv_usec = 0;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0)
		comLogErrorAndDie("Failure as creating connection socket [2]");

	// Trata ipv4
    if (isIpv4) {
		struct sockaddr_in *addr4 = (struct sockaddr_in *)&addrStorage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(port);
        addr4->sin_addr = inaddr4;

    // Trata ipv6
    } else {
		struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&addrStorage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons(port);
        // addr6->sin6_addr = inaddr6
        memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));
    }

	// Cria conexao
	if (connect(sock, (struct sockaddr *)(&addrStorage), sizeof(addrStorage)) != 0)
		comLogErrorAndDie("Failure as creating connection socket [3]");

	return sock;
}

bool netSend(const int socket, const char *msg) {

	char buffer[BUF_SIZE + strlen(NET_END_SEQ)];
	strcpy(buffer, msg);
	strcat(buffer, NET_END_SEQ);

	ssize_t acc = 0;
	const ssize_t bytesToSend = strlen(buffer);

	while (true) {

		// Check if is there any trouble before sending anything
		int sockError = 0;
		socklen_t sockErrLength = sizeof(sockError);
		int errResult = getsockopt(socket, SOL_SOCKET, SO_ERROR, &sockError, &sockErrLength);
		if (errResult != 0)
			comLogErrorAndDie("Failure as trying to get socket error information [send]");

		if (sockError != 0) {
			char aux[100];
			sprintf(aux, "Socket error detected (send): %d", sockError);
			comDebugStep(aux);
			return false;
		}

		// Try to send stuff
		ssize_t sentBytes = send(socket, buffer + acc, bytesToSend - acc, 0/* MSG_DONTWAIT */);
		if (sentBytes == -1)
			return false;
		
		if (sentBytes == 0)
			break;

		acc += sentBytes;
	}

	return acc >= bytesToSend;
}

int netAccept(const int servSocket) {

	struct sockaddr_storage clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);

	int clientSocket = accept(servSocket, (struct sockaddr *)(&clientAddr), &clientAddrLen);
	if (clientSocket == -1) {
		if (errno != EAGAIN && errno != EWOULDBLOCK)
			comLogErrorAndDie("Failure as trying to accept client connection");
		comDebugStep("Disconnecting server because of inactivity...\n\n");
	}

	return clientSocket;
}

ssize_t netRecv(const int cliSocket, char *buffer, const int timeoutSecs) {

	struct timeval timeout;
    timeout.tv_sec = timeoutSecs;
    timeout.tv_usec = 0;

	size_t acc = 0;
	while (true) {
		
		// Wait for some data
		const bool isThereAnyData = netIsActionAvailable(cliSocket, SOCK_ACTION_RD, &timeout);
        if (!isThereAnyData)
            break;

		// Receive data
		ssize_t receivedBytes = 0;
		receivedBytes = recv(cliSocket, buffer + acc, BUF_SIZE - acc, MSG_DONTWAIT);
		if (receivedBytes == 0) // Transmission is over
			break;

		if (receivedBytes == -1) // Something wrong is not right
			return (errno == EAGAIN || errno == EWOULDBLOCK) ? acc : -1;

		acc += receivedBytes;
		
		// Check if message is over
		bool isOver = strEndsWith(buffer, NET_END_SEQ);
		if (isOver) {
			int endSeqSize = strlen(NET_END_SEQ);
			acc -= endSeqSize; // Don't compute control characters
			buffer[strlen(buffer) - endSeqSize] = '\0';
			break;
		}
	}

	return acc;
}

bool netIsActionAvailable(int socket, const SocketActionEnum action, struct timeval *timeout) {

	if (!socket || !action)
		comLogErrorAndDie("Invalid arguments for action availability check");

	fd_set readFds;
	FD_ZERO(&readFds);

	fd_set writeFds;
	FD_ZERO(&writeFds);

	if (action == SOCK_ACTION_RD)
		FD_SET(socket, &readFds);
	else if (action == SOCK_ACTION_WT)
		FD_SET(socket, &writeFds);
	else
		comLogErrorAndDie("Invalid action type for availability check");

	int result = select(socket + 1, &readFds, &writeFds, NULL, timeout);
	if (result == 0) // Timeout
		return false;
	
	if (result == -1) {
		if (errno != EINTR) // Erro: Interrupted System Call | TODO: Why is this error ignored?
			comLogErrorAndDie("Failure as running availability check");
		return false;
	}

	if (result != 1
		|| (action == SOCK_ACTION_RD && FD_ISSET(socket, &readFds) == 0)
		|| (action == SOCK_ACTION_WT && FD_ISSET(socket, &writeFds) == 0)
	)
		comLogErrorAndDie("Availability check: Something wrong isn't right... D:");
	
	return true;
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

int netGetIpType(const char *ipTypeStr) {
    if (strcmp(ipTypeStr, "v4") == 0)
        return AF_INET;
    if (strcmp(ipTypeStr, "v6") == 0)
        return AF_INET6;
    return -1;
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
	size_t targetLength = strlen(target);
    size_t prefixLength = strlen(prefix);
	if (prefixLength > targetLength)
        return false;
   return strncmp(target, prefix, strlen(prefix)) == 0;
}

bool strEndsWith(const char *target, const char *suffix) {
	size_t targetLength = strlen(target);
    size_t suffixLength = strlen(suffix);
	if (suffixLength > targetLength)
        return false;
    return strncmp(target + targetLength - suffixLength, suffix, suffixLength) == 0;
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
