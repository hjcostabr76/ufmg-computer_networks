#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include "common.h"

/**
 * ------------------------------------------------
 * == HEADERS =====================================
 * ------------------------------------------------
 */

bool cliValidateInput(int argc, char **argv);
void cliExplainAndDie(char **argv);
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
	comDebugStep("Validating initialization command...\n");
    if (!cliValidateInput(argc, argv))
		cliExplainAndDie(argv);
	comDebugStep("Initialization is ok!\n");


	// Create socket
	comDebugStep("Creating socket...\n");

	struct timeval connTimeout;
	memset(&connTimeout, 0, sizeof(connTimeout));
	connTimeout.tv_sec = TIMEOUT_CONN_SECS;
	connTimeout.tv_usec = 0;

	const char *addrStr = argv[1];
	const char *portStr = argv[2];
	int sock = netConnect(atoi(portStr), addrStr, &connTimeout);

	if (DEBUG_ENABLE) {
		sprintf(dbgTxt, "\nConnected to %s:%s\n", addrStr, portStr);
		comDebugStep(dbgTxt);
	}

	do {

		// Wait for command
		char input[BUF_SIZE];
		if (!strReadFromStdIn(input, BUF_SIZE))
            comLogErrorAndDie("Failure as trying to read user input");

		memset(dbgTxt, 0, dbgTxtLength);
		sprintf(dbgTxt, "\nInput: \"%s\" %lu...\n", input, strlen(input));
		comDebugStep(dbgTxt);

		// TODO:...
		// Check if end is required
		// char regexMsg[100];
        // const bool shouldFinish = strRegexMatch(CMD_PATTERN[CMD_CODE_KILL], input, regexMsg);
        // if (shouldFinish)
        //     break;

		// Send command
		char answer[BUF_SIZE];
		sendCommand(sock, input, answer);
		printf("\n answer: '%s'", answer); // TODO: Remove...
		break;

	} while (true);

	// Finish
	if (DEBUG_ENABLE)
		comDebugStep("\nThe end...\n");

	close(sock);
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
	if (!strValidateNumeric(portStr, strlen(portStr))) {
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

void cliSendCommand(const int socket, const char* input, char *answer) {

	// Send command
	struct timeval transferTimeout;
	memset(&transferTimeout, 0, sizeof(transferTimeout));
	transferTimeout.tv_sec = TIMEOUT_TRANSFER_SECS;
	transferTimeout.tv_usec = 0;
	
	comDebugStep("Sending command...\n");
	const bool isSuccess = netSend(socket, input, strlen(input), &transferTimeout);
	if (!isSuccess)
		comLogErrorAndDie("Sending command failure");

	// Wait for response
	comDebugStep("Waiting server answer...\n");
	memset(answer, 0, BUF_SIZE);
	size_t receivedBytes = netRecv(socket, answer, &transferTimeout);
	if (receivedBytes == -1)
		comLogErrorAndDie("Failure as trying to receive server answer");

	if (DEBUG_ENABLE) {
		char dbgTxt[BUF_SIZE];
		memset(dbgTxt, 0, BUF_SIZE);
		sprintf(dbgTxt, "\tReceived %lu bytes!\n", receivedBytes);
		commonDebugStep(dbgTxt);
	}
}