#include <stdlib.h>
// #include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

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
char* cliGetCleanInput(char* input);
int cliGetCommandFromInput(const char* input);

Message cliSendMessage(const int socket, const Message requestMsg);
int cliRequestToGetIn(const int socket);

/**
 * ------------------------------------------------
 * == MAIN ========================================
 * ------------------------------------------------
 */

int myId = 0;
int nEquipments = 0;
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
		close(sock);
		comLogErrorAndDie("Failure as trying to get in the network");
	}
	printf("\nNew ID: %d\n", myId); // NOTE: This really should be printed

	// Handle terminal commands...
	while (true) {

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
	cliDebugStep("Closing connection...");
	close(sock);
	cliDebugStep("\n --- THE END --- \n");
	exit(EXIT_SUCCESS);
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
        comDebugStep(aux);
        free(aux);
    }
}

/* -- Main ------------------- */

bool cliValidateInitialization(int argc, char **argv) {

	if (argc != 3) {
        cliDebugStep("Invalid argc!\n");
		return false;
    }

    // Validate port
	const char *portStr = argv[2];
	if (!strIsNumeric(portStr)) {
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

Message cliSendMessage(const int socket, const Message requestMsg) {
	
	cliDebugStep("Sending message...");
	char buffer[BUF_SIZE] = "";
    if (!buildMessageToSend(requestMsg, buffer, BUF_SIZE)) {
        comLogErrorAndDie("[cli] Failure as trying to build message to send");
    }
	if (!netSend(socket, buffer)) {
        comLogErrorAndDie("[cli] Failure as trying to send message");
	}

	cliDebugStep("Waiting for server answer...");
	char answer[BUF_SIZE] = "";
	ssize_t receivedBytes = netRecv(socket, answer, TIMEOUT_TRANSFER_SECS);
	if (receivedBytes == -1)
		comLogErrorAndDie("[cli] Failure as trying to receive server answer");

	if (DEBUG_ENABLE) {
		const char auxTemplate[] = "%lu bytes received: '%s'";
		char *aux = (char *)malloc(strlen(answer) + strlen(auxTemplate) + 1);
		sprintf(aux, auxTemplate, receivedBytes, answer);
		cliDebugStep(aux);
		free(aux);
	}

	Message responseMsg = getEmptyMessage();
	setMessageFromText(answer, &responseMsg);
	if (!responseMsg.isValid) {
		cliDebugStep("Invalid response received from server...");
		return responseMsg;
	}
	
	if (responseMsg.id == MSG_ERR) {
		printf("\n%s\n", ERR_NAMES[*(int *)responseMsg.payload]); // NOTE: This really should be printed
	}
	return responseMsg;
}

int cliRequestToGetIn(const int socket) {
	
	Message requestMsg = getEmptyMessage();
	requestMsg.id = MSG_REQ_ADD;
	
	Message responseMsg = cliSendMessage(socket, requestMsg);
	if (!responseMsg.isValid || responseMsg.id != MSG_RES_ADD) {
		cliDebugStep("Unexpected response for 'aks to get in' request...");
		return 0;
	}

	return *(int *)responseMsg.payload;
}
