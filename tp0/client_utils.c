#include "common/common.h"
#include "common/string_utils.h"
#include "common/posix_utils.h"
#include "client_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <arpa/inet.h>
#include <errno.h>

int clientValidateInput(int argc, char **argv) {

	if (argc != 5) {
		commonDebugStep("Invalid argc!\n");
		return 0;
	}

	const char *portStr = argv[2];
	if (!stringValidateNumericString(portStr, strlen(portStr))) {
		commonDebugStep("Invalid Port!\n");
		return 0;
	}

	const char *msg = argv[3];
	if (!stringValidateLCaseString(msg, strlen(msg))) {
		commonDebugStep("Invalid Text!\n");
		return 0;
	}

	const char *cipherKeyStr = argv[4];
	if (!stringValidateNumericString(cipherKeyStr, strlen(cipherKeyStr))) {
		commonDebugStep("Invalid Cipher Key!\n");
		return 0;
	}

	return 1;
}

void clientSendNumericParam(
	const int socketFD,
	char *buffer,
	const uint32_t valueToSend,
	struct timeval *timeout,
	const char *opLabel
) {

	char errMsg[BUF_SIZE];

	// Preparar conteudo do buffer
	memset(buffer, 0, BUF_SIZE);
	snprintf(buffer, BUF_SIZE, "%u", valueToSend);

	// Enviar parametro
	int bytesToSend = strlen(buffer) + 1;
	if (!posixSend(socketFD, buffer, bytesToSend, timeout)) {
		sprintf(errMsg, "Failure as sending data to server [%s] [1]", opLabel);
		commonLogErrorAndDie(errMsg);
	}

	// Receber retorno
	memset(buffer, 0, BUF_SIZE);
	ssize_t receivedBytes = posixRecv(socketFD, buffer, timeout);
	if (receivedBytes == -1) {
		sprintf(errMsg, "Failure as sending data to server [%s] [2]", opLabel);
		commonLogErrorAndDie(errMsg);
	}

	if (strcmp(buffer, "1") != 0) {
		sprintf(errMsg, "Server sent no success confirmation [%s]: %d - \"%.500s\"", opLabel, errno, buffer);
		commonLogErrorAndDie(errMsg);
	}
}