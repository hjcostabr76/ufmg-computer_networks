#include "common.h"
#include "caesar_cipher.h"
#include "posix_utils.h"
#include "server_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <arpa/inet.h>

// TODO: qual das 02?
// #include <sys/time.h>
#include <time.h>
// TODO: qual das 02?

#include <string.h>
#include <unistd.h>
#include <errno.h>

#define MAX_CONNECTIONS 20

/**
 * ------------------------------------------------
 * == ABSTACTS ====================================
 * ------------------------------------------------
 */

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
 * TODO: 2021-06-04 - ADD Descricao
 */
enum ServerRecvValidationEnum {
    RCV_VALIDATION_NUMERIC = 1,
    RCV_VALIDATION_LCASE
};

/**
 * ------------------------------------------------
 * == HEADERS =====================================
 * ------------------------------------------------
 */

/**
 * TODO: 2021-06-03 - ADD Descriptions
 */

int servValidateInput(int argc, char **argv);
void servExplainAndDie(char **argv);
void *servThreadConnHandler(void *data);

void servRecvParam(
    struct ClientData *client,
    char *buffer,
    const unsigned bytesToRecv,
    const enum ServerRecvValidationEnum validationType,
    const char *opLabel
);

/**
 * ------------------------------------------------
 * == MAIN ========================================
 * ------------------------------------------------
 */

int main(int argc, char **argv) {

	comDebugStep("\nStarting...\n\n");

	comDebugStep("Validating input...\n");
    if (!servValidateInput(argc, argv))
        servExplainAndDie(argv);

    const int notificationMsgLen = 500;
    char notificationMsg[notificationMsgLen];
    const int port = atoi(argv[1]);

    struct timeval timeoutConn;
    memset(&timeoutConn, 0, sizeof(timeoutConn));
    timeoutConn.tv_sec = TIMEOUT_CONN_SECS;
    timeoutConn.tv_usec = 0;

    // Inicializa ipv6
    comDebugStep("Creating server socket [ipv6]...\n");
    char boundAddr6[200];
    memset(boundAddr6, 0, 200);
    int serverSocket = posixListen(port, AF_INET6, &timeoutConn, MAX_CONNECTIONS, boundAddr6);

    // Notifica sucesso na inicializacao
    if (DEBUG_ENABLE) {
        memset(notificationMsg, 0, notificationMsgLen);
        sprintf(notificationMsg, "\nAll set! Server is bound to %s:%d\nWaiting for connections...\n", boundAddr6, port);
        comDebugStep(notificationMsg);
    }

    while (1) {

        // Criar socket para receber conexoes de clientes
        struct sockaddr_storage clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);

        int clientSocket = accept(serverSocket, (struct sockaddr *)(&clientAddr), &clientAddrLen);
        if (clientSocket == -1) {
            
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                comLogErrorAndDie("Failure as trying to accept client connection");

            comDebugStep("\nDisconnecting server because of innactivity...\n\n");
            break;
        }

        // Define dados para thread de tratamento da nova conexao
        struct ClientData *clientData = malloc(sizeof(*clientData));
        if (!clientData)
            comLogErrorAndDie("Failure as trying to set new client connection data");

        struct timeval timeoutTransfer;
        timeoutTransfer.tv_sec = TIMEOUT_TRANSFER_SECS;
        timeoutTransfer.tv_usec = 0;

        clientData->socket = clientSocket;
        memcpy(&(clientData->address), &clientAddr, sizeof(clientAddr));
        memcpy(&(clientData->timeout), &timeoutTransfer, sizeof(timeoutTransfer));

        // Inicia nova thread para tratar a nova conexao
        pthread_t tid;
        pthread_create(&tid, NULL, servThreadConnHandler, clientData);
    }

    exit(EXIT_SUCCESS);
}

/**
 * TODO: 2021-06-07 - ADD Descricao
 */
void *servThreadConnHandler(void *threadInput) {

    comDebugStep("\n[thread: connection] Starting new thread..\n");
    char errMsg[BUF_SIZE];
    
    // Avalia entrada
    struct ClientData *client = (struct ClientData *)threadInput;
    struct sockaddr *clientAddr = (struct sockaddr *)(&client->address);

    // Notifica origem da conexao
    if (DEBUG_ENABLE) {
        char clientAddrStr[INET_ADDRSTRLEN + 1] = "";
        if (posixAddressToString(clientAddr, clientAddrStr)) {
            memset(errMsg, 0, BUF_SIZE);
            sprintf(errMsg, "[thread: connection] Connected to client at %s...\n", clientAddrStr);
            comDebugStep(errMsg);
        }
    }

    // Receber tamanho do texto a ser decodificado
    char buffer[BUF_SIZE];
    
    comDebugStep("[thread: connection] Receiving text length...\n");
    int bytesToReceive = sizeof(uint32_t);
    servRecvParam(client, buffer, bytesToReceive, RCV_VALIDATION_NUMERIC, "text length");
    const uint32_t txtLength = htonl(atoi(buffer));

    if (DEBUG_ENABLE) {
        memset(errMsg, 0, txtLength);
        sprintf(errMsg, "\tText Length: \"%u\"\n", txtLength);
        comDebugStep(errMsg);
    }

    // Receber chave da cifra
    comDebugStep("[thread: connection] Receiving cipher key...\n");
    memset(buffer, 0, BUF_SIZE);
    bytesToReceive = sizeof(uint32_t);
    servRecvParam(client, buffer, bytesToReceive, RCV_VALIDATION_NUMERIC, "cipher key");
    const uint32_t cipherKey = htonl(atoi(buffer));

    if (DEBUG_ENABLE) {
        memset(errMsg, 0, txtLength);
        sprintf(errMsg, "\tCipher key: \"%u\"\n", cipherKey);
        comDebugStep(errMsg);
    }

    // Receber texto cifrado
    comDebugStep("[thread: connection] Receiving ciphered text...\n");
    memset(buffer, 0, BUF_SIZE);
    bytesToReceive = txtLength;
    servRecvParam(client, buffer, bytesToReceive, RCV_VALIDATION_LCASE, "ciphered text");
    
    if (DEBUG_ENABLE) {
        memset(errMsg, 0, BUF_SIZE);
        sprintf(errMsg, "\tCiphered text is: \"%.600s...\"\n", buffer);
        comDebugStep(errMsg);
    }

    /**
     * TODO: 2021-06-08 - Remover essa gambiarra...
     */
    char doubleCheckBuffer[10];
    memset(doubleCheckBuffer, 0, 10);
	posixRecv(client->socket, doubleCheckBuffer, &client->timeout);
	if (strcmp(doubleCheckBuffer, "1") != 0) {
		sprintf(errMsg, "Double check failure: \"%s\"\n", doubleCheckBuffer);
		comLogErrorAndDie(errMsg);
	}

    // Decodificar texto
    comDebugStep("[thread: connection] Decrypting text...\n");
	
    char text[txtLength];
	memset(text, 0, txtLength);
	caesarDecipher(buffer, txtLength, text, cipherKey);
    comDebugStep("\tText successfully decrypted:\n");
    
    puts(text);

    // Enviar
    comDebugStep("[thread: connection] Sending answer to client...\n");
    if (!posixSend(client->socket, text, txtLength, &client->timeout))
        comLogErrorAndDie("Failure as sending answer to client");

    // Encerra conexao
	comDebugStep("\n[thread: connection] Done!\n");
    close(client->socket);
    pthread_exit(EXIT_SUCCESS);
}


/**
 * ------------------------------------------------
 * == AUXILIARY ===================================
 * ------------------------------------------------
 */

int servValidateInput(int argc, char **argv) {

	if (argc != 2) {
        comDebugStep("Invalid argc!\n");
		return 0;
    }

	const char *portStr = argv[1];
	if (!comValidateNumericString(portStr, strlen(portStr))) {
		comDebugStep("Invalid Port!\n");
		return 0;
	}

	return 1;
}

/**
 * TODO: 2022-05-12 - ADD Descricao
 */
void servExplainAndDie(char **argv) {
    printf("\nInvalid Input\n");
    printf("Usage: %s [ip type] [port number]>\n", argv[0]);
	printf("Example: %s v4 5000\n", argv[0]);
    exit(EXIT_FAILURE);
}

/**
 * NOTE: Funcao 'privada'
 */
void serverCloseThreadOnError(const struct ClientData *client, const char *errMsg) {
	close(client->socket);
    perror(errMsg);
    puts("\nClosing thread because of failure... :(\n");
    pthread_exit(NULL);
}

/**
 * NOTE: Funcao 'privada'
 */
void serverSendFailureResponse(struct ClientData *client, const char *errMsg) {
	posixSend(client->socket, "0", 1, &client->timeout);
	serverCloseThreadOnError(client, errMsg);
}

void serverRecvParam(
    struct ClientData *client,
    char *buffer,
    const unsigned bytesToRecv,
    const enum ServerRecvValidationEnum validationType,
    const char *opLabel
) {

    char errMsg[BUF_SIZE];

    // Valida parametros
    if (validationType != RCV_VALIDATION_NUMERIC && validationType != RCV_VALIDATION_LCASE) {
        sprintf(errMsg, "Failure as receiving data from client [%s] [1]", opLabel);
        serverCloseThreadOnError(client, errMsg);
    }
    
    // Recebe valor do cliente
    size_t receivedBytes = posixRecv(client->socket, buffer, &client->timeout);
    if (receivedBytes == -1) {
        sprintf(errMsg, "Failure as receiving data from client [%s] [2]", opLabel);
        serverCloseThreadOnError(client, errMsg);
    }

    // Validar: Contagem de bytes
    if (receivedBytes < bytesToRecv) {
        sprintf(errMsg, "Failure as receiving data from client [%s] [3]", opLabel);
        serverCloseThreadOnError(client, errMsg);
    }

    // Validar: Conteudo recebido
    if ((validationType == RCV_VALIDATION_NUMERIC && !comValidateNumericString(buffer, strlen(buffer)))
        || (validationType == RCV_VALIDATION_LCASE && !comValidateLCaseString(buffer, strlen(buffer)))
    ) {
        sprintf(errMsg, "Invalid data sent by client [%s]: \"%.800s\"", opLabel, buffer);
        serverSendFailureResponse(client, errMsg);
    }

    const char *aux = "1";
    if (!posixSend(client->socket, aux, 1, &client->timeout)) {
        sprintf(errMsg, "Failure as sending receiving confirmation to client [%s]", opLabel);
        serverSendFailureResponse(client, errMsg);
    }
}
