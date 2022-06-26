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

void cliSendCommand(const int socket, const char* input, char *answer);
int cliRequestToGetIn(const int socket);

/**
 * ------------------------------------------------
 * == MAIN ========================================
 * ------------------------------------------------
 */

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
	if (!cliRequestToGetIn(sock)) {
		close(sock);
		comLogErrorAndDie("Failure as trying to get in the network");
	}

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
		printf("\n Command detected: '%d'\n", cmdCode);

		// Send command
		// char answer[BUF_SIZE];
		// cliSendCommand(sock, input, answer);

		// Finish execution
		// if (strcmp(answer, CMD_NAME[CMD_CODE_KILL]) == 0)
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
	
	if (strcmp(input, CMD_NAME[CMD_CODE_KILL]) == 0)
		return CMD_CODE_KILL;
	if (strcmp(input, CMD_NAME[CMD_CODE_LIST]) == 0)
		return CMD_CODE_LIST;

	// Test for request info command
	const char aux[] = " [0-9]{1,2}";
	const int patternSize = strlen(CMD_NAME[CMD_CODE_INFO]);
	char *infoPattern = (char *)malloc(patternSize + strlen(aux) + 1);
	const bool isInfoCommand = strRegexMatch(infoPattern, input, NULL);
	free(infoPattern);
	
	return isInfoCommand ? CMD_CODE_INFO : -1;
}

void cliSendCommand(const int socket, const char* input, char *answer) {

	// Send command
	cliDebugStep("Sending command...");
	const bool isSuccess = netSend(socket, input);
	if (!isSuccess)
		comLogErrorAndDie("Sending command failure");

	// Wait for response
	cliDebugStep("Waiting server answer...");
	ssize_t receivedBytes = netRecv(socket, answer, TIMEOUT_TRANSFER_SECS);
	if (receivedBytes == -1)
		comLogErrorAndDie("Failure as trying to receive server answer");

	if (DEBUG_ENABLE) {
		char dbgTxt[BUF_SIZE] = "";
		sprintf(dbgTxt, "Server response received with %lu bytes", receivedBytes);
		cliDebugStep(dbgTxt);
	}
}

int cliRequestToGetIn(const int socket) {
	
	// Build
	Message msg = getEmptyMessage();
	msg.id = MSG_REQ_ADD;
	msg.isValid = MSG_REQ_ADD;
	char input[BUF_SIZE] = "";
	if (!buildMessageToSend(msg, input, BUF_SIZE)) {
		cliDebugStep("Failure as trying to build");
		return 0;
	}

	// Send
	char answerBuffer[BUF_SIZE] = "";
	cliSendCommand(socket, input, answerBuffer);

	// Receive
	Message answer = getEmptyMessage();
	if (isValidReceivedMsg(answerBuffer))
		setMessageFromText(answerBuffer, &answer);
	return answer.id;
}
