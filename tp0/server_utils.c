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
 * NOTE: Funcao eh 'privada'
 */
short int serverRecvParamAux(
    const int socketFD,
    char *buffer,
    const unsigned bytesToRecv,
    const enum ServerRecvValidationEnum validationType,
    struct timeval *timeout
) {

    // Recebe valor do cliente
    unsigned receivedBytes = 0;
    posixRecv(socketFD, buffer, &receivedBytes, timeout);

    // Validar: Contagem de butes
    if (receivedBytes < bytesToRecv)
        return RCV_ERR_BYTES_COUNT;

    // Validar: Conteudo recebido
    const int bufLen = sizeof(buffer);

    switch (validationType) {
        case RCV_VALIDATION_NUMERIC:
            if (!stringValidateNumericString(buffer, bufLen))
                return RCV_ERR_VALIDATION_STR;
            break;
        case RCV_VALIDATION_LCASE:
            if (!stringValidateLCaseString(buffer, bufLen))
                return RCV_ERR_VALIDATION_STR;
            break;
        default:
            return RCV_ERR_VALIDATION_PARAM;
    }
    
    // Notifica sucesso no recebimento
    return posixSend(socketFD, "1", 1, timeout) ? RCV_SUCCESS : RCV_ERR_SUCCESS_RETURN;
}

void serverSendFailureResponse(const int sock, char *errMsg, struct timeval *timeout) {
	posixSend(sock, "0", 1, timeout);
	close(sock);
    pthread_exit(NULL);
	commonLogErrorAndDie(errMsg);
}

int serverValidateInput(int argc, char **argv) {

	if (argc != 2) {
        commonDebugStep("ERROR: Invalid argc!\n");
		return 0;
    }

	const char *portStr = argv[1];
	if (!stringValidateNumericString(portStr, strlen(portStr))) {
		commonDebugStep("ERROR: Invalid Port!\n");
		return 0;
	}

	return 1;
}

void serverRecvParam(
    const int socketFD,
    char *buffer,
    const unsigned bytesToRecv,
    const enum ServerRecvValidationEnum validationType,
    struct timeval *timeout,
    const char *opLabel
) {

    short int recvStatus = serverRecvParamAux(socketFD, buffer, bytesToRecv, RCV_VALIDATION_NUMERIC, timeout);
    if (recvStatus != RCV_SUCCESS)
        return;

    char aux[BUF_SIZE];

    if (recvStatus == RCV_ERR_VALIDATION_STR)
        sprintf(aux, "Invalid data sent by client [%d / %s]: \"%.800s\"", recvStatus, opLabel, buffer);
    else
        sprintf(aux, "Failure as trying to get data from client [%d / %s]: \"%.800s\"", recvStatus, opLabel, buffer);

    serverSendFailureResponse(socketFD, aux, timeout);
}
