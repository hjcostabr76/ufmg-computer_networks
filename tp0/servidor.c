#include "common.h"
#include "caesar_cipher.h"
#include "server_utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define SIZE_TXT_BYTES 8
#define MAX_CONNECTIONS 2

struct ClientData { int socket; struct sockaddr_storage address; };

void explainAndDie(char **argv) {
    printf("Invalid Input\n");
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
 * 
 */
int main(int argc, char **argv) {

	commonDebugStep("\nStarting...\n");

    /*=================================================== */
    /*-- Validar entrada -------------------------------- */

	commonDebugStep("Validating input...\n");
    if (!serverValidateInput(argc, argv)) {
        explainAndDie(argv);
    }

    /*=================================================== */
    /*-- Receber porto ---------------------------------- */

    // Estabelece endereco em que o servidor vai aguardar conexoes 
    commonDebugStep("Setting server address...\n");
    
    struct sockaddr_storage serverAddress;
    const char serverPortStr = argv[1];
    if (!serverInitSocket(serverPortStr, &serverAddress)) {
        explainAndDie(argv);
    }

    /*=================================================== */
    /*-- Criar socket para receber conexoes ------------- */

    commonDebugStep("Creating server socket...\n");

    int serverSocket;
    serverSocket = socket(serverAddress.ss_family, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        commonLogErrorAndDie("Failure as creating server socket [1]");
    }

    // Evitar que porta utlizada numa execucao fique 02 min inativa apos sua conclusao
    commonDebugStep("Enabling server address reusing...\n");

    int enableAddressReusing = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &enableAddressReusing, sizeof(int)) != 0) {
        commonLogErrorAndDie("Failure as creating server socket [2]");
    }

    // Define timeout de escuta
    commonDebugStep("Setting server listening timeout...\n");

    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SECS;

    if (setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0) {
        commonLogErrorAndDie("Failure as creating server socket [3]");
    }

    /* ================================================== */
    /* -- Inicializar espera por conexoes --------------- */

    commonDebugStep("Binding server to it's address...\n");

    struct sockaddr *_address = (struct sockaddr *)(&serverAddress);
    if (bind(serverSocket, _address, sizeof(serverAddress)) != 0) {
        commonLogErrorAndDie("Failure as biding server socket");
    }

    commonDebugStep("Starting to listen...\n");

    if (listen(serverSocket, MAX_CONNECTIONS) != 0) {
        commonLogErrorAndDie("Failure as starting server listening");
    }

    if (DEBUG_ENABLE) {
        
        char serverAddressStr[SIZE_BUFFER];
        memset(serverAddressStr, 0, SIZE_BUFFER);
        addressToString(_address, serverAddressStr);
        
        char aux[200];
        memset(aux, 0, SIZE_BUFFER);
        sprintf(aux, "\nAll set! Server is bound to %s:%s and waiting for connections...\n", serverAddressStr, serverPortStr);
        commonDebugStep(aux);
    }

    /* ================================================== */
    /* -- Escutar conexoes ------------------------------ */

    while (1) {

        // Criar socket para receber conexoes de clientes
        struct sockaddr_storage clientAddress;
        struct sockaddr *_clientAddress = (struct sockaddr *)(&clientAddress);
        socklen_t clientAddressLength = sizeof(clientAddress);

        int clientSocket = accept(serverSocket, _clientAddress, &clientAddressLength);
        if (clientSocket == -1) {
            commonLogErrorAndDie("Failure as trying to accept client connection");
        }

        // Define dados para thread de tratamento da nova conexao
        struct ClientData *clientData = malloc(sizeof(*clientData));
        if (!clientData) {
            commonLogErrorAndDie("Failure as trying to set new client connection data");
        }

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
    if (!addressToString(clientAddr, clientAddrStr)) {
        commonLogErrorAndDie("Failure as parsing client address");
    }
    
    if (DEBUG_ENABLE) {
        char aux[200];
        memset(aux, 0, SIZE_BUFFER);
        sprintf(aux, "[thread] Connected to client at %s...\n", clientAddrStr);
        commonDebugStep(aux);
    }

    /* ================================================== */
    /* -- Receber tamanho do texto a ser decodificado --- */
    
    commonDebugStep("[thread] Receiving text length...\n");
    
    char txtLengthStr[SIZE_BUFFER];
    memset(txtLengthStr, 0, SIZE_BUFFER);
    int receivingStatus = serverReceiveParam(clientData->socket, txtLengthStr, SIZE_TXT_BYTES, RCV_VALIDATION_NUMERIC);

    if (receivingStatus == RCV_ERR_VALIDATION_STR) {
        char aux[200];
        sprintf(aux, "Invalid text length '%s' sent by client [%d]", txtLengthStr, receivingStatus);
        serverSendFailureResponse(clientData->socket, aux);

    } else if (receivingStatus != RCV_SUCCESS) {
        serverSendFailureResponse(clientData->socket, "Failure as trying to get message length");
    }

    uint32_t txtLength = atoi(txtLengthStr);
    txtLength = htonl(txtLength);

    if (DEBUG_ENABLE) {
        char aux[200];
        sprintf(aux, "\tText Length: \"%ul\"\n", txtLength);
        commonDebugStep(aux);
    }

    /* ================================================== */
    /* -- Receber texto cifrado ------------------------- */

    commonDebugStep("[thread] Receiving ciphered text...\n");
    
    char cipheredText[SIZE_BUFFER];
    memset(cipheredText, 0, SIZE_BUFFER);
    receivingStatus = serverReceiveParam(clientData->socket, cipheredText, txtLength, RCV_VALIDATION_LCASE);

    if (receivingStatus == RCV_ERR_VALIDATION_STR) {
        char aux[200];
        sprintf(aux, "Invalid ciphered text received: \"%s\" [%d]", cipheredText, receivingStatus);
        serverSendFailureResponse(clientData->socket, aux);

    } else if (receivingStatus != RCV_SUCCESS) {
        serverSendFailureResponse(clientData->socket, "Failure as trying to get ciphered text");
    }

    if (DEBUG_ENABLE) {
        char aux[200];
        sprintf(aux, "\tCiphered text is: \"%s\"\n", cipheredText);
        commonDebugStep(aux);
    }

    /*=================================================== */
    /*-- Receber chave da cifra ------------------------- */

    commonDebugStep("[thread] Receiving cipher key...\n");
    
    char cipherKeyStr[SIZE_BUFFER];
    memset(cipherKeyStr, 0, SIZE_BUFFER);
    receivingStatus = serverReceiveParam(clientData->socket, cipherKeyStr, SIZE_TXT_BYTES, RCV_VALIDATION_NUMERIC);

    if (receivingStatus == RCV_ERR_VALIDATION_STR) {
        char aux[200];
        sprintf(aux, "Invalid cipher key '%s' sent by client [%d]", cipherKeyStr, receivingStatus);
        serverSendFailureResponse(clientData->socket, aux);

    } else if (receivingStatus != RCV_SUCCESS) {
        serverSendFailureResponse(clientData->socket, "Failure as trying to get cipher key");
    }

    uint32_t cipherKey = atoi(cipherKeyStr);
    cipherKey = htonl(cipherKey);

    if (DEBUG_ENABLE) {
        char aux[200];
        sprintf(aux, "\tCipher key: \"%ul\"\n", cipherKey);
        commonDebugStep(aux);
    }

    /*=================================================== */
    /*-- Decodificar texto ------------------------------ */

    commonDebugStep("[thread] Decrypting text...\n");

	char text[txtLength];
	memset(text, 0, txtLength);
	caesarDecipher(cipheredText, txtLength, text, txtLength);

    commonDebugStep("\tText successfully decrypted:\n");
    puts(text);

    /*=================================================== */
    /*-- Enviar string decodificada --------------------- */

    commonDebugStep("[thread] Sending answer to client...\n");

    if (!posixSend(clientData->socket, text, txtLength)) {
		commonLogErrorAndDie("Failure as sending answer to client");
	}

    // Encerra conexao
	commonDebugStep("\n[thread] Done!\n");
    close(clientData->socket);
    pthread_exit(EXIT_SUCCESS);
}

/*
    Set socket FD's option OPTNAME at protocol level LEVEL
    to *OPTVAL (which is OPTLEN bytes long).
    Returns 0 on success, -1 for errors.
*/
// extern int setsockopt (int __fd, int __level, int __optname, const void *__optval, socklen_t __optlen) __THROW;

// SO_RCVTIMEO SO_SNDTIMEO

// Specify the receiving or sending timeouts until reporting an error.  The argument  is  a
// struct timeval.  If an input or output function blocks for this period of time, and data
// has been sent or received, the return value of that function will be the amount of  data
// transferred;  if  no data has been transferred and the timeout has been reached, then -1
// is returned with errno set to EAGAIN or EWOULDBLOCK,  or  EINPROGRESS  (for  connect(2))
// just  as  if  the socket was specified to be nonblocking.  If the timeout is set to zero
// (the default), then the operation will never timeout.  Timeouts  only  have  effect  for
// system  calls  that perform socket I/O (e.g., read(2), recvmsg(2), send(2), sendmsg(2));
// timeouts have no effect for select(2), poll(2), epoll_wait(2), and so on.