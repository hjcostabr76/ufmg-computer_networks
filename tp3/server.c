#include "common.h"

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>

/**
 * TODO: 2022-05-13 - Solve all todo's...
 */

/**
 * ------------------------------------------------
 * == ABSTRACTS ===================================
 * ------------------------------------------------
 */


/**
 * ------------------------------------------------
 * == HEADERS =====================================
 * ------------------------------------------------
 */

bool servValidateInput(int argc, char **argv);
void servExplainAndDie(char **argv);

/**
 * ------------------------------------------------
 * == MAIN ========================================
 * ------------------------------------------------
 */

int main(int argc, char **argv) {

    // Validate initialization command
	comDebugStep("Validating input...\n");
    if (!servValidateInput(argc, argv))
        servExplainAndDie(argv);

    // Create socket
    const int dbgTxtLength = 500;
    char dbgTxt[dbgTxtLength];
    const int port = atoi(argv[2]);

    struct timeval timeoutConn;
    memset(&timeoutConn, 0, sizeof(timeoutConn));
    timeoutConn.tv_sec = TIMEOUT_CONN_SECS;
    timeoutConn.tv_usec = 0;

    // Listen for messages
    comDebugStep("Creating server socket...\n");
    int socket = netListen(port, &timeoutConn, MAX_CONNECTIONS);

    if (DEBUG_ENABLE) {

        char boundAddr[200];
        memset(dbgTxt, 0, dbgTxtLength);
        if (!posixSetSocketAddressString(socket, boundAddr)) {
            sprintf(dbgTxt, "\nFailure as trying to exhibit bound address...\n");
            comLogErrorAndDie(dbgTxt);
        }

        sprintf(dbgTxt, "\nAll set! Server is bound to %s:%d\nWaiting for connections...\n", boundAddr, port);
        comDebugStep(dbgTxt);
    }

    // Validate messages
    // Determine command
        // Add sensor
        // Read sensor
        // Remove sensor
        // List equipment sensors
    // Run command
    // Send response
    // Close connection
    // End execution

    // Finish...
	comDebugStep("\nClosing connection...\n");
    close(socket);
    exit(EXIT_SUCCESS);
}

/**
 * ------------------------------------------------
 * == AUXILIARY ===================================
 * ------------------------------------------------
 */

bool servValidateInput(int argc, char **argv) {

	if (argc != 3) {
        comDebugStep("Invalid argc!\n");
		return false;
    }

    // Validate ip type
	if (netGetIpType(argv[1]) == -1) {
		comDebugStep("Invalid IP type!\n");
		return false;
	}

    // Validate port
	const char *portStr = argv[1];
	if (!comValidateNumericString(portStr, strlen(portStr))) {
		comDebugStep("Invalid Port!\n");
		return false;
	}

	return true;
}

void servExplainAndDie(char **argv) {
    printf("\nInvalid Input\n");
    printf("Usage: %s [ip type] [port number]>\n", argv[0]);
	printf("Example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

/**
 * @brief Initialize new equipment with all sensors deactivated.
 * 
 * @param id  TODO...
 * @return Equipment 
 */
Equipment getNewEquipment(const char* id) {
    Equipment E;
    strcpy(E.id, id);
    E.sensors[0] = false;
    E.sensors[1] = false;
    E.sensors[2] = false;
    E.sensors[3] = false;
    return E;
}