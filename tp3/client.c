#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>

#include "common.h"

/**
 * ------------------------------------------------
 * == HEADERS =====================================
 * ------------------------------------------------
 */

bool cliValidateInput(int argc, char **argv);
void cliExplainAndDie(char **argv);
char* cliGetCleanInput(char* input);
void cliSendCommand(const int socket, const char* input, char *answer);

/**
 * ------------------------------------------------
 * == MAIN ========================================
 * ------------------------------------------------
 */

int main(int argc, char **argv) {

	const int dbgTxtLength = 500;
    char dbgTxt[dbgTxtLength];

	// Validate initialization command
	comDebugStep("Validating initialization command...");
    if (!cliValidateInput(argc, argv))
		cliExplainAndDie(argv);
	comDebugStep("Initialization is ok!");


	// Create socket
	comDebugStep("Creating socket...");

	const char *addrStr = argv[1];
	const char *portStr = argv[2];
	int sock = netConnect(atoi(portStr), addrStr, TIMEOUT_CONN_SECS);

	if (DEBUG_ENABLE) {
		memset(dbgTxt, 0, dbgTxtLength);
		sprintf(dbgTxt, "Connected to %s:%s", addrStr, portStr);
		comDebugStep(dbgTxt);
	}

	do {

		// Wait for command
		char aux[BUF_SIZE];
		if (!strReadFromStdIn(aux, BUF_SIZE))
            comLogErrorAndDie("Failure as trying to read user input");

		char* input = cliGetCleanInput(aux);
		if (strcasecmp(input, aux) != 0) {
			memset(dbgTxt, 0, dbgTxtLength);
			sprintf(dbgTxt, "Input had to be cleared:\n\t[before] '%s'\n\t[after] '%s'", aux, input);
			comDebugStep(dbgTxt);
		}

		// Send command
		char answer[BUF_SIZE];
		cliSendCommand(sock, input, answer);

		// Finish execution
		if (strcmp(answer, CMD_NAME[CMD_CODE_KILL]) == 0)
			break;

		// Exhibit response
		printf("%s\n", answer);

	} while (true);

	// Finish
	comDebugStep("Closing connection...");
	close(sock);
	comDebugStep("\n --- THE END --- \n");
	exit(EXIT_SUCCESS);
}

/**
 * ------------------------------------------------
 * == AUXILIARY ===================================
 * ------------------------------------------------
 */

bool cliValidateInput(int argc, char **argv) {

	if (argc != 3) {
        comDebugStep("Invalid argc!\n");
		return false;
    }

    // Validate port
	const char *portStr = argv[2];
	if (!strIsNumeric(portStr)) {
		comDebugStep("Invalid Port!\n");
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
	char* cleanInput = (char*)malloc(strlen(input));

	for (int j = 0; j < strlen(input); j++) {
		char c = input[j];
		if (strIsAlphaNumericChar(c))
			cleanInput[i++] = c;
	}
	
	cleanInput[i] = '\0';
	return cleanInput;
}

void cliSendCommand(const int socket, const char* input, char *answer) {

	// Send command
	comDebugStep("Sending command...");
	const bool isSuccess = netSend(socket, input);
	if (!isSuccess)
		comLogErrorAndDie("Sending command failure");

	// Wait for response
	comDebugStep("Waiting server answer..");
	memset(answer, 0, BUF_SIZE);
	size_t receivedBytes = netRecv(socket, answer, TIMEOUT_TRANSFER_SECS);
	if (receivedBytes == -1)
		comLogErrorAndDie("Failure as trying to receive server answer");

	if (DEBUG_ENABLE) {
		char dbgTxt[BUF_SIZE];
		memset(dbgTxt, 0, BUF_SIZE);
		sprintf(dbgTxt, "Server response received with %lu bytes", receivedBytes);
		comDebugStep(dbgTxt);
	}
}