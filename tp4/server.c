#include "common.h"

// #include <stdio.h>
#include <stdlib.h>
// #include <pthread.h>
// #include <sys/socket.h>
// #include <inttypes.h>
#include <arpa/inet.h>
// #include <sys/time.h>
// #include <string.h>
// #include <unistd.h>
// #include <errno.h>

struct ConnThreadData {
    int socket;
    int addrFamily;
};

struct ClientData {
    int socket;
    struct sockaddr_storage address;
    struct timeval timeout;
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


int main(int argc, char **argv) {

	// Validate initialization command
	comDebugStep("Validating input...");
    if (!servValidateInput(argc, argv))
        servExplainAndDie(argv);

    // Create socket
    comDebugStep("Creating server socket...");
    const int port = atoi(argv[2]);
    int servSocket = netListen(port, TIMEOUT_CONN_SECS, MAX_CONNECTIONS);

    const int dbgTxtLength = BUF_SIZE;
    char dbgTxt[dbgTxtLength];
    
    if (DEBUG_ENABLE) {

        char boundAddr[200];
        memset(dbgTxt, 0, dbgTxtLength);
        if (!netSetSocketAddressString(servSocket, boundAddr)) {
            sprintf(dbgTxt, "Failure as trying to exhibit bound address...");
            comLogErrorAndDie(dbgTxt);
        }

        sprintf(dbgTxt, "All set! Server is bound to %s:%d\nWaiting for connections...", boundAddr, port);
        comDebugStep(dbgTxt);
    }

    // Initialize equipments list
    Equipment equipments[MAX_CONNECTIONS];
    for (int i = 0; i < MAX_CONNECTIONS; i++)
        equipments[i] = getEmptyEquipment();

    while (true) {

        // Criar socket para receber conexoes de clientes
        struct sockaddr_storage clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);

        // Accept client
        int cliSocket = netAccept(servSocket);

        // Define dados para thread de tratamento da nova conexao
        struct ClientData *clientData = malloc(sizeof(*clientData));
        if (!clientData)
            comLogErrorAndDie("Failure as trying to set new client connection data");

        struct timeval timeoutTransfer;
        timeoutTransfer.tv_sec = TIMEOUT_TRANSFER_SECS;
        timeoutTransfer.tv_usec = 0;

        clientData->socket = cliSocket;
        memcpy(&(clientData->address), &clientAddr, sizeof(clientAddr));
        memcpy(&(clientData->timeout), &timeoutTransfer, sizeof(timeoutTransfer));

        // Inicia nova thread para tratar a nova conexao
        pthread_t tid;
        pthread_create(&tid, NULL, servThreadClientHandler, clientData);
    }

    exit(EXIT_SUCCESS);
}

/**
 * TODO: 2021-06-07 - ADD Descricao
 */
void *servThreadClientHandler(void *threadInput) {

    comDebugStep("\n[thread] Starting new thread..\n");
    char errMsg[BUF_SIZE];
    
    // Avalia entrada
    struct ClientData *client = (struct ClientData *)threadInput;
    struct sockaddr *clientAddr = (struct sockaddr *)(&client->address);

    // Notifica origem da conexao
    if (DEBUG_ENABLE) {
        char clientAddrStr[INET_ADDRSTRLEN + 1] = "";
        if (posixAddressToString(clientAddr, clientAddrStr)) {
            memset(errMsg, 0, BUF_SIZE);
            sprintf(errMsg, "[thread] Connected to client at %s...\n", clientAddrStr);
            comDebugStep(errMsg);
        }
    }


    // RECV
    char buffer[BUF_SIZE];
    size_t receivedBytes = posixRecv(client->socket, buffer, &client->timeout);
    if (receivedBytes == -1) {
        // sprintf(errMsg, "Failure as receiving data from client [%s] [2]", opLabel);
        servThreadCloseOnError(client, errMsg);
    }


    // SEND
    comDebugStep("[thread] Receiving text length...\n");
    int bytesToReceive = sizeof(uint32_t);
    servReceiveMsg(client->socket, buffer);

    // char answer[10];
    // netSend(client->socket, answer);

    // End thread successfully
	comDebugStep("\n[thread] Done!\n");
    close(client->socket);
    pthread_exit(EXIT_SUCCESS);
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
	if (!strIsNumeric(portStr)) {
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

void servReceiveMsg(const int cliSocket, char buffer[BUF_SIZE]) {
    
    comDebugStep("Waiting for command...");
    memset(buffer, 0, BUF_SIZE);
    
    size_t receivedBytes = netRecv(cliSocket, buffer, TIMEOUT_TRANSFER_SECS);
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