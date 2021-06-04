#include "common/common.h"
#include "common/caesar_cipher.h"
#include "common/posix_utils.h"
#include "server_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <string.h>
#include <unistd.h>

#define MAX_CONNECTIONS 2

struct ClientData { int socket; struct sockaddr_storage address; };

void explainAndDie(char **argv) {
    printf("\nInvalid Input\n");
    printf("Usage: %s server port>\n", argv[0]);
	printf("Example: %s 5000\n", argv[0]);
    exit(EXIT_FAILURE);
}

void *threadClientConnectionHandler(void *data);

/**
 * ------------------------------------------------
 * == Programa Servidor ===========================
 * ------------------------------------------------
 *
 * Etapas:
 * [server] Criar socket -> ??;
 * [server] Bind -> ??;
 * [server] Listen -> ??;
 * [server] Accept -> (Gera socket do cliente)
 * 
 * TODO: 2021-06-02 - Resolver todo's
 * TODO: 2021-06-04 - Abstrair operacao de bind
 * 
 */
int main(int argc, char **argv) {

	commonDebugStep("\nStarting...\n\n");

    /*=================================================== */
    /*-- Validar entrada -------------------------------- */

	commonDebugStep("Validating input...\n");
    if (!serverValidateInput(argc, argv)) {
        explainAndDie(argv);
    }

    /*=================================================== */
    /*-- Receber porto ---------------------------------- */

    commonDebugStep("Setting server address...\n");
    
    struct sockaddr_storage serverAddress;
    const char *serverPortStr = argv[1];
    if (!serverInitSocket(serverPortStr, &serverAddress)) // Estabelece endereco em que o servidor vai aguardar conexoes 
        explainAndDie(argv);

    /*=================================================== */
    /*-- Criar socket para receber conexoes ------------- */

    commonDebugStep("Creating server socket...\n");

    int serverSocket = socket(serverAddress.ss_family, SOCK_STREAM, 0);
    if (serverSocket == -1)
        commonLogErrorAndDie("Failure as creating server socket [1]");

    // Evitar que porta utlizada numa execucao fique 02 min inativa apos sua conclusao
    commonDebugStep("Enabling server address reusing...\n");

    int enableAddressReusing = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &enableAddressReusing, sizeof(int)) != 0)
        commonLogErrorAndDie("Failure as creating server socket [2]");

    // Define timeout de escuta
    commonDebugStep("Setting server listening timeout...\n");

    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SECS;
    timeout.tv_usec = 0;

    if (setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0)
        commonLogErrorAndDie("Failure as creating server socket [3]");

    /* ================================================== */
    /* -- Inicializar espera por conexoes --------------- */

    commonDebugStep("Binding server to it's address...\n");

    struct sockaddr *_address = (struct sockaddr *)(&serverAddress);
    if (bind(serverSocket, _address, sizeof(serverAddress)) != 0)
        commonLogErrorAndDie("Failure as biding server socket");

    commonDebugStep("Starting to listen...\n");

    if (listen(serverSocket, MAX_CONNECTIONS) != 0)
        commonLogErrorAndDie("Failure as starting server listening");

    if (DEBUG_ENABLE) {
        
        char serverAddressStr[100];
        memset(serverAddressStr, 0, 100);
        posixAddressToString(_address, serverAddressStr);
        
        char aux[500];
        memset(aux, 0, 500);
        sprintf(aux, "\nAll set! Server is bound to %s:%s and waiting for connections...\n", serverAddressStr, serverPortStr);
        commonDebugStep(aux);
    }

    /* ================================================== */
    /* -- Escutar conexoes ------------------------------ */

    while (1) {

        // Criar socket para receber conexoes de clientes
        struct sockaddr_storage clientAddress;
        socklen_t clientAddressLength = sizeof(clientAddress);

        int clientSocket = accept(serverSocket, (struct sockaddr *)(&clientAddress), &clientAddressLength);
        if (clientSocket == -1)
            commonLogErrorAndDie("Failure as trying to accept client connection");

        // Define dados para thread de tratamento da nova conexao
        struct ClientData *clientData = malloc(sizeof(*clientData));
        if (!clientData)
            commonLogErrorAndDie("Failure as trying to set new client connection data");

        clientData->socket = clientSocket;
        memcpy(&(clientData->address), &clientAddress, sizeof(clientAddress));

        // Inicia nova thread para tratar a nova conexao
        pthread_t tid; // Thread ID
        pthread_create(&tid, NULL, threadClientConnectionHandler, clientData);
    }

    exit(EXIT_SUCCESS);
}

