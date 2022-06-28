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


/* -- Main ----------- */

void cliAddEquipment(const int newEquipId);
void *cliThreadServerListener(void *threadData);
int cliRequestToGetIn(const int socket);
void cliRequestToGetOut(const int socket);
void cliSetEquipmentList(const int equipList[], const int nEquipments);
void cliListEquipments();
void cliRequestInfo(const int sock, const int target);

/* -- Helper --------- */

void cliSendMessage(const int socket, const Message message);
Message cliReceiveMsg(const int socket);

/* -- I/O ------------ */

bool cliValidateInitialization(int argc, char **argv);
void cliExplainAndDie(char **argv);
void cliFinishGracefully(const int sock);
char* cliGetCleanInput(char* input);
char* cliGetEquipListString(void);
int cliGetCommandFromInput(const char* input);
void cliDbgEquipments(void);


/**
 * ------------------------------------------------
 * == MAIN ========================================
 * ------------------------------------------------
 */

int myId = 0;
int nEquipments = 0;
bool isThreadOpen = false;
bool equipments[MAX_CONNECTIONS] = { false };

int main(int argc, char **argv) {

    // Validate initialization command
	comDbgStep("Validating initialization...");
    if (!cliValidateInitialization(argc, argv))
		cliExplainAndDie(argv);

	// Create socket
	comDbgStep("Creating socket...");

	char *addrStr = argv[1];
	char *portStr = argv[2];
	const int ipVersion = 4;
	const int sock = netConnect(portStr, addrStr, TIMEOUT_CONN_SECS, &ipVersion);

	if (DEBUG_ENABLE) {
		const char auxTemplate[] = "Connected to %s:%s";
		char *aux = (char *)malloc(strlen(addrStr) + strlen(portStr) + strlen(auxTemplate) + 1);
		sprintf(aux, auxTemplate, addrStr, portStr);
		comDbgStep(aux);
		free(aux);
	}

	// Get in the network
	myId = cliRequestToGetIn(sock);
	if (!myId) {
		comDbgStep("Failure as trying to get in the network");
		cliFinishGracefully(sock);
	}
	printf("\nNew ID: %d\n", myId); // NOTE: This really should be printed

	// Start thread to listen to server commands
	pthread_t threadID;
	ThreadData threadData = { sock };
	pthread_create(&threadID, NULL, (void *)cliThreadServerListener, &threadData);

	// Handle terminal commands...
	while (myId || isThreadOpen) {

		// Wait for command
		comDbgStep("Waiting for commands...");
		char inputRaw[BUF_SIZE] = "";
		if (!strReadFromStdIn(inputRaw, BUF_SIZE))
            comLogErrorAndDie("Failure as trying to read user input");

		char* input = cliGetCleanInput(inputRaw);
		if (DEBUG_ENABLE && strcmp(input, inputRaw) != 0) {
			const char auxTemplate[] = "Input had to be cleared:\n\t[before] '%s'\n\t[after] '%s'";
			char *aux = (char *)malloc(strlen(input) + strlen(auxTemplate) + 1);
			sprintf(aux, auxTemplate, inputRaw, input);
			comDbgStep(aux);
			free(aux);
		}

		// Parse command
		CommandCodeEnum cmdCode = cliGetCommandFromInput(input);
		if (DEBUG_ENABLE) {
			const char auxTemplate[] = "Command detected: '%d'";
			char *aux = (char *)malloc(strlen(input) + strlen(auxTemplate) + 1);
			sprintf(aux, auxTemplate, cmdCode);
			comDbgStep(aux);
			free(aux);
		}

		// Execute command
		switch (cmdCode) {
			case CMD_CODE_KILL:
				cliRequestToGetOut(sock);
				break;
			case CMD_CODE_LIST:
				cliListEquipments();
				break;
			case CMD_CODE_INFO: {
				const int nPieces = 4;
				const size_t maxLength = strlen(CMD_NAMES[CMD_CODE_INFO]);
				char** pieces = strSplit(input, ' ', nPieces, maxLength, NULL);
				const int target = atoi(pieces[nPieces - 1]);
				cliRequestInfo(sock, target);
				break;
			}
			// no default
		}
	};

	// Happy ending
	cliFinishGracefully(sock);
}

