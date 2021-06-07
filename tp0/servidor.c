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
#include <errno.h>

#define MAX_CONNECTIONS 20

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

	commonDebugStep("Validating input...\n");
    if (!serverValidateInput(argc, argv)) {
        explainAndDie(argv);
    }

    commonDebugStep("Creating server socket...\n");

    struct timeval timeoutConn;
    timeoutConn.tv_sec = TIMEOUT_CONN_SECS;
    timeoutConn.tv_usec = 0;

    const char *portStr = argv[1];
    char boundAddr[200];
    int serverSocket = posixListen(atoi(portStr), &timeoutConn, MAX_CONNECTIONS, boundAddr);

    if (DEBUG_ENABLE) {
        char aux[500];
        memset(aux, 0, 500);
        sprintf(aux, "\nAll set! Server is bound to %s:%s and waiting for connections...\n", boundAddr, portStr);
        commonDebugStep(aux);
    }

    while (1) {

        // Criar socket para receber conexoes de clientes
        struct sockaddr_storage clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);

        int clientSocket = accept(serverSocket, (struct sockaddr *)(&clientAddr), &clientAddrLen);
        if (clientSocket == -1) {
            
            if (errno != EAGAIN && errno != EWOULDBLOCK)
                commonLogErrorAndDie("Failure as trying to accept client connection");

            commonDebugStep("\nDisconnecting server because of innactivity...\n\n");
            break;
        }
            

        // Define dados para thread de tratamento da nova conexao
        struct ClientData *clientData = malloc(sizeof(*clientData));
        if (!clientData)
            commonLogErrorAndDie("Failure as trying to set new client connection data");

        struct timeval timeoutTransfer;
        timeoutTransfer.tv_sec = TIMEOUT_TRANSFER_SECS;
        timeoutTransfer.tv_usec = 0;

        clientData->socket = clientSocket;
        memcpy(&(clientData->address), &clientAddr, sizeof(clientAddr));
        memcpy(&(clientData->timeout), &timeoutTransfer, sizeof(timeoutTransfer));

        // Inicia nova thread para tratar a nova conexao
        pthread_t tid;
        pthread_create(&tid, NULL, threadClientConnectionHandler, clientData);
    }

    exit(EXIT_SUCCESS);
}

void *threadClientConnectionHandler(void *threadInput) {

    commonDebugStep("\n[thread] Starting new thread..\n");
    
    // Avalia entrada
    struct ClientData *client = (struct ClientData *)threadInput;
    struct sockaddr *clientAddr = (struct sockaddr *)(&client->address);

    // Notifica origem da conexao
    if (DEBUG_ENABLE) {
        char clientAddrStr[INET_ADDRSTRLEN + 1] = "";
        if (posixAddressToString(clientAddr, clientAddrStr)) {
            char aux[200];
            memset(aux, 0, 200);
            sprintf(aux, "[thread] Connected to client at %s...\n", clientAddrStr);
            commonDebugStep(aux);
        }
    }

    // Receber tamanho do texto a ser decodificado
    char buffer[BUF_SIZE];
    
    commonDebugStep("[thread] Receiving text length...\n");
    int bytesToReceive = sizeof(uint32_t);
    serverRecvParam(client, buffer, bytesToReceive, RCV_VALIDATION_NUMERIC, "text length");
    const uint32_t txtLength = ntohl(atoi(buffer));

    if (DEBUG_ENABLE) {
        char aux[200];
        sprintf(aux, "\tText Length: \"%u\"\n", txtLength);
        commonDebugStep(aux);
    }

    // Receber chave da cifra
    commonDebugStep("[thread] Receiving cipher key...\n");
    memset(buffer, 0, BUF_SIZE);
    bytesToReceive = sizeof(uint32_t);
    serverRecvParam(client, buffer, bytesToReceive, RCV_VALIDATION_NUMERIC, "cipher key");
    const uint32_t cipherKey = htonl(atoi(buffer));

    if (DEBUG_ENABLE) {
        char aux[200];
        sprintf(aux, "\tCipher key: \"%u\"\n", cipherKey);
        commonDebugStep(aux);
    }

    // Receber texto cifrado
    commonDebugStep("[thread] Receiving ciphered text...\n");
    memset(buffer, 0, BUF_SIZE);
    bytesToReceive = txtLength;
    serverRecvParam(client, buffer, bytesToReceive, RCV_VALIDATION_LCASE, "ciphered text");
    
    if (DEBUG_ENABLE) {
        char aux[BUF_SIZE];
        sprintf(aux, "\tCiphered text is: \"%.600s...\"\n", buffer);
        commonDebugStep(aux);
    }

    // Decodificar texto
    commonDebugStep("[thread] Decrypting text...\n");
	
    char text[txtLength];
	memset(text, 0, txtLength);
	caesarDecipher(buffer, txtLength, text, txtLength);
    commonDebugStep("\tText successfully decrypted:\n");
    
    puts(text);

    // Enviar
    commonDebugStep("[thread] Sending answer to client...\n");
    if (!posixSend(client->socket, text, txtLength, &client->timeout))
        commonLogErrorAndDie("Failure as sending answer to client");

    // Encerra conexao
	commonDebugStep("\n[thread] Done!\n");
    close(client->socket);
    pthread_exit(EXIT_SUCCESS);
}
