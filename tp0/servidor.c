#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/socket.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <sys/time.h>

#define SIZE_BUFFER 1024
#define SIZE_TXT_BYTES 8

#define ASCII_NUMBER_FIRST 48
#define ASCII_NUMBER_LAST 57
#define ASCII_LC_LETTER_LAST 122
#define ASCII_LC_LETTER_FIRST 97

#define DEBUG_ENABLE 1
#define DEBUG_TXT_LENGTH 150

#define TIMEOUT_SECS 15
#define MAX_CONNECTIONS 2


void debugStep(const char *text);
void logErrorAndDie(const char *msg);
void explainAndDie(char **argv);
int validateLowerCaseString(const char *string);
int validateNumericString(const char *string);
int validateInput(int argc, char **argv);
void caesarDecipher(const char *cipheredText, const int textLength, char *text, int key);
int initServerSocket(const char *portStr, struct sockaddr_storage *address);
int addressToString(const struct sockaddr *address, char *addressString);
void *threadClientConnectionHandler(void *data);

struct ClientData {
    int socket;
    struct sockaddr_storage address;
};

/**
 * ------------------------------------------------
 * == Programa Servidor ===========================
 * ------------------------------------------------
 * 
 * TODO: 2021-06-02 - ADD Descricao
 * TODO: 2021-06-02 - Resolver todo's
 * 
 */
int main(int argc, char **argv) {

    const int debugTextLength = DEBUG_ENABLE ? DEBUG_TXT_LENGTH : 0;
	char debugTxt[debugTextLength];
	debugStep("\nStarting...\n");

    /*=================================================== */
    /*-- Validar entrada -------------------------------- */

	debugStep("Validating input...\n");
    if (!validateInput(argc, argv)) {
        explainAndDie(argv);
    }

    /*=================================================== */
    /*-- Receber porto ---------------------------------- */

    // Estabelece endereco em que o servidor vai aguardar conexoes 
    debugStep("Setting server address...\n");
    
    struct sockaddr_storage serverAddress;
    const char serverPortStr = argv[1];
    if (!initServerSocket(serverPortStr, &serverAddress)) {
        explainAndDie(argv);
    }

    /*=================================================== */
    /*-- Criar socket para receber conexoes ------------- */

    debugStep("Creating server socket...\n");

    int serverSocket;
    serverSocket = socket(serverAddress.ss_family, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        logErrorAndDie("Failure as creating server socket [1]");
    }

    // Evitar que porta utlizada numa execucao fique 02 min inativa apos sua conclusao
    debugStep("Enabling server address reusing...\n");

    int enableAddressReusing = 1;
    if (setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, &enableAddressReusing, sizeof(int)) != 0) {
        logErrorAndDie("Failure as creating server socket [2]");
    }

    // Define timeout de escuta
    debugStep("Setting server listening timeout...\n");

    struct timeval timeout;
    timeout.tv_sec = TIMEOUT_SECS;

    if (setsockopt(serverSocket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout)) != 0) {
        logErrorAndDie("Failure as creating server socket [2]");
    }

    /*=================================================== */
    /*-- Iniciar espera por conexoes -------------------- */

    debugStep("Binding server to it's address...\n");

    struct sockaddr *_address = (struct sockaddr *)(&serverAddress);
    if (bind(serverSocket, _address, sizeof(serverAddress)) != 0) {
        logErrorAndDie("Failure as biding server socket");
    }

    debugStep("Starting to listen...\n");

    if (listen(serverSocket, MAX_CONNECTIONS) != 0) {
        logErrorAndDie("Failure as starting server listening");
    }

    if (DEBUG_ENABLE) {
        char serverAddressStr[SIZE_BUFFER];
        addressToString(_address, serverAddressStr);
        memset(debugTxt, 0, SIZE_BUFFER);
        sprintf(debugTxt, "\nAll set! Server is bound to %s and waiting for connections...\n", serverAddressStr);
        debugStep(debugTxt);
    }

    while (1) {

        // Criar socket para receber conexoes de clientes
        struct sockaddr_storage clientAddress;
        struct sockaddr *_clientAddress = (struct sockaddr *)(&clientAddress);
        socklen_t clientAddressLength = sizeof(clientAddress);

        int clientSocket = accept(serverSocket, _clientAddress, &clientAddressLength);
        if (clientSocket == -1) {
            logErrorAndDie("Failure as trying to accept client connection");
        }

        // Define dados para thread de tratamento da nova conexao
        struct ClientData *clientData = malloc(sizeof(*clientData));
        if (!clientData) {
            logErrorAndDie("Failure as trying to set new client connection data");
        }

        clientData->socket = clientSocket;
        memcpy(&(clientData->address), &clientAddress, sizeof(clientAddress));

        // Inicia nova thread para tratar a nova conexao
        pthread_t tid; // Thread ID
        pthread_create(&tid, NULL, threadClientConnectionHandler, clientData);
    }

    exit(EXIT_SUCCESS);
}

int initServerSocket(const char *portStr, struct sockaddr_storage *address) {
    
    uint16_t port = (uint16_t)atoi(portStr); // unsigned short
    if (port == 0) {
        return 0;
    }
    
    port = htons(port); // host to network short
    memset(address, 0, sizeof(*address));
    
    struct sockaddr_in *addr4 = (struct sockaddr_in *)address;
    addr4->sin_family = AF_INET;
    addr4->sin_addr.s_addr = INADDR_ANY; // Significa que o bind deve ocorrer em qualquer endereco da maquiena que estiver disponivel (IPv4)
    addr4->sin_port = port;
    return 1;
}

void *threadClientConnectionHandler(void *data) {

    debugStep("\n[thread] Starting...\n");

    const int debugTextLength = DEBUG_ENABLE ? SIZE_BUFFER : 0;
	char debugTxt[debugTextLength];

    // Avalia entrada
    struct ClientData *clientData = (struct ClientData *)data;
    struct sockaddr *clientAddr = (struct sockaddr *)(&clientData->address);

    // Notifica origem da conexao
    char clientAddrStr[INET_ADDRSTRLEN + 1] = "";
    if (!addressToString(clientAddr, clientAddrStr)) {
        logErrorAndDie("Failure as parsing client address");
    }
    
    if (DEBUG_ENABLE) {
        memset(debugTxt, 0, SIZE_BUFFER);
        sprintf(debugTxt, "[thread] Connected to client at %s...\n", clientAddrStr);
        debugStep(debugTxt);
    }

    /*=================================================== */
    /*-- Receber dados do cliente ----------------------- */
    
    /*
        01: Tamanho do texto a ser decododificado - 04B;
        02: Texto cifrado;
        03: Chave da cifra;
    */

    debugStep("[thread] Reading client data...\n");

    char buffer[SIZE_BUFFER];
    memset(buffer, 0, SIZE_BUFFER);

    unsigned receivedBytesAcc = 0;
	while (1) {
		size_t receivedBytes = recv(clientData->socket, buffer + receivedBytesAcc, SIZE_BUFFER - receivedBytesAcc, 0);
		if (receivedBytes == 0) { // Connection terminated
			break;
		}
		receivedBytesAcc += receivedBytes;
	}

    if (DEBUG_ENABLE) {
        memset(debugTxt, 0, SIZE_BUFFER);
        sprintf(debugTxt, "\tReceived Text: %.1000s\n", clientAddrStr); // Limita qtd de caracteres incluidos na string
        debugStep(debugTxt);
    }

    /*=================================================== */
    /*-- Interpretar conteudo recebido do cliente ------- */

    // Comprimento do texto
    debugStep("[thread] Parsing text length...\n");

    char txtLengthStr[SIZE_TXT_BYTES];
    memcpy(txtLengthStr, buffer, SIZE_TXT_BYTES);

    if (!validateNumericString(txtLengthStr)) {
        char aux[SIZE_TXT_BYTES];
        sprintf(aux, "Invalid text length '%s' sent by client...", txtLengthStr);
        logErrorAndDie(aux);
    }

    const int txtLength = atoi(txtLengthStr);

    if (DEBUG_ENABLE) {
        memset(debugTxt, 0, SIZE_BUFFER);
        sprintf(debugTxt, "\nText length is: %d\n", txtLength);
        debugStep(debugTxt);
    }

    // Texto
    debugStep("[thread] Parsing ciphered text...\n");

    char cipheredText[txtLength];
    memcpy(cipheredText, buffer + SIZE_TXT_BYTES, txtLength);

    if (!validateLowerCaseString(cipheredText)) {
        memset(debugTxt, 0, SIZE_BUFFER);
        sprintf(debugTxt, "Invalid text '%s' sent by client...", cipheredText);
        logErrorAndDie(debugTxt);
    }

    if (DEBUG_ENABLE) {
        memset(debugTxt, 0, SIZE_BUFFER);
        sprintf(debugTxt, "\tCiphered text is: \"%s\"\n", cipheredText);
        debugStep(debugTxt);
    }

    // Chave da criptografia
    debugStep("[thread] Parsing cipher key...\n");

    const int aux = txtLength + SIZE_TXT_BYTES;
    const int cipherKeyStrLength = receivedBytesAcc - aux;
    char key[cipherKeyStrLength];
    memcpy(cipheredText, buffer + aux, cipherKeyStrLength);

    if (!validateNumericString(cipherKeyStrLength)) {
        memset(debugTxt, 0, SIZE_BUFFER);
        sprintf(debugTxt, "Invalid cipher key '%s' sent by client...", cipherKeyStrLength);
        logErrorAndDie(debugTxt);
    }

    const int cipherKey = atoi(cipherKeyStrLength);

    if (DEBUG_ENABLE) {
        memset(debugTxt, 0, SIZE_BUFFER);
        sprintf(debugTxt, "\tCipher key is: %d\n", cipherKey);
        debugStep(debugTxt);
    }

    /*=================================================== */
    /*-- Decodificar texto ------------------------------ */

    debugStep("[thread] Decrypting text...\n");

	char text[txtLength];
	memset(text, 0, txtLength);
	caesarDecipher(cipheredText, txtLength, text, txtLength);

    debugStep("\tText successfully decrypted...\n");
    puts(text);

    /*=================================================== */
    /*-- Enviar string decodificada --------------------- */

    debugStep("[thread] Sending answer to client...\n");

	const int bytesToSend = strlen(text) + 1;
	const int sentBytes = send(clientData->socket, text, bytesToSend, 0); // Retorna qtd de bytes transmitidos (3o argumento serve para parametrizar o envio)

	if (sentBytes != bytesToSend) {
		logErrorAndDie("[thread] Failure as sending answer to client");
	}

    // Encerra conexao
	debugStep("\n[thread] Done!\n");
    close(clientData->socket);
    pthread_exit(EXIT_SUCCESS);
}

