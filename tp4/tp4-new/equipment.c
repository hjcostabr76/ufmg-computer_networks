#include <stdlib.h>
#include <pthread.h>
// #include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

typedef struct { int socket; } ThreadData;

/**
 * ------------------------------------------------
 * == HEADERS =====================================
 * ------------------------------------------------
 */

/* -- Debug ------------------ */
void cliDebugStep(const char* log);

/* -- Main ------------------- */
bool cliValidateInitialization(int argc, char **argv);
void cliExplainAndDie(char **argv);
void cliThreadDebugStep(const char* log);
void cliFinishGracefully(const int sock);

char* cliGetCleanInput(char* input);
int cliGetCommandFromInput(const char* input);

void *cliThreadServerListener(void *threadData);
void cliSendMessage(const int socket, const Message message);
Message cliReceiveMsg(const int socket);

int cliRequestToGetIn(const int socket);
void cliAddEquipment(const int newEquipId);
void cliParseEquipmentsList(const int equipList[], const int nEquipments);

/**
 * ------------------------------------------------
 * == MAIN ========================================
 * ------------------------------------------------
 */

int myId = 0;
bool equipments[MAX_CONNECTIONS] = { false };

int main(int argc, char **argv) {

    // Validate initialization command
	cliDebugStep("Validating initialization command...");
    if (!cliValidateInitialization(argc, argv))
		cliExplainAndDie(argv);
	cliDebugStep("Initialization is ok!");

	// Create socket
	cliDebugStep("Creating socket...");

	char *addrStr = argv[1];
	char *portStr = argv[2];
	const int ipVersion = 4;
	const int sock = netConnect(portStr, addrStr, TIMEOUT_CONN_SECS, &ipVersion);

	if (DEBUG_ENABLE) {
		const char auxTemplate[] = "Connected to %s:%s";
		char *aux = (char *)malloc(strlen(addrStr) + strlen(portStr) + strlen(auxTemplate) + 1);
		sprintf(aux, auxTemplate, addrStr, portStr);
		free(aux);
		cliDebugStep(aux);
	}

	// Get in the network
	myId = cliRequestToGetIn(sock);
	if (!myId) {
		cliDebugStep("Failure as trying to get in the network");
		cliFinishGracefully(sock);
	}
	printf("\nNew ID: %d\n", myId); // NOTE: This really should be printed

	// Start thread to listen to server commands
	pthread_t threadID;
	ThreadData threadData = { sock };
	pthread_create(&threadID, NULL, (void *)cliThreadServerListener, &threadData);

	// Handle terminal commands...
	while (myId) {

		// Wait for command
		char inputRaw[BUF_SIZE] = "";
		if (!strReadFromStdIn(inputRaw, BUF_SIZE))
            comLogErrorAndDie("Failure as trying to read user input");

		char* input = cliGetCleanInput(inputRaw);
		if (strcasecmp(input, inputRaw) != 0 && DEBUG_ENABLE) {
			const char auxTemplate[] = "Input had to be cleared:\n\t[before] '%s'\n\t[after] '%s'";
			char *aux = (char *)malloc(strlen(input) + strlen(auxTemplate) + 1);
			sprintf(aux, auxTemplate, inputRaw, input);
			free(aux);
			cliDebugStep(aux);
		}

		CommandCodeEnum cmdCode = cliGetCommandFromInput(input);
		if (DEBUG_ENABLE) {
			const char auxTemplate[] = "Command detected: '%d'";
			char *aux = (char *)malloc(strlen(input) + strlen(auxTemplate) + 1);
			sprintf(aux, auxTemplate, cmdCode);
			cliDebugStep(aux);
			free(aux);
		}

		// Send command
		// char answer[BUF_SIZE];
		// cliSendCommand(sock, input, answer);

		// Finish execution
		// if (strcmp(answer, CMD_NAMES[CMD_CODE_KILL]) == 0)
		// 	break;

		// Exhibit response
		// printf("%s\n", answer);
	};

	// Finish
	cliFinishGracefully(sock);
}

void *cliThreadServerListener(void *threadData) {
	
	cliThreadDebugStep("Starting thread to listen to server messages...");
	ThreadData *client = (ThreadData *)threadData;

	while (myId) {
		
		// Receive message
		cliThreadDebugStep("Waiting for messages...");
		Message msg = cliReceiveMsg(client->socket);
		if (!msg.isValid) {
			myId = 0;
			break;
		}

		// Handle request
		const bool shouldAddNewEquip = msg.id == MSG_RES_ADD && *(int *)msg.payload != myId;
		if (shouldAddNewEquip) {
			cliThreadDebugStep("Someone new is getting in...");
			cliAddEquipment(*(int *)msg.payload);
			continue;
		}

		// Handle request
		const bool shouldListEquipments = msg.id == MSG_RES_LIST;
		if (shouldListEquipments) {
			cliThreadDebugStep("Receiving list of current equipments...");
			cliParseEquipmentsList((int *)msg.payload, msg.payloadSize);
			continue;
		}
	}

    // End thread successfully
	cliThreadDebugStep("Done!");
    close(client->socket);
    pthread_exit(EXIT_SUCCESS);
}

/**
 * ------------------------------------------------
 * == HELPER ======================================
 * ------------------------------------------------
 */

/* -- Debug ------------------ */

void cliDebugStep(const char* log) {
    if (DEBUG_ENABLE) {
        const char prefix[] = "[cli]";
        char *aux = (char *)malloc(strlen(log) + strlen(prefix) + 2);
        sprintf(aux, "%s %s", prefix, log);
        comDbgStep(aux);
        free(aux);
    }
}

void cliThreadDebugStep(const char* log) {
    if (DEBUG_ENABLE) {
        const char prefix[] = "[cli: thread]";
        char *aux = (char *)malloc(strlen(log) + strlen(prefix) + 2);
        sprintf(aux, "%s %s", prefix, log);
        comDbgStep(aux);
        free(aux);
    }
}

void cliDebugEquipmentsCount() {
	
	if (!DEBUG_ENABLE)
		return;
	
	int nEquipments = 0;
	int *eqIds = (int *)malloc(nEquipments * sizeof(int));
	
	for (int i = 0; i < MAX_CONNECTIONS; i++) {
		if (equipments[i]) {
			eqIds[i] = i + 1;
			nEquipments++;
		}
	}
	
	const char *equipListString = strGetStringFromIntList(eqIds, nEquipments);
	const char auxTemplate[] = "'%d' equipment(s) currently: '%s'";
    char *aux = (char *)malloc(strlen(auxTemplate) + strlen(equipListString) + 1);
    sprintf(aux, auxTemplate, nEquipments, equipListString);
	
	cliDebugStep(aux);
	free(aux);
}

/* -- Main ------------------- */

bool cliValidateInitialization(int argc, char **argv) {

	if (argc != 3) {
        cliDebugStep("Invalid argc!\n");
		return false;
    }

    // Validate port
	const char *portStr = argv[2];
	const bool isIntOnly = true;
	if (!strIsNumeric(portStr, &isIntOnly)) {
		cliDebugStep("Invalid Port!\n");
		return false;
	}

	return true;
}

void cliExplainAndDie(char **argv) {
    printf("\nInvalid Input\n");
    printf("Usage: %s <server IP> <server port>\n", argv[0]);
	printf("Example: %s 127.0.0.1 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

void cliFinishGracefully(const int sock) {
	cliDebugStep("Closing connection...");
	close(sock);
	cliDebugStep("\n --- THE END --- \n");
	exit(EXIT_SUCCESS);
}

char* cliGetCleanInput(char* input) {
	
	int i = 0;
	char* cleanInput = (char*)malloc(strlen(input) + 1);

	for (size_t j = 0; j < strlen(input); j++) {
		char c = input[j];
		if (strIsAlphaNumericChar(c))
			cleanInput[i++] = c;
	}
	
	cleanInput[i] = '\0';
	return cleanInput;
}

int cliGetCommandFromInput(const char* input) {
	
	if (strcmp(input, CMD_NAMES[CMD_CODE_KILL]) == 0)
		return CMD_CODE_KILL;
	if (strcmp(input, CMD_NAMES[CMD_CODE_LIST]) == 0)
		return CMD_CODE_LIST;

	// Test for request info command
	const char aux[] = " [0-9]{1,2}";
	const int patternSize = strlen(CMD_NAMES[CMD_CODE_INFO]);
	char *infoPattern = (char *)malloc(patternSize + strlen(aux) + 1);
	const bool isInfoCommand = strRegexMatch(infoPattern, input, NULL);
	free(infoPattern);
	
	return isInfoCommand ? CMD_CODE_INFO : -1;
}

Message cliReceiveMsg(const int socket) {
    cliDebugStep("Waiting for messages...");

	char answer[BUF_SIZE] = "";
	ssize_t receivedBytes = netRecv(socket, answer, TIMEOUT_TRANSFER_SECS);
	if (receivedBytes == -1)
		comLogErrorAndDie("[cli] Failure as trying to receive message");

	if (DEBUG_ENABLE) {
		const char auxTemplate[] = "%lu bytes received: '%s'";
		char *aux = (char *)malloc(strlen(answer) + strlen(auxTemplate) + 1);
		sprintf(aux, auxTemplate, receivedBytes, answer);
		cliDebugStep(aux);
		free(aux);
	}

	Message responseMsg = getEmptyMessage();
	if (!receivedBytes)
		return responseMsg;

	setMessageFromText(answer, &responseMsg);
	if (!responseMsg.isValid) {
		cliDebugStep("Invalid message received...");
		return responseMsg;
	}
	
	if (responseMsg.id == MSG_ERR) {
		printf("\n%s\n", ERR_NAMES[*(int *)responseMsg.payload - 1]); // NOTE: This really should be printed
	}
	return responseMsg;
}

void cliSendMessage(const int socket, const Message message) {
	cliDebugStep("Sending message...");
	char buffer[BUF_SIZE] = "";
    if (!buildMessageToSend(message, buffer, BUF_SIZE))
		comLogErrorAndDie("[cli] Failure as trying to build message to send");
	if (!netSend(socket, buffer))
		comLogErrorAndDie("[cli] Failure as trying to send message");
}

int cliRequestToGetIn(const int socket) {

	Message requestMsg = getEmptyMessage();
	requestMsg.id = MSG_REQ_ADD;
	cliSendMessage(socket, requestMsg);

	Message addResponseMsg = cliReceiveMsg(socket);
	const bool hasError = addResponseMsg.id == MSG_ERR;
	if (hasError)
		return 0;

	const bool isInvalidAnswer = !addResponseMsg.isValid || addResponseMsg.id != MSG_RES_ADD;
	if (isInvalidAnswer) {
		cliDebugStep("Unexpected response for 'aks to get in' request...");
		return 0;
	}

	// We're good ;)
	return *(int *)addResponseMsg.payload;
}

void cliAddEquipment(const int newEquipId) {
	equipments[newEquipId - 1] = true;
    printf("\nEquipment %d added\n", newEquipId); // NOTE: This really should be printed
    cliDebugEquipmentsCount();
}

void cliParseEquipmentsList(const int equipList[], const int length) {
	for (int i = 0; i < length; i++) {
		const int id = equipList[i];
		equipments[id - 1] = true;
	}
	cliDebugEquipmentsCount();
}