void *cliThreadServerListener(void *threadData) {
	
	isThreadOpen = true;
	comDbgStep("Starting thread to listen to server messages...");
	ThreadData *client = (ThreadData *)threadData;

	while (myId) {
		
		// Receive message
		comDbgStep("Waiting for messages...");
		Message msg = cliReceiveMsg(client->socket);
		if (!msg.isValid)
			continue;

		switch (msg.id) {
			case MSG_RES_ADD: {
				const int newGuy = atoi(msg.payload);
				if (newGuy != myId) {
					comDbgStep("Someone new is getting in...");
					cliAddEquipment(newGuy);
				}
				continue;
			}
			case MSG_RES_LIST: {
				comDbgStep("Receiving list of current equipments...");
				int nIds = 0;
				int *ids = strGetIntListFromString(msg.payload, &nIds);
				cliSetEquipmentList(ids, nIds);
				continue;
			}
			default:
				continue;
		}
	}

    // End thread successfully
	comDbgStep("Closing thread...");
    // close(client->socket);
	isThreadOpen = false;
    pthread_exit(EXIT_SUCCESS);
}

int cliRequestToGetIn(const int socket) {

	Message requestMsg = getEmptyMessage();
	requestMsg.id = MSG_REQ_ADD;
	cliSendMessage(socket, requestMsg);

	Message responseMsg = cliReceiveMsg(socket);
	const bool isSuccess = responseMsg.isValid && responseMsg.id == MSG_RES_ADD;
	if (isSuccess)
		return atoi(responseMsg.payload);
	
	if (responseMsg.id != MSG_ERR) // Errors should be handled elsewhere
		comDbgStep("Unexpected response for 'aks to get in' request...");
	
	return 0;
}

void cliRequestToGetOut(const int socket) {

	Message requestMsg = getEmptyMessage();
	requestMsg.id = MSG_REQ_RM;
	requestMsg.source = myId;
	cliSendMessage(socket, requestMsg);

	Message responseMsg = cliReceiveMsg(socket);
	const bool isSuccess = responseMsg.id == MSG_OK && responseMsg.isValid && atoi(responseMsg.payload) == OK_RM;
	if (isSuccess) {
		myId = 0;
		return;
	}

	if (responseMsg.id != MSG_ERR) // Errors should be handled elsewhere
		comDbgStep("Unexpected response for 'aks to get out' request...");
}

void cliRequestInfo(const int socket, const int target) {
	
	Message requestMsg = getEmptyMessage();
	requestMsg.id = MSG_REQ_INF;
	requestMsg.source = myId;
	requestMsg.target = target;
	cliSendMessage(socket, requestMsg);

	Message responseMsg = cliReceiveMsg(socket);
	const bool isSuccess = responseMsg.id == MSG_RES_INF && responseMsg.isValid;
	if (isSuccess) {
		printf("\nValue from equipment %d: %s\n", responseMsg.source, responseMsg.payload); // NOTE: This really have to be printed
		return;
	}
	
	if (responseMsg.id != MSG_ERR) // Errors should be handled elsewhere
		comDbgStep("Unexpected response for 'request info' request...");
}

void cliListEquipments() {
	// NOTE: These really should be printed
	char *list = cliGetEquipListString();
	if (strlen(list))
		printf("\n%s\n", list);
	else
		printf("\nNo equipments\n");
}

void cliAddEquipment(const int id) {
	
	// Add equipment
	const int i = getEquipIndexFromId(id); 
	const bool isRepeated = equipments[i];
	
	if (!isRepeated) {
		equipments[i] = true;
		nEquipments++;
		printf("\nEquipment %d added\n", id); // NOTE: This really should be printed
		cliDbgEquipments();
		return;
	}

	// Log error
	const char auxTemplate[] = "Something wrong is not right (trying to add equipment '%d')...";
	char *aux = (char *)malloc(strlen(auxTemplate) + 2);
	sprintf(aux, auxTemplate, id);
	comDbgStep(aux);
	free(aux);
}

