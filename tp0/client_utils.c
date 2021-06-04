#include "common/common.h"
#include "common/string_utils.h"
#include "common/posix_utils.h"
#include "client_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>

int clientValidateInput(int argc, char **argv) {

	if (argc != 5) {
        commonDebugStep("ERROR: Invalid argc!\n");
		return 0;
    }

	const char *portStr = argv[2];
	if (!stringValidateNumericString(portStr, strlen(portStr))) {
		commonDebugStep("ERROR: Invalid Port!\n");
		return 0;
	}

	const char *msg = argv[3];
	if (!stringValidateLCaseString(msg, strlen(msg))) {
		commonDebugStep("ERROR: Invalid Text!\n");
		return 0;
	}

	const char *cipherKeyStr = argv[4];
	if (!stringValidateNumericString(cipherKeyStr, strlen(cipherKeyStr))) {
		commonDebugStep("ERROR: Invalid Cipher Key!\n");
		return 0;
	}

	return 1;
}

void clientSendNumericParam(
	const int socketFD,
	char *buffer,
	const int valueToSend,
	struct timeval *timeout,
	const int opNum
) {

    // Preparar conteudo do buffer
	memset(buffer, 0, BUF_SIZE);
    snprintf(buffer, BUF_SIZE, "%d", valueToSend);

    // Enviar parametro
	int bytesToSend = strlen(buffer) + 1;
	if (!posixSend(socketFD, buffer, bytesToSend, timeout)) {
        char aux[100];
        sprintf(aux, "Failure as sending data to server [%d]", opNum);
        commonLogErrorAndDie(aux);
    }

	// Receber retorno
	unsigned receivedBytes = 0;
	memset(buffer, 0, BUF_SIZE);
	posixRecv(socketFD, buffer, &receivedBytes, timeout);

	if (strcmp(buffer, "1") != 0) {
        char aux[100];
        sprintf(aux, "Server sent failure response [%d]", opNum);
        commonLogErrorAndDie(aux);
	}
}