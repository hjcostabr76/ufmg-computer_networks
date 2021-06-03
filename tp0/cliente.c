#include "common.h"
#include "client_utils.h"
#include "posix_utils.h"

#include <stdlib.h>
#include <arpa/inet.h>

/**
 * TODO: 2021-06-03 - Checar a devida localizacao disso aqui...
 */
#define SIZE_NUMBER_STR 10

void explainAndDie(char **argv) {
    printf("Invalid Input\n");
    printf("Usage: %s <server IP> <server port> <text> <encryption key number>\n", argv[0]);
    printf("\nOnly lowercase with no spaces strings are accepted as text.\n");
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
	commonDebugStep("Starting...\n");

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
		sprintf(debugTxt, "\nConnected to s:%s\n", addrStr, portStr);
		commonDebugStep(debugTxt);
	}

	/*=================================================== */
    /*-- Enviar tamanho da string ----------------------- */

	commonDebugStep("Sending message length...\n");
	
	// Enviar
	const char *msg = argv[3];
	uint32_t msgLength = strlen(msg);
	msgLength = htonl(msgLength);
	
	char msgLengthStr[SIZE_NUMBER_STR];
	memset(msgLengthStr, 0, SIZE_NUMBER_STR);
	snprintf(msgLengthStr, SIZE_NUMBER_STR, "%d", msgLength);

	int bytesToSend = strlen(msgLengthStr) + 1;
	if (!posixSend(sock, msgLengthStr, bytesToSend)) {
        commonLogErrorAndDie("Failure as sending msg length");
    }

	// Receber retorno
	char *sendingConfirmation = "0";
	unsigned receivedBytes = 0;
	posixReceive(sock, sendingConfirmation, &receivedBytes);
	if (sendingConfirmation != "1") {
		commonLogErrorAndDie("Server sent failure response");
	}

	/*=================================================== */
    /*-- Enviar chave da cifra -------------------------- */

	commonDebugStep("Sending encryption key...\n");

	// Enviar
	const char *cipherKeyStr = argv[4];
	uint32_t cipherKey = atoi(cipherKeyStr);
	cipherKey = htonl(cipherKey);

	bytesToSend = strlen(cipherKeyStr) + 1;
	if (!posixSend(sock, cipherKeyStr, bytesToSend)) {
        commonLogErrorAndDie("Failure as sending cipher key");
    }

	// Receber retorno
	sendingConfirmation = "0";
	receivedBytes = 0;
	posixReceive(sock, sendingConfirmation, &receivedBytes);
	if (sendingConfirmation != "1") {
		commonLogErrorAndDie("Server sent failure response");
	}

	/*=================================================== */
    /*-- Enviar string cifrada -------------------------- */

	commonDebugStep("Sending message...\n");

	// Enviar
	char cipheredMsg[msgLength];
	memset(cipheredMsg, 0, msgLength); // Inicializar buffer com 0
	caesarCipher(msg, msgLength, cipheredMsg, cipherKey);

	bytesToSend = msgLength + 1;
	if (!posixSend(sock, cipheredMsg, bytesToSend)) {
        commonLogErrorAndDie("Failure as sending message");
    }

	/*=================================================== */
    /*-- Receber resposta (string desencriptografada) --- */

	commonDebugStep("Waiting server answer...\n");
	
	char answer[SIZE_BUFFER];
	memset(answer, 0, SIZE_BUFFER); // Inicializar buffer com 0

	receivedBytes = 0;
	posixReceive(sock, answer, &receivedBytes);
	if (receivedBytes != SIZE_BUFFER) {
		sprintf(debugTxt, "Invalid deciphered response from server: \"%.1000s\"\n", answer);
		commonLogErrorAndDie(debugTxt);
	}

	if (DEBUG_ENABLE) {
		sprintf(debugTxt, "\tReceived %u bytes!\n", receivedBytes);
		commonDebugStep(debugTxt);
	}

	/*=================================================== */
    /*-- Imprimir resposta ------------------------------ */
	
	puts(answer);

	close(sock);
	exit(EXIT_SUCCESS);
}