void cliSetEquipmentList(const int ids[], const int nEquips) {

	nEquipments = 0;
	for (int i = 0; i < MAX_CONNECTIONS; i++) {
		
		const int id = getEquipIdFromIndex(i);
		bool isFound = false;
		
		for (int j = 0; j < nEquips; j++) {
			isFound = id == getEquipIndexFromId(ids[j]);
			if (isFound)
				break;
		}

		equipments[i] = isFound;
		if (isFound)
			nEquipments++;
	}
	
	cliDbgEquipments();
}

/**
 * ------------------------------------------------
 * == HELPER ======================================
 * ------------------------------------------------
 */

Message cliReceiveMsg(const int socket) {

	char answer[BUF_SIZE] = "";
	ssize_t receivedBytes = netRecv(socket, answer, TIMEOUT_TRANSFER_SECS);
	if (receivedBytes == -1)
		comLogErrorAndDie("Failure as trying to receive message");

	if (DEBUG_ENABLE) {
		const char auxTemplate[] = "%lu bytes received: '%s'";
		char *aux = (char *)malloc(strlen(answer) + strlen(auxTemplate) + 1);
		sprintf(aux, auxTemplate, receivedBytes, answer);
		comDbgStep(aux);
		free(aux);
	}

	Message msg = getEmptyMessage();
	if (!receivedBytes)
		return msg;

	setMessageFromText(answer, &msg);
	if (!msg.isValid) {
		comDbgStep("Invalid message received...");
		comDebugProtocolMessage(msg);
		return msg;
	}
	
	if (msg.id == MSG_ERR) {
		printf("\n%s\n", ERR_NAMES[*(int *)msg.payload - 1]); // NOTE: This really should be printed
	}
	return msg;
}

void cliSendMessage(const int socket, const Message message) {
	comDbgStep("Sending message...");
	char buffer[BUF_SIZE] = "";
    if (!buildMessageToSend(message, buffer, BUF_SIZE))
		comLogErrorAndDie("Failure as trying to build message to send");
	if (!netSend(socket, buffer))
		comLogErrorAndDie("Failure as trying to send message");
}


/**
 * ------------------------------------------------
 * == I/O =========================================
 * ------------------------------------------------
 */

bool cliValidateInitialization(int argc, char **argv) {

	if (argc != 3) {
        comDbgStep("Invalid argc!\n");
		return false;
    }

    // Validate port
	const char *portStr = argv[2];
	const bool isIntOnly = true;
	if (!strIsNumeric(portStr, &isIntOnly)) {
		comDbgStep("Invalid Port!\n");
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
	comDbgStep("Closing connection...");
	close(sock);
	comDbgStep("\n --- THE END --- \n");
	exit(EXIT_SUCCESS);
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

char* cliGetEquipListString() {
	
	// Count equipments
	int nEquipments = 0;
	for (int i = 0; i < MAX_CONNECTIONS; i++) {
		if (equipments[i])
			nEquipments++;
	}

	// Make list
	int *eqIds = (int *)malloc(nEquipments * sizeof(int));
	for (int i = 0; i < MAX_CONNECTIONS; i++) {
		if (equipments[i])
			eqIds[i] = getEquipIdFromIndex(i);
	}

	char *listString = strGetStringFromIntList(eqIds, nEquipments);
	return listString;
}

void cliDbgEquipments() {
	if (DEBUG_ENABLE) {
		const char *listString = cliGetEquipListString();
		const char auxTemplate[] = "'%d' equipment(s) currently: '%s'";
		char *aux = (char *)malloc(strlen(auxTemplate) + strlen(listString) + 1);
		sprintf(aux, auxTemplate, nEquipments, listString);
		comDbgStep(aux);
		free(aux);
	}
}
