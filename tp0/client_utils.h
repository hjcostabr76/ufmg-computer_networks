#pragma once

#include <time.h>

/**
 * TODO: 2021-05-31 - ADD Descricao
 */
void clientSendNumericParam(
	const int socketFD,
	char *buffer,
	const int valueToSend,
	struct timeval *timeout,
	const int opNum
);

/**
 * TODO: 2021-05-31 - ADD Descricao
 */
int clientValidateInput(int argc, char **argv);
