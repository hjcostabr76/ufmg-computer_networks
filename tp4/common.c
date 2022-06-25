#include "common.h"

// #include <stdio.h>
#include <stdlib.h>
// #include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <string.h>
#include <regex.h>
#include <errno.h>
#include <ctype.h>
#include <netdb.h>
// #include <netinet/in.h>

/**
 * ------------------------------------------------
 * == CONSTS ======================================
 * ------------------------------------------------
 */

/*
-------------------------------------- >
>> How messages should look like
-------------------------------------- >
<msg>
	<id>[msg_id]<id/>
	<src>[eq_id]<src/>
	<target>[eq_id]<target/>
	<payload>[whatever]<payload/>
<msg/>
*/
const char* NET_TAG_MSG = "<msg>";
const char* NET_TAG_ID = "<id>";
const char* NET_TAG_SRC = "<src>";
const char* NET_TAG_TARGET = "<target>";
const char* NET_TAG_PAYLOAD = "<payload>";

const char* CMD_NAME[CMD_COUNT] = { "close connection", "list equipment", "request information from" };

typedef enum { SOCK_ACTION_RD = 10, SOCK_ACTION_WT } SocketActionEnum;

/**
 * ------------------------------------------------
 * == Local Headers ===============================
 * ------------------------------------------------
 */

/*-- Main --------------------*/
void initProtocolMessagesValidator();
void buildMessageToSend(const Message msg, char *buffer);

bool isValidMessageId(const int id);
bool isValidEquipId(const int id);
bool isValidMessageSource(const MessageIdEnum msgId, const int source);
bool isValidMessageTarget(const MessageIdEnum msgId, const int target);
bool isValidMessagePayload(const MessageIdEnum msgId, char *payloadText, void *payload);

int getIntTypeMessageField(const char *text, const char* delimiter);

void setMessagePayload(const char *text, const MessageIdEnum msgId, char **payloadText, void **payload);
void setIntTypePayload(const char *payloadText, void **payload);
void setFloatTypePayload(const char *payloadText, void **payload);
void setIntListTypePayload(char *payloadText, void **payload);

/*-- Network -----------------*/
bool netIsActionAvailable(int socket, const SocketActionEnum action, struct timeval *timeout);

/**
 * ------------------------------------------------
 * == DEBUG =======================================
 * ------------------------------------------------
 */

void comLogErrorAndDie(char *msg) {

	if (DEBUG_ENABLE) {
		if (errno) {
			char *descSuffix = "\n\tWhat: ";
			char errMsg[strlen(msg) + strlen(descSuffix)];
			strcpy(errMsg, msg);
			strcat(errMsg, descSuffix);
			perror(errMsg);
		} else
			fprintf(stderr, "\n%s\n", msg);
	}

	exit(EXIT_FAILURE);
}

void comDebugStep(const char *text) {
	if (DEBUG_ENABLE)
		printf("[log] %s\n", text);
}

/**
 * ------------------------------------------------
 * == MAIN ========================================
 * ------------------------------------------------
 */

bool isProtocolInitialized = false;
char protocolPattern[150] = "";

void initProtocolMessagesValidator() {

    if (isProtocolInitialized)
        return;
    isProtocolInitialized = true;

    char patternHeader[] = "<id>[0-9]<id><src>[0-9]{1,2}<src><target>[0-9]{1,2}<target>";
    char patternPayload[] = "(<payload>[a-zA-Z][0-9a-zA-Z\\t ]+[a-zA-Z]<payload>)?";
    
    strcat(protocolPattern, NET_TAG_MSG);
    strcat(protocolPattern, patternHeader);
    strcat(protocolPattern, patternPayload);
    strcat(protocolPattern, NET_TAG_MSG);
}

bool isValidReceivedMsg(const char *message) {
    initProtocolMessagesValidator();
    return strRegexMatch(protocolPattern, message, NULL);
}

void buildMessageToSend(const Message msg, char *buffer) {
	memset(buffer, 0, BUF_SIZE);

    char aux = '\0';
	strcat(buffer, NET_TAG_MSG); // Message

    strcat(buffer, NET_TAG_ID); // Id
    aux = msg.id + '0';
    strcat(buffer, &aux);
	strcat(buffer, NET_TAG_ID); // Id
	
	strcat(buffer, NET_TAG_SRC); // Src
    aux = msg.source + '0';
    strcat(buffer, &aux);
	strcat(buffer, NET_TAG_SRC); // Src
	
	strcat(buffer, NET_TAG_TARGET); // Target
    aux = msg.target + '0';
    strcat(buffer, &aux);
	strcat(buffer, NET_TAG_TARGET); // Target
	
	strcat(buffer, NET_TAG_PAYLOAD); // Payload
    strcat(buffer, (char *)msg.payload);
	strcat(buffer, NET_TAG_PAYLOAD); // Payload

	strcat(buffer, NET_TAG_MSG); // Message
}

void setMessageFromText(const char *text, Message *message) {

    message->id = 0;
    message->source = 0;
    message->target = 0;
    message->payload = NULL;
    message->payloadText = NULL;
    
	message->isValid = strSetDelimitedTextBounds(text, NET_TAG_MSG, NULL, NULL);
	if (!message->isValid)
		return;

	message->id = getIntTypeMessageField(text, NET_TAG_ID);
	message->source = getIntTypeMessageField(text, NET_TAG_SRC);
	message->target = getIntTypeMessageField(text, NET_TAG_TARGET);
	setMessagePayload(text, message->id, &message->payloadText, &message->payload);

	message->isValid = (
		isValidMessageId(message->id)
		&& isValidMessageSource(message->id, message->source)
		&& isValidMessageTarget(message->id, message->target)
		&& isValidMessagePayload(message->id, message->payloadText, message->payload)
	);
}

int getIntTypeMessageField(const char *text, const char* delimiter) {

	int begin = 0;
    int end = 0;
	bool hasSomething = strSetDelimitedTextBounds(text, delimiter, &begin, &end);
	if (!hasSomething)
		return 0;

	const int msgSize = (int)strlen(text);
    char *temp = (char *)malloc(msgSize);
    temp[0] = '\0';
	strSubstring(text, temp, begin, end);
	
	const int aux = atoi(temp);
	return aux > 0 ? aux : 0;
}

void setMessagePayload(const char *text, const MessageIdEnum msgId, char **payloadText, void **payload) {

	// Extract value from text
	int begin = 0;
    int end = 0;

	// Set payload text
	bool isPayloadEmpty = !strSetDelimitedTextBounds(text, NET_TAG_PAYLOAD, &begin, &end);
	if (!isPayloadEmpty) {

		const int msgSize = (int)strlen(text);
		char *temp = (char *)malloc(msgSize);
		temp[0] = '\0';
		strSubstring(text, temp, begin, end);

		isPayloadEmpty = !strlen(temp);
		if (!isPayloadEmpty) {
			*payloadText = (char *)malloc(strlen(temp));
			strcpy(*payloadText, temp);
		}
	}

	if (isPayloadEmpty)
		return;

	// Set value by type
	const bool isIntTypePayload = msgId == MSG_RES_ADD || msgId == MSG_ERR || msgId == MSG_OK;
	const bool isFloatTypePayload = msgId == MSG_RES_INF;
	const bool isIntListTypePayload = msgId == MSG_RES_LIST;

	if (isIntTypePayload)
		setIntTypePayload(*payloadText, payload);
	else if (isFloatTypePayload)
		setFloatTypePayload(*payloadText, payload);
	else if (isIntListTypePayload)
		setIntListTypePayload(*payloadText, payload);
}

bool isValidMessageId(const int id) {
	return (
		id == MSG_REQ_ADD
		|| id == MSG_REQ_RM
		|| id == MSG_RES_ADD
		|| id == MSG_RES_LIST
		|| id == MSG_REQ_INF
		|| id == MSG_RES_INF
		|| id == MSG_ERR
		|| id == MSG_OK
	);
}

bool isValidEquipId(const int id) {
	return id > 0 && id < MAX_CONNECTIONS;
}

bool isValidMessageSource(const MessageIdEnum msgId, const int source) {
	const bool shouldHaveSource = msgId == MSG_REQ_RM || msgId == MSG_REQ_INF || msgId == MSG_RES_INF;
	return (shouldHaveSource && isValidEquipId(source)) || (!shouldHaveSource && source == 0);
}

bool isValidMessageTarget(const MessageIdEnum msgId, const int target) {
	const bool shouldHaveTarget = msgId == MSG_REQ_INF || msgId == MSG_RES_INF || msgId == MSG_ERR || msgId == MSG_OK;
	return (shouldHaveTarget && isValidEquipId(target)) || (!shouldHaveTarget && target == 0);
}

bool isValidMessagePayload(const MessageIdEnum msgId, char *payloadText, void* payload) {

	// Validate coherence
	bool isPayloadEmpty = payload == NULL || *(float *)payload == 0;
	bool isPayloadTextEmpty = payloadText == NULL || strlen(payloadText) == 0;
	if (isPayloadEmpty != isPayloadTextEmpty)
		return false;

	/**
	 * Validate fulfilling
	 * NOTE: Payload is optional for MSG_RES_LIST
	 */
	const bool isRequiredPayload = msgId == MSG_RES_ADD || msgId == MSG_RES_INF || msgId == MSG_ERR || msgId == MSG_OK;
	const bool isForbiddenPayload = msgId == MSG_REQ_ADD || msgId == MSG_REQ_RM || msgId == MSG_REQ_INF;
	
	bool isValid = (isPayloadEmpty && !isRequiredPayload) || (!isPayloadEmpty && !isForbiddenPayload);
	if (isPayloadEmpty)
		return isValid;

	// Validate value by type
	const bool isIntTypePayload = msgId == MSG_RES_ADD || msgId == MSG_ERR || msgId == MSG_OK;
	const bool isFloatTypePayload = msgId == MSG_RES_INF;
	const bool isIntListTypePayload = msgId == MSG_RES_LIST;
	
	if (isIntTypePayload || isFloatTypePayload)
		return *(float *)payload > 0;

	if (!isIntListTypePayload)
		return false;

	int nEquipments = 0;
	payload = (int *)malloc(nEquipments * sizeof(int));
	for (int i = 0; i < nEquipments; i++) {
		isValid = isValid && *(int *)payload > 0;
		if (!isValid)
			break;
	}

	return isValid;
}

void setIntTypePayload(const char* payloadText, void **payload) {
	if (payloadText == NULL)
		payload = NULL;
	else {
		*payload = (int *)malloc(sizeof(int));
		*(int *)*payload = atoi(payloadText);
	}
}

void setFloatTypePayload(const char* payloadText, void **payload) {
	const bool isValid = payloadText != NULL && strRegexMatch("^[0-9]+\\.[0-9]{2}$", payloadText, NULL);
	if (!isValid)
		payload = NULL;
	else {
		*payload = (float *)malloc(sizeof(float));
		*(float *)*payload = atof(payloadText);
	}
}

void setIntListTypePayload(char* payloadText, void **payload) {

	bool isValid = payloadText != NULL && strRegexMatch("^([0-9]{1,2}(,[0-9]{1,2})*)?$", payloadText, NULL);
	if (!isValid) {
		*payload = NULL;
		return;
	}

	int nEquipments = 0;
	char **idStringList = strSplit(payloadText, ',', MAX_CONNECTIONS, 2, &nEquipments);
	*payload = (int *)malloc(nEquipments * sizeof(int));
	for (int i = 0; i < nEquipments; i++)
		((int *)*payload)[i] = atoi(idStringList[i]);
}

/**
 * ------------------------------------------------
 * == NETWORK =====================================
 * ------------------------------------------------
 */

int netListen(const char *portStr, const int timeoutSecs, const int maxConnections, const int *ipVersion) {

	// Validate params
	if (!strIsNumeric(portStr))
		comLogErrorAndDie("Invalid listening socket port");

	if (!maxConnections){
		comLogErrorAndDie("Listening socket must accept at least one connection");}

	// Load address info
	struct addrinfo hints;
	memset(&hints, 0, sizeof hints);

	hints.ai_flags = AI_PASSIVE; // Fill in my IP for me
	hints.ai_family = ipVersion == NULL ? AF_UNSPEC : (*ipVersion == 4 ? AF_INET : AF_INET6);
	hints.ai_socktype = SOCK_STREAM;

	struct addrinfo *addrInfo;
	getaddrinfo(NULL, portStr, &hints, &addrInfo);

	// Create socket
	const int sock = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
    if (sock == -1) {
        comLogErrorAndDie("Failure as creating listening socket [1]");}

    // Avoid that port used in an execution become deactivated for 02 min after conclusion
    const int enableAddrReuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enableAddrReuse, sizeof(int)) != 0) {
        comLogErrorAndDie("Failure as creating listening socket [2]");
	}

	// Set listening timeout
	struct timeval timeout;
    timeout.tv_sec = timeoutSecs;
    timeout.tv_usec = 0;
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0)
		comLogErrorAndDie("Failure as creating listening socket [3]");

	// Enable ipv4 connections in ipv6 socket (only if we accept both)
	if (ipVersion == NULL) {
		int no = 0;
		if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&no, sizeof(no)) != 0)
			comLogErrorAndDie("Failure as trying to enable ipv4 clients to ipv6 server");
	}

	// Bind socket
	if (bind(sock, addrInfo->ai_addr, addrInfo->ai_addrlen) != 0)
		comLogErrorAndDie("Failure as biding listening socket");
    
	// Start listening
	if (listen(sock, maxConnections) != 0)
		comLogErrorAndDie("Failure as starting to listen");
	

	freeaddrinfo(addrInfo); // Free the linked list
	return sock;
}

int netConnect(const char *portStr, const char *addrStr, const int timeoutSecs, const int *ipVersion) {

	// Validate
	if (!strIsNumeric(portStr))
		comLogErrorAndDie("invalid connection port");

	// Load address info
	struct addrinfo hints;
	memset(&hints, 0, sizeof hints);
	hints.ai_family = ipVersion == NULL ? AF_UNSPEC : (*ipVersion == 4 ? AF_INET : AF_INET6);
	hints.ai_socktype = SOCK_STREAM;

	struct addrinfo *addrInfo;
	getaddrinfo(addrStr, portStr, &hints, &addrInfo);

	// Create socket
	int sock = socket(addrInfo->ai_family, addrInfo->ai_socktype, addrInfo->ai_protocol);
	if (sock == -1)
		comLogErrorAndDie("Failure as creating connection socket [1]");

	struct timeval timeout;
    timeout.tv_sec = timeoutSecs;
    timeout.tv_usec = 0;
	if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0)
		comLogErrorAndDie("Failure as creating connection socket [2]");

	// Create connection
	if (connect(sock, addrInfo->ai_addr, addrInfo->ai_addrlen) != 0)
		comLogErrorAndDie("Failure as creating connection socket [3]");

	freeaddrinfo(addrInfo); // Free the linked list
	return sock;
}

bool netSend(const int socket, const char *msg) {

	char buffer[BUF_SIZE + strlen(NET_TAG_MSG)];
	strcpy(buffer, msg);
	strcat(buffer, NET_TAG_MSG);

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
		ssize_t sentBytes = send(socket, buffer + acc, bytesToSend - acc, MSG_DONTWAIT);
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

	ssize_t acc = 0;
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
		bool isOver = strEndsWith(buffer, NET_TAG_MSG);
		if (isOver) {
			int endSeqSize = strlen(NET_TAG_MSG);
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

bool netSetSocketAddrString(const int sock, char *addrStr) {

	struct sockaddr_storage storage;
	memset(&storage, 0, sizeof(storage));
	
	struct sockaddr *addr = (struct sockaddr *)&storage;
	memset(addr, 0, sizeof(*addr));

	socklen_t socketLength = sizeof(addr);
	getsockname(sock, (struct sockaddr*)addr, &socketLength);

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

bool strIsNumeric(const char *string) {
	for (size_t i = 0; i < strlen(string); i++) {
		if (!isdigit(string[i]))
			return false;
	}
	return true;
}

bool strIsAlphaNumericChar(const char c) {
	return (
		(c >= ASCII_NUM_FIRST && c <= ASCII_NUM_LAST)
		|| (c >= ASCII_CHAR_LC_FIRST && c <= ASCII_CHAR_LC_LAST)
		|| (c >= ASCII_CHAR_UC_FIRST && c <= ASCII_CHAR_UC_LAST)
		|| c == ' '
	);
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

void strSubstring(const char *src, char *dst, size_t start, size_t end) {
	
	if (start >= end) {
		char aux[100] = "";
		sprintf(aux, "strSubstring: 'start' / 'end' (%ld / %ld) nonsense...", start, end);
		comLogErrorAndDie(aux);
	}
	
	strncpy(dst, src + start, end - start);
	dst[end - start] = '\0';
}

bool strSetDelimitedTextBounds(const char* src, const char *delimiter, int *begin, int *end) {

    // Validate
	const int msgSize = strlen(src);
    const int delimiterSize = strlen(delimiter);

    if (!msgSize || !delimiterSize || delimiterSize > msgSize) {
        char msg[60] = "";
        sprintf(msg, "[Error!] 'msgSize' / 'delimiterSize' nonsense (%d / %d)...", msgSize, delimiterSize);
        comDebugStep(msg);
        return false;
    }

	const bool noBegin = begin == NULL;
	if (noBegin)
		begin = (int *)malloc(sizeof(int));
	const bool noEnd = end == NULL;
	if (noEnd)
		end = (int *)malloc(sizeof(int));

    // Seek for the message we're looking for
    *begin = -1;
    *end = -1;

    int i = 0;
    char *temp = (char *)malloc(msgSize);
    temp[0] = '\0';

    do {

        // Check for a match
        strSubstring(src, temp, i, msgSize);
        
        const bool isMatch = strStartsWith(temp, delimiter);
        if (!isMatch) {
            i += 1;
            continue;
        }
        
        // Update search state
        if (*begin >= 0) {
            *end = i;
            break;
        }
        
        i += delimiterSize;
        *begin = i;

    } while (i < msgSize);

    const bool isSuccess = (*begin >= 0 && *end > *begin);

	// Free some memory
	if (noBegin)
		free(begin);
	if (noEnd)
		free(end);

	// We're good
	return isSuccess;
}

char** strSplit(const char* source, const char delimiter, const int maxTokens, const int maxLength, int *tokensCount) {
    
    *tokensCount = 0;
    char** tokens = malloc(maxTokens * sizeof(char*));
	char *aux = (char *)malloc(strlen(source));
	strcpy(aux, source);

    char *token = strtok(aux, &delimiter);
    if (token == NULL)
        return tokens;

    while (token != NULL && *tokensCount < maxTokens) {

        tokens[*tokensCount] = malloc(maxLength * sizeof(char));
        memset(tokens[*tokensCount], '\n', maxLength * sizeof(char));
        strcpy(tokens[*tokensCount], token);
        
        *tokensCount = *tokensCount + 1;
        token = strtok(NULL, &delimiter);
    }

    return tokens;
}

bool strRegexMatch(const char* pattern, const char* str, char errorMsg[100]) {

    regex_t regex;
	if (errorMsg != NULL) {
    	memset(errorMsg, 0, 100);
	}
    
    // Compile
    int regStatus = regcomp(&regex, pattern, REG_EXTENDED);
    if (regStatus != 0) {
        sprintf(errorMsg, "Compiling error");
        return false;
    }

    // Execute
    regStatus = regexec(&regex, str, 0, NULL, 0);
    
    bool isSuccess = regStatus == 0;
    if (errorMsg != NULL && !isSuccess && regStatus != REG_NOMATCH) { // Error
        char aux[100];
        regerror(regStatus, &regex, aux, 100);
        sprintf(errorMsg, "Match error: %s\n", aux);
    }

    // Free memory allocated to the pattern buffer by regcomp()
    regfree(&regex);
    return isSuccess;
}

// char* strTrim(const char* input) {

// 	// Left trim
// 	char* trimmed = (char *)malloc(strlen(input));
//     while (isspace(*trimmed)) trimmed++;
    
// 	// Right trim
// 	char* back = trimmed + strlen(trimmed);
//     while (isspace(*--back));
//     *(back + 1) = '\0';

// 	return trimmed;
// }