int addressToString(const struct sockaddr *address, char *addressString) {
    struct sockaddr_in *addr4 = (struct sockaddr_in *)address;
    if (inet_ntop(AF_INET, &(addr4->sin_addr), addressString, INET_ADDRSTRLEN + 1)) { // network to presentation
        return 1;
    }
    return 0;
}

int validateLowerCaseString(const char *string) {

	for (int i; i < strlen(string); i++) {
		if ((int)string[i] < ASCII_LC_LETTER_FIRST || (int)string[i] > ASCII_LC_LETTER_LAST) {
			return 0;
		}
	}

	return 1;
}

int validateNumericString(const char *string) {

	for (int i; i < strlen(string); i++) {
		if ((int)string[i] < ASCII_NUMBER_FIRST || (int)string[i] > ASCII_NUMBER_LAST) {
			return 0;
		}
	}

	return 1;
}

int validateInput(int argc, char **argv) {

	if (argc != 2) {
        debugStep("ERROR: Invalid argc!");
		return 0;
    }

	const char *portStr = argv[1];
	if (!validateNumericString(portStr)) {
		debugStep("ERROR: Invalid Port!");
		return 0;
	}

	return 1;
}

void caesarDecipher(const char *cipheredText, const int textLength, char *text, int key) {

	const int range = ASCII_LC_LETTER_LAST - ASCII_LC_LETTER_FIRST;

	for (int i = 0; i < textLength; i++) {

		const int cipheredCharCode = (int)text[i];
		int charCode = cipheredCharCode - key;
		
		while (charCode < ASCII_LC_LETTER_FIRST) {
			charCode += (range + 1);
		}
		
		text[i] = (char)charCode;
	}

	text[textLength] = '\0';
}

void explainAndDie(char **argv) {
    printf("Invalid Input\n");
    printf("Usage: %s server port>\n", argv[0]);
	printf("Example: %s 5000\n", argv[0]);
    exit(EXIT_FAILURE);
}

void logErrorAndDie(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

void debugStep(const char *text) {
	if (DEBUG_ENABLE) {
		puts(text);
	}
}


/*
    Set socket FD's option OPTNAME at protocol level LEVEL
    to *OPTVAL (which is OPTLEN bytes long).
    Returns 0 on success, -1 for errors.
*/
// extern int setsockopt (int __fd, int __level, int __optname, const void *__optval, socklen_t __optlen) __THROW;

// SO_RCVTIMEO SO_SNDTIMEO

// Specify the receiving or sending timeouts until reporting an error.  The argument  is  a
// struct timeval.  If an input or output function blocks for this period of time, and data
// has been sent or received, the return value of that function will be the amount of  data
// transferred;  if  no data has been transferred and the timeout has been reached, then -1
// is returned with errno set to EAGAIN or EWOULDBLOCK,  or  EINPROGRESS  (for  connect(2))
// just  as  if  the socket was specified to be nonblocking.  If the timeout is set to zero
// (the default), then the operation will never timeout.  Timeouts  only  have  effect  for
// system  calls  that perform socket I/O (e.g., read(2), recvmsg(2), send(2), sendmsg(2));
// timeouts have no effect for select(2), poll(2), epoll_wait(2), and so on.