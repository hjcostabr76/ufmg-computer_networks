#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <math.h>

#define SIZE_BUFFER 1024
#define SIZE_NUMBER_STR 10

#define ASCII_NUMBER_FIRST 48
#define ASCII_NUMBER_LAST 57
#define ASCII_LC_LETTER_LAST 122
#define ASCII_LC_LETTER_FIRST 97

#define DEBUG_ENABLE 1
#define DEBUG_TXT_LENGTH 150


/**
 * - Recebe struct ainda a ser preenchida com dados de ipv4 ou ipv6;
 * - O tipo sockaddr_storage eh 'super classe' de sockaddr_in & sockaddr_in6;
 * - Ao analisea a string de numero de endereco identifica se eh v4 ou v6;
 * - Especializa & preenche dados do endereco de acordo com a versao apropriada;
 * 
 * NOTE: inet_pton: internet presentation to network
 * NOTE: inet_ntop: internet network to presentation
 * NOTE: AF: Address Family
 * 
 * @param addr_number_str: String com numero do endereco ipv4 ou ipv6;
 * @param portstr: String com numero da porta;
 * @param addr: Struct generica a ser preenchida com dados de conexao ipv4 OU ipv6;
 */
int parseAddress(const char *addrstr, const char *portstr, struct sockaddr_storage *addr);

/**
 * TODO: 2021-05-31 - ADD Descricao
 */
void caesarCipher(const char *text, const int textLength, char *cipheredText, int key);

/**
 * TODO: 2021-05-31 - ADD Descricao
 */
void explainAndDie(char **argv);

/**
 * TODO: 2021-05-31 - ADD Descricao
 */
void logErrorAndDie(const char *msg);

/**
 * TODO: 2021-05-31 - ADD Descricao
 */
void debugStep(const char *text);

/**
 * TODO: 2021-05-31 - ADD Descricao
 */
int validateInput(int argc, char **argv);

/**
 * TODO: 2021-05-31 - ADD Descricao
 */
int validateLowerCaseString(const char *string, const int strLength);

/**
 * TODO: 2021-05-31 - ADD Descricao
 */
int validateNumericString(const char *string, const int strLength);

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

	const int debugTextLength = DEBUG_ENABLE ? DEBUG_TXT_LENGTH : 0;
	char debugTxt[debugTextLength];
	debugStep("Starting...\n");

    /*=================================================== */
    /*-- Validar entrada -------------------------------- */

	debugStep("Validating input...\n");
    if (!validateInput(argc, argv)) {
        explainAndDie(argv);
    }
	debugStep("\tInput is valid!\n");

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
	if (!parseAddress(addrStr, portStr, &address)) { // Funcao customizada
		explainAndDie(argv);
	}
	debugStep("\tAddress is fine!\n");
    
	/*=================================================== */
    /*-- Conectar com servidor -------------------------- */

	debugStep("Creating socket...\n");
	int sock = socket(address.ss_family, SOCK_STREAM, 0); // socket tcp (existem outros tipos)
	if (sock == -1) {
		logErrorAndDie("socket");
	}
	debugStep("\tSocket successfully creating...\n");

	/*
		Cria conexao no enderenco (IP + Porta) do socket
		- Struct sockaddr equivale a uma 'interface' implementada por sockaddr_in / sockaddr_in6;
	*/

	debugStep("Creating connection...\n");
	struct sockaddr *_address = (struct sockaddr *)(&address);
	if (0 != connect(sock, _address, sizeof(address))) {
		logErrorAndDie("Failure as connecting to server");
	}
	int ipVersion = address.ss_family == AF_INET ? 4 : 6;
	sprintf(debugTxt, "Connected to address IPv%d %s %s", ipVersion, addrStr, portStr);
	debugStep(debugTxt);

	/*=================================================== */
    /*-- Enviar tamanho da string ----------------------- */

	debugStep("Sending message length...\n");
	
	const char *msg = argv[3];
	const int msgLength = strlen(msg);
	printf("msg: %d / %s\n", msgLength, msg);
	char msgLengthStr[SIZE_NUMBER_STR];
	
	memset(msgLengthStr, 0, SIZE_NUMBER_STR); // Inicializar buffer com 0
	snprintf(msgLengthStr, SIZE_NUMBER_STR, "%d", msgLength);

	int bytesToSend = strlen(msgLengthStr) + 1;
	size_t sentBytes = send(sock, msgLengthStr, bytesToSend, 0); // Retorna qtd de bytes transmitidos (3o argumento serve para parametrizar o envio)

	if (sentBytes != bytesToSend) {
		logErrorAndDie("Failure as sending msg length");
	}

	debugStep("\tMessage length sent...\n");

	/*=================================================== */
    /*-- Enviar string cifrada -------------------------- */

	debugStep("Sending message...\n");
	const char *cipherKeyStr = argv[4];
	const int cipherKey = atoi(cipherKeyStr);
	char cipheredMsg[msgLength];
	memset(cipheredMsg, 0, msgLength); // Inicializar buffer com 0
	printf("\nchico: %d / %s\n", (int)strlen(cipheredMsg), cipheredMsg);
	caesarCipher(msg, msgLength, cipheredMsg, cipherKey);

	printf("ciphered: %s\n", cipheredMsg);

	bytesToSend = msgLength + 1;
	sentBytes = send(sock, cipheredMsg, bytesToSend, 0); // Retorna qtd de bytes transmitidos (3o argumento serve para parametrizar o envio)

	if (sentBytes != bytesToSend) {
		logErrorAndDie("Failure as sending message");
	}
	
	debugStep("\tMessage sent...\n");

    /*=================================================== */
    /*-- Enviar chave da cifra -------------------------- */

	debugStep("Sending encryption key...\n");

	bytesToSend = strlen(cipherKeyStr) + 1;
	sentBytes = send(sock, cipherKeyStr, bytesToSend, 0); // Retorna qtd de bytes transmitidos (3o argumento serve para parametrizar o envio)

	if (sentBytes != bytesToSend) {
		logErrorAndDie("Failure as sending cipher key");
	}

	debugStep("\tEncryption key sent...\n");

	/*=================================================== */
    /*-- Aguardar resposta (string desencriptografada) -- */

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
		debugStep("\treceiving data from server...");
	}

	sprintf(debugTxt, "\tReceived %u bytes!\n", receivedBytesAcc);
	debugStep(debugTxt);

	/*=================================================== */
    /*-- Fechar conexao com o servidor ------------------ */

	debugStep("Closing socket...");
	close(sock);
	debugStep("\tSocket closed.");

	/*=================================================== */
    /*-- Imprimir resposta ------------------------------ */

	puts(answer);
	debugStep("\n-- END --\n");
	exit(EXIT_SUCCESS);
}

