#include "common/common.h"
#include "common/string_utils.h"
#include "common/posix_utils.h"
#include "server_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

/**
 * NOTE: Funcao 'privada'
 */
void serverCloseThreadOnError(const struct ClientData *client, const char *errMsg) {
	close(client->socket);
    perror("\nClosing thread because of failure");
    puts("");
    pthread_exit(NULL);
}

/**
 * NOTE: Funcao 'privada'
 */
void serverSendFailureResponse(struct ClientData *client, const char *errMsg) {
	posixSend(client->socket, "0", 1, &client->timeout);
	serverCloseThreadOnError(client, errMsg);
}

int serverValidateInput(int argc, char **argv) {

	if (argc != 2) {
        commonDebugStep("Invalid argc!\n");
		return 0;
    }

	const char *portStr = argv[1];
	if (!stringValidateNumericString(portStr, strlen(portStr))) {
		commonDebugStep("Invalid Port!\n");
		return 0;
	}

	return 1;
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
    if ((validationType == RCV_VALIDATION_NUMERIC && !stringValidateNumericString(buffer, strlen(buffer)))
        || (validationType == RCV_VALIDATION_LCASE && !stringValidateLCaseString(buffer, strlen(buffer)))
    ) {
        sprintf(errMsg, "Invalid data sent by client [%s]: \"%.800s\"", opLabel, buffer);
        serverSendFailureResponse(client, errMsg);
    }

    if (!posixSend(client->socket, "1", 1, &client->timeout)) {
        sprintf(errMsg, "Failure as sending receiving confirmation to client [%s]", opLabel);
        serverSendFailureResponse(client, errMsg);
    }
}
