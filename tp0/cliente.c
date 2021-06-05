#include "common/common.h"
#include "common/posix_utils.h"
#include "common/caesar_cipher.h"
#include "client_utils.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/time.h>

void explainAndDie(char **argv) {
    printf("\nInvalid Input\n");
    printf("Usage: %s <server IP> <server port> <text> <encryption key number>\n", argv[0]);
    printf("Only lowercase with no spaces strings are accepted as text!\n");
	printf("Example: %s 127.0.0.1 5000 lorenipsumdolur 4\n", argv[0]);
    exit(EXIT_FAILURE);
}

/**
 * ------------------------------------------------
 * == Programa CLIENTE ============================
 * ------------------------------------------------
 * 
 * TODO: 2021-05-27 - ADD Descricao
 * TODO: 2021-05-27 - Resolver todo's
 * 
 */
int main(int argc, char **argv) {

	const int dbgTxtLen = DEBUG_ENABLE ? 200 : 0;
	char dbgTxt[dbgTxtLen];
	commonDebugStep("\nStarting...\n\n");

    // Validar entrada
	commonDebugStep("Validating input...\n");
    if (!clientValidateInput(argc, argv)) {
        explainAndDie(argv);
    }

	// Conectar com servidor
	commonDebugStep("Creating socket...\n");

	struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SECS;
    timeout.tv_usec = 0;

	const char *addrStr = argv[1];
	const char *portStr = argv[2];
	int socketFD = posixConnect(atoi(portStr), addrStr, &timeout);
	
	if (DEBUG_ENABLE) {
		sprintf(dbgTxt, "\nConnected to %s:%s\n", addrStr, portStr);
		commonDebugStep(dbgTxt);
	}

	// Enviar tamanho da string
	char buffer[BUF_SIZE];
	
	commonDebugStep("Sending text length...\n");
	const char *text = argv[3];
	uint32_t txtLen = htonl(strlen(text));
	clientSendNumericParam(socketFD, buffer, txtLen, &timeout, "text length");

	// Enviar chave da cifra
	commonDebugStep("Sending encryption key...\n");
	const char *cipherKeyStr = argv[4];
	uint32_t cipherKey = htonl(atoi(cipherKeyStr));
	clientSendNumericParam(socketFD, buffer, cipherKey, &timeout, "encryption key");

	// Enviar string cifrada
	commonDebugStep("Sending ciphered text...\n");
	memset(buffer, 0, BUF_SIZE);
	caesarCipher(text, txtLen, buffer, cipherKey);

	unsigned bytesToSend = strlen(buffer) + 1;
	if (!posixSend(socketFD, buffer, bytesToSend, &timeout))
		commonLogErrorAndDie("Failure as sending ciphered text");

	// Receber resposta (string desencriptografada)
	commonDebugStep("Waiting server answer...\n");
	
	memset(buffer, 0, BUF_SIZE);
	size_t receivedBytes = posixRecv(socketFD, buffer, &timeout);
	if (receivedBytes < txtLen) {
		sprintf(dbgTxt, "Invalid deciphered response from server: \"%.1000s\"\n", buffer);
		commonLogErrorAndDie(dbgTxt);
	}

	if (DEBUG_ENABLE) {
		sprintf(dbgTxt, "\tReceived %lu bytes!\n", receivedBytes);
		commonDebugStep(dbgTxt);
	}

	// Imprimir resposta
	puts(buffer);
	close(socketFD);
	exit(EXIT_SUCCESS);
}