void *threadClientConnectionHandler(void *data) {

    commonDebugStep("\n[thread] Starting...\n");

    // Avalia entrada
    struct ClientData *clientData = (struct ClientData *)data;
    struct sockaddr *clientAddr = (struct sockaddr *)(&clientData->address);

    // Notifica origem da conexao
    char clientAddrStr[INET_ADDRSTRLEN + 1] = "";
    if (!posixAddressToString(clientAddr, clientAddrStr)) {
        commonLogErrorAndDie("Failure as parsing client address");
    }
    
    if (DEBUG_ENABLE) {
        char aux[200];
        memset(aux, 0, BUF_SIZE);
        sprintf(aux, "[thread] Connected to client at %s...\n", clientAddrStr);
        commonDebugStep(aux);
    }

    /* ================================================== */
    /* -- Receber tamanho do texto a ser decodificado --- */
    
    char buffer[BUF_SIZE];

    commonDebugStep("[thread] Receiving text length...\n");
    
    memset(buffer, 0, BUF_SIZE);
    int bytesToReceive = sizeof(uint32_t);
    short int receivingStatus = serverReceiveParam(clientData->socket, buffer, bytesToReceive, RCV_VALIDATION_NUMERIC);

    if (receivingStatus == RCV_ERR_VALIDATION_STR) {
        char aux[200];
        sprintf(aux, "Invalid text length '%.10s' sent by client [%d]", buffer, receivingStatus);
        serverSendFailureResponse(clientData->socket, aux);

    } else if (receivingStatus != RCV_SUCCESS) {
        serverSendFailureResponse(clientData->socket, "Failure as trying to get message length");
    }

    const uint32_t txtLength = ntohl(atoi(buffer));

    if (DEBUG_ENABLE) {
        char aux[200];
        sprintf(aux, "\tText Length: \"%ul\"\n", txtLength);
        commonDebugStep(aux);
    }

    /*=================================================== */
    /*-- Receber chave da cifra ------------------------- */

    commonDebugStep("[thread] Receiving cipher key...\n");
    
    memset(buffer, 0, SIZE_NUMBER_STR);
    bytesToReceive = sizeof(uint32_t);
    receivingStatus = serverReceiveParam(clientData->socket, buffer, bytesToReceive, RCV_VALIDATION_NUMERIC);

    if (receivingStatus == RCV_ERR_VALIDATION_STR) {
        char aux[BUF_SIZE];
        sprintf(aux, "Invalid cipher key '%.400s' sent by client [%d]", buffer, receivingStatus);
        serverSendFailureResponse(clientData->socket, aux);

    } else if (receivingStatus != RCV_SUCCESS) {
        serverSendFailureResponse(clientData->socket, "Failure as trying to get cipher key");
    }

    const uint32_t cipherKey = htonl(atoi(buffer));

    if (DEBUG_ENABLE) {
        char aux[200];
        sprintf(aux, "\tCipher key: \"%ul\"\n", cipherKey);
        commonDebugStep(aux);
    }

    /* ================================================== */
    /* -- Receber texto cifrado ------------------------- */

    commonDebugStep("[thread] Receiving ciphered text...\n");
    
    memset(buffer, 0, BUF_SIZE);
    receivingStatus = serverReceiveParam(clientData->socket, buffer, txtLength, RCV_VALIDATION_LCASE);

    if (receivingStatus == RCV_ERR_VALIDATION_STR) {
        char aux[BUF_SIZE];
        sprintf(aux, "Invalid ciphered text received: \"%.600s...\" [%d]", buffer, receivingStatus);
        serverSendFailureResponse(clientData->socket, aux);

    } else if (receivingStatus != RCV_SUCCESS) {
        serverSendFailureResponse(clientData->socket, "Failure as trying to get ciphered text");
    }

    if (DEBUG_ENABLE) {
        char aux[BUF_SIZE];
        sprintf(aux, "\tCiphered text is: \"%.600s...\"\n", buffer);
        commonDebugStep(aux);
    }

    /*=================================================== */
    /*-- Enviar string decodificada --------------------- */

    // Decodificar
    commonDebugStep("[thread] Decrypting text...\n");
	
    char text[txtLength];
	memset(text, 0, txtLength);
	caesarDecipher(buffer, txtLength, text, txtLength);
    
    commonDebugStep("\tText successfully decrypted:\n");
    puts(text);

    // Enviar
    commonDebugStep("[thread] Sending answer to client...\n");
    if (!posixSend(clientData->socket, text, txtLength)) {
		commonLogErrorAndDie("Failure as sending answer to client");
	}

    // Encerra conexao
	commonDebugStep("\n[thread] Done!\n");
    close(clientData->socket);
    pthread_exit(EXIT_SUCCESS);
}
