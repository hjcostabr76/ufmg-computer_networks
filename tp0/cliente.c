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

	const int debugTextLength = DEBUG_ENABLE ? 200 : 0;
	char debugTxt[debugTextLength];
	commonDebugStep("\nStarting...\n\n");

    /*=================================================== */
    /*-- Validar entrada -------------------------------- */

	commonDebugStep("Validating input...\n");
    if (!clientValidateInput(argc, argv)) {
        explainAndDie(argv);
    }

	/*
		Define endereco do socket:

		- Struct sockaddr_storage equivale a uma 'super classe';
		- Permite alocar enderecos tanto ipv4 quanto ipv6;
		- sockaddr_in / sockaddr_in6;
	*/

	commonDebugStep("Parsing address...\n");
	struct sockaddr_storage address;
	const char *addrStr = argv[1];
	const char *portStr = argv[2];
	if (!clientParseAddress(addrStr, portStr, &address)) { // Funcao customizada
		explainAndDie(argv);
	}
    
	/*=================================================== */
    /*-- Conectar com servidor -------------------------- */

	commonDebugStep("Creating socket...\n");
	int sock = socket(address.ss_family, SOCK_STREAM, 0); // socket tcp (existem outros tipos)
	if (sock == -1) {
		commonLogErrorAndDie("Failure as creating socket [1]");
	}

	// Define timeout de escuta
    commonDebugStep("Setting listening timeout...\n");

    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SECS;
    timeout.tv_usec = 0;

    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0) {
        commonLogErrorAndDie("Failure as creating socket [2]");
    }

	/*
		Cria conexao no enderenco (IP + Porta) do socket
		- Struct sockaddr equivale a uma 'interface' implementada por sockaddr_in / sockaddr_in6;
	*/

	commonDebugStep("Creating connection...\n");

	struct sockaddr *_address = (struct sockaddr *)(&address);
	if (0 != connect(sock, _address, sizeof(address))) {
		commonLogErrorAndDie("Failure as connecting to server");
	}
	
	if (DEBUG_ENABLE) {
		sprintf(debugTxt, "\nConnected to %s:%s\n", addrStr, portStr);
		commonDebugStep(debugTxt);
	}

	/*=================================================== */
    /*-- Enviar tamanho da string ----------------------- */

	char buffer[SIZE_BUFFER];
	commonDebugStep("Sending message length...\n");
	
	// Enviar
	const char *text = argv[3];
	uint32_t txtLength = htonl(strlen(text));

	memset(buffer, 0, SIZE_BUFFER);
	snprintf(buffer, SIZE_BUFFER, "%d", txtLength);
	int bytesToSend = strlen(buffer) + 2;

	if (!posixSend(sock, buffer, bytesToSend)) {
        commonLogErrorAndDie("Failure as sending text length");
    }

	// Receber retorno
	unsigned receivedBytes = 0;
	memset(buffer, 0, SIZE_BUFFER);
	posixReceive(sock, buffer, &receivedBytes);
	if (strcmp(buffer, "1") != 0) {
		commonLogErrorAndDie("Server sent failure response");
	}

	/*=================================================== */
    /*-- Enviar chave da cifra -------------------------- */

	commonDebugStep("Sending encryption key...\n");

	// Enviar
	const char *cipherKeyStr = argv[4];
	uint32_t cipherKey = htonl(atoi(cipherKeyStr));

	memset(buffer, 0, SIZE_BUFFER);
	snprintf(buffer, SIZE_BUFFER, "%d", cipherKey);
	bytesToSend = strlen(buffer) + 1;

	if (!posixSend(sock, buffer, bytesToSend)) {
        commonLogErrorAndDie("Failure as sending cipher key");
    }

	// Receber retorno
	receivedBytes = 0;
	posixReceive(sock, buffer, &receivedBytes);
	if (strcmp(buffer, "1") != 0) {
		commonLogErrorAndDie("Server sent failure response");
	}

	/*=================================================== */
    /*-- Enviar string cifrada -------------------------- */

	commonDebugStep("Sending message...\n");

	// Enviar
	memset(buffer, 0, SIZE_BUFFER); // Inicializar buffer com 0
	caesarCipher(text, txtLength, buffer, cipherKey);
	bytesToSend = strlen(buffer) + 1;

	if (!posixSend(sock, buffer, bytesToSend)) {
        commonLogErrorAndDie("Failure as sending message");
    }

	/*=================================================== */
    /*-- Receber resposta (string desencriptografada) --- */

	commonDebugStep("Waiting server answer...\n");
	
	memset(buffer, 0, SIZE_BUFFER);
	receivedBytes = 0;
	posixReceive(sock, buffer, &receivedBytes);

	if (receivedBytes < txtLength) {
		sprintf(debugTxt, "Invalid deciphered response from server: \"%.1000s\"\n", buffer);
		commonLogErrorAndDie(debugTxt);
	}

	if (DEBUG_ENABLE) {
		sprintf(debugTxt, "\tReceived %u bytes!\n", receivedBytes);
		commonDebugStep(debugTxt);
	}

	/*=================================================== */
    /*-- Imprimir resposta ------------------------------ */
	
	puts(buffer);
	close(sock);
	exit(EXIT_SUCCESS);
}