/**
 * NOTE: Soh trata ipv4
 */
int parseAddress(const char *addrStr, const char *portstr, struct sockaddr_storage *addr) {
    
    // Valida parametros
    if (addrStr == NULL || portstr == NULL) {
        return 0;
    }

    // Converte string da porta para numero da porta (em network byte order)
    uint16_t port = (uint16_t)atoi(portstr); // unsigned short
    if (port == 0) {
        return 0;
    }
    port = htons(port); // host to network short

    // Trata ipv4
    struct in_addr addrNumber4;
	int is_ipv4 = inet_pton(AF_INET, addrStr, &addrNumber4);
	if (!is_ipv4) {
		return 0
	}

    struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
    addr4->sin_family = AF_INET;
    addr4->sin_port = port;
    addr4->sin_addr = addrNumber4;

    return 1;
}

void logErrorAndDie(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

void explainAndDie(char **argv) {
    printf("Invalid Input\n");
    printf("Usage: %s <server IP> <server port> <text> <encryption key number>\n", argv[0]);
    printf("\nOnly lowercase with no spaces strings are accepted as text.\n");
	printf("Example: %s 127.0.0.1 5000 lorenipsumdolur 4\n", argv[0]);
    exit(EXIT_FAILURE);
}

/**
 * TODO: 2021-05-31 - ADD Descricao
 */
void caesarCipher(const char *text, const int textLength, char *cipheredText, int key) {

	const int range = ASCII_LC_LETTER_LAST - ASCII_LC_LETTER_FIRST;

	for (int i = 0; i < textLength; i++) {

		const int currentCharCode = (int)text[i];
		int cipheredCharCode = currentCharCode + key;
		
		while (cipheredCharCode > ASCII_LC_LETTER_LAST) {
			cipheredCharCode -= (range + 1);
		}
		
		cipheredText[i] = (char)cipheredCharCode;
	}

	cipheredText[textLength] = '\0';
}


void debugStep(const char *text) {
	if (DEBUG_ENABLE) {
		puts(text);
	}
}

int validateInput(int argc, char **argv) {

	if (argc != 5) {
        debugStep("ERROR: Invalid argc!");
		return 0;
    }

	// const char *addrStr = argv[1];

	const char *portStr = argv[2];
	if (!validateNumericString(portStr, strlen(portStr))) {
		debugStep("ERROR: Invalid Port!");
		return 0;
	}

	const char *msg = argv[3];
	if (!validateLowerCaseString(msg, strlen(msg))) {
		debugStep("ERROR: Invalid Text!");
		return 0;
	}

	const char *cipherKeyStr = argv[4];
	if (!validateNumericString(cipherKeyStr, strlen(cipherKeyStr))) {
		debugStep("ERROR: Invalid Cipher Key!");
		return 0;
	}

	return 1;
}

int validateNumericString(const char *string, const int strLength) {

	for (int i; i < strLength; i++) {
		if ((int)string[i] < ASCII_NUMBER_FIRST || (int)string[i] > ASCII_NUMBER_LAST) {
			return 0;
		}
	}

	return 1;
}


int validateLowerCaseString(const char *string, const int strLength) {

	for (int i; i < strLength; i++) {
		if ((int)string[i] < ASCII_LC_LETTER_FIRST || (int)string[i] > ASCII_LC_LETTER_LAST) {
			return 0;
		}
	}

	return 1;
}