#include "common.h"
#include "client_utils.h"

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
	debugStep("Starting...\n");

    /*=================================================== */
    /*-- Validar entrada -------------------------------- */

	debugStep("Validating input...\n");
    if (!clientValidateInput(argc, argv)) {
        explainAndDie(argv);
    }

	/*
		Define endereco do socket (ipv4 OU ipv6):

		- Struct sockaddr_storage equivale a uma 'super classe';
		- Permite alocar enderecos tanto ipv4 quanto ipv6;
		- sockaddr_in / sockaddr_in6;
	*/

	debugStep("Parsing address...\n");
	struct sockaddr_storage address;
	const char *addrStr = argv[1];
	const char *portStr = argv[2];
	if (!clientParseAddress(addrStr, portStr, &address)) { // Funcao customizada
		explainAndDie(argv);
	}
    
	/*=================================================== */
    /*-- Conectar com servidor -------------------------- */

	debugStep("Creating socket...\n");
	int sock = socket(address.ss_family, SOCK_STREAM, 0); // socket tcp (existem outros tipos)
	if (sock == -1) {
		logErrorAndDie("socket");
	}

	/*
		Cria conexao no enderenco (IP + Porta) do socket
		- Struct sockaddr equivale a uma 'interface' implementada por sockaddr_in / sockaddr_in6;
	*/

	debugStep("Creating connection...\n");

	struct sockaddr *_address = (struct sockaddr *)(&address);
	if (0 != connect(sock, _address, sizeof(address))) {
		logErrorAndDie("Failure as connecting to server");
	}
	
	sprintf(debugTxt, "\nConnected to s:%s\n", addrStr, portStr);
	debugStep(debugTxt);

	/*=================================================== */
    /*-- Enviar tamanho da string ----------------------- */

	debugStep("Sending message length...\n");
	
	const char *msg = argv[3];
	const int msgLength = strlen(msg);
	char msgLengthStr[SIZE_NUMBER_STR];
	
	memset(msgLengthStr, 0, SIZE_NUMBER_STR); // Inicializar buffer com 0
	snprintf(msgLengthStr, SIZE_NUMBER_STR, "%d", msgLength);

	int bytesToSend = strlen(msgLengthStr) + 1;
	size_t sentBytes = send(sock, msgLengthStr, bytesToSend, 0); // Retorna qtd de bytes transmitidos (3o argumento serve para parametrizar o envio)

	if (sentBytes != bytesToSend) {
		logErrorAndDie("Failure as sending msg length");
	}

	/*=================================================== */
    /*-- Enviar string cifrada -------------------------- */

	debugStep("Sending message...\n");
	const char *cipherKeyStr = argv[4];
	const int cipherKey = atoi(cipherKeyStr);
	char cipheredMsg[msgLength];
	memset(cipheredMsg, 0, msgLength); // Inicializar buffer com 0
	caesarCipher(msg, msgLength, cipheredMsg, cipherKey);

	bytesToSend = msgLength + 1;
	sentBytes = send(sock, cipheredMsg, bytesToSend, 0); // Retorna qtd de bytes transmitidos (3o argumento serve para parametrizar o envio)

	if (sentBytes != bytesToSend) {
		logErrorAndDie("Failure as sending message");
	}

    /*=================================================== */
    /*-- Enviar chave da cifra -------------------------- */

	debugStep("Sending encryption key...\n");

	bytesToSend = strlen(cipherKeyStr) + 1;
	sentBytes = send(sock, cipherKeyStr, bytesToSend, 0); // Retorna qtd de bytes transmitidos (3o argumento serve para parametrizar o envio)

	if (sentBytes != bytesToSend) {
		logErrorAndDie("Failure as sending cipher key");
	}

	/*=================================================== */
    /*-- Receber resposta (string desencriptografada) --- */

	debugStep("Waiting server answer...\n");
	
	char answer[SIZE_BUFFER];
	memset(answer, 0, SIZE_BUFFER); // Inicializar buffer com 0
	
	unsigned receivedBytesAcc = 0;
	while (1) {
		size_t receivedBytes = recv(sock, answer + receivedBytesAcc, SIZE_BUFFER - receivedBytesAcc, 0);
		if (receivedBytes == 0) { // Connection terminated
			break;
		}
		receivedBytesAcc += receivedBytes;
	}

	sprintf(debugTxt, "\tReceived %u bytes!\n", receivedBytesAcc);
	debugStep(debugTxt);

	/*=================================================== */
    /*-- Fechar conexao com o servidor ------------------ */

	debugStep("Closing socket...");
	close(sock);

	/*=================================================== */
    /*-- Imprimir resposta ------------------------------ */

	puts(answer);
	debugStep("\n-- END --\n");
	exit(EXIT_SUCCESS);
}
