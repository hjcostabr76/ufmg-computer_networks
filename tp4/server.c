#include "common.h"

// #include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
// #include <sys/socket.h>
#include <inttypes.h>
#include <arpa/inet.h>
// #include <sys/time.h>
#include <string.h>
#include <unistd.h>
// #include <errno.h>

struct ClientData {
    int socket;
};

/**
 * ------------------------------------------------
 * == HEADERS =====================================
 * ------------------------------------------------
 */

bool servValidateInput(int argc, char **argv);
void servExplainAndDie(char **argv);

void servReceiveMsg(int cliSocket, char buffer[BUF_SIZE]);
// void servExecuteCommand(Command* cmd, Equipment allEquipments[EQUIP_COUNT], char answer[BUF_SIZE]);

// void handleError(const ErrCodeEnum error, char* input, char* answer, bool* mustFinish);

void *servThreadClientHandler(void *data);
void servThreadCloseOnError(const struct ClientData *client, const char *errMsg);

/**
 * ------------------------------------------------
 * == MAIN ========================================
 * ------------------------------------------------
 */

Equipment equipments[MAX_CONNECTIONS];

int main(int argc, char **argv) {

	// Validate initialization command
	comDebugStep("Validating input...");
    if (!servValidateInput(argc, argv))
        servExplainAndDie(argv);

    // Create socket
    comDebugStep("Creating server socket...");
    const char *portStr = argv[1];
    const int ipVersion = 4;
    int servSocket = netListen(portStr, TIMEOUT_CONN_SECS, MAX_CONNECTIONS, &ipVersion);

    if (DEBUG_ENABLE) {

        char dbgTxt[BUF_SIZE] = "";
        char boundAddr[200] = "";
        if (!netSetSocketAddrString(servSocket, boundAddr)) {
            sprintf(dbgTxt, "Failure as trying to exhibit bound address...");
            comLogErrorAndDie(dbgTxt);
        }

        sprintf(dbgTxt, "All set! Server is bound to %s:%s\nWaiting for connections...", boundAddr, portStr);
        comDebugStep(dbgTxt);
    }

    // Initialize equipments list
    for (int i = 0; i < MAX_CONNECTIONS; i++)
        equipments[i] = getEmptyEquipment();

    // Accept & open thread to handle new client
    while (true) {
        int cliSocket = netAccept(servSocket);
        pthread_t threadID;
        struct ClientData clientData = { cliSocket };
        pthread_create(&threadID, NULL, servThreadClientHandler, &clientData);
    }

    exit(EXIT_SUCCESS);
}

/**
 * TODO: 2021-06-07 - ADD Descricao
 */
void *servThreadClientHandler(void *threadInput) {

    comDebugStep("[thread] Starting new thread...");
    char notificationMsg[BUF_SIZE];
    
    // Parse input
    struct ClientData *client = (struct ClientData *)threadInput;

    if (DEBUG_ENABLE) {
        char clientAddrStr[INET6_ADDRSTRLEN + 1] = "";
        if (netSetSocketAddrString(client->socket, clientAddrStr)) {
            memset(notificationMsg, 0, BUF_SIZE);
            sprintf(notificationMsg, "[thread] Connected to client at %s...", clientAddrStr);
            comDebugStep(notificationMsg);
        }
    }

    // RECV
    char buffer[BUF_SIZE];
    comDebugStep("[thread] Receiving text length...");
    servReceiveMsg(client->socket, buffer);

    // SEND
    char answer[1000];
    netSend(client->socket, answer);

    // End thread successfully
	comDebugStep("[thread] Done!");
    close(client->socket);
    pthread_exit(EXIT_SUCCESS);
}

/**
 * ------------------------------------------------
 * == AUXILIARY ===================================
 * ------------------------------------------------
 */

bool servValidateInput(int argc, char **argv) {

	if (argc != 2) {
        comDebugStep("Invalid argc!\n");
		return false;
    }

    // Validate port
	const char *portStr = argv[1];
	if (!strIsNumeric(portStr)) {
		comDebugStep("Invalid Port!\n");
		return false;
	}

	return true;
}

void servExplainAndDie(char **argv) {
    printf("\nInvalid Input\n");
    printf("Usage: %s [port number]>\n", argv[0]);
	printf("Example: %s %d\n", argv[0], PORT_DEFAULT);
    exit(EXIT_FAILURE);
}

void servReceiveMsg(const int cliSocket, char buffer[BUF_SIZE]) {
    
    comDebugStep("Waiting for command...");
    memset(buffer, 0, BUF_SIZE);
    
    ssize_t receivedBytes = netRecv(cliSocket, buffer, TIMEOUT_TRANSFER_SECS);
    if (receivedBytes == -1)
        comLogErrorAndDie("Failure as trying to receive messages from client");

    if (DEBUG_ENABLE) {
        char aux[BUF_SIZE];
        sprintf(aux, "Received buffer: '%s'", buffer);
        comDebugStep(aux);
    }
}

void serverCloseThreadOnError(const struct ClientData *client, const char *errMsg) {
	close(client->socket);
    perror(errMsg);
    puts("\nClosing thread because of failure... :(\n");
    pthread_exit(NULL);
}