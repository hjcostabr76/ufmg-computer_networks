#pragma once

#include <time.h>

/**
 * TODO: 2021-05-31 - ADD Descricao
 */
void clientSendNumericParam(
	const int socketFD,
	char *buffer,
	const uint32_t valueToSend,
	struct timeval *timeout,
	const char *opLabel
);

/**
 * TODO: 2021-05-31 - ADD Descricao
 * TODO: 2021-06-08 - Verificar necessidade desta funcao
 */
void clientValidateServerReceiving(const int socketFD, struct timeval *timeout, const char *opLabel);

/**
 * TODO: 2021-05-31 - ADD Descricao
 */
int clientValidateInput(int argc, char **argv);
