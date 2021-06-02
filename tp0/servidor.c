#include <stdio.h>
#include <stdlib.h>

#define SIZE_BUFFER 1024
#define SIZE_TXT_BYTES 8

#define ASCII_NUMBER_FIRST 48
#define ASCII_NUMBER_LAST 57
#define ASCII_LC_LETTER_LAST 122
#define ASCII_LC_LETTER_FIRST 97

#define DEBUG_ENABLE 1
#define DEBUG_TXT_LENGTH 150

void debugStep(const char *text) {
	if (DEBUG_ENABLE) {
		puts(text);
	}
}

void logErrorAndDie(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

void explainAndDie(char **argv) {
    printf("Invalid Input\n");
    printf("Usage: %s server port>\n", argv[0]);
	printf("Example: %s 5000\n", argv[0]);
    exit(EXIT_FAILURE);
}

int validateLowerCaseString(const char *string, const int strLength) {

	for (int i; i < strLength; i++) {
		if ((int)string[i] < ASCII_LC_LETTER_FIRST || (int)string[i] > ASCII_LC_LETTER_LAST) {
			return 0;
		}
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

int validateInput(int argc, char **argv) {

	if (argc != 2) {
        debugStep("ERROR: Invalid argc!");
		return 0;
    }

	const char *portStr = argv[1];
	if (!validateNumericString(portStr, strlen(portStr))) {
		debugStep("ERROR: Invalid Port!");
		return 0;
	}

	return 1;
}

/**
 * TODO: 2021-05-31 - ADD Descricao
 */
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

struct ClientData {
    int socket;
    struct sockaddr_storage address;
};

/**
 * TODO: 2021-06-02 - ADD Descricao
 * TODO: 2021-06-02 - Porque essa funcao tem que ter essa assinatura??
 */
void * createClientThread(void *data) {

    const int debugTextLength = DEBUG_ENABLE ? SIZE_BUFFER : 0;
	char debugTxt[debugTextLength];

    // Avalia entrada
    struct ClientData *clientData = (struct ClientData *)data;
    struct sockaddr *clientAddr = (struct sockaddr *)(&clientData->address);

    // Notifica origem da conexao
    
    /**
     * TODO: 2021-06-02 - Tratar exibicao do endereco da conexao
     */
    
    if (DEBUG_ENABLE) {
        char clientAddrStr[SIZE_BUFFER];
        // addrtostr(clientAddr, clientAddrStr, SIZE_BUFFER);
        memset(debugTxt, 0, SIZE_BUFFER)
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
	
    	if (DEBUG_ENABLE) {
            memset(debugTxt, 0, SIZE_BUFFER)
            sprintf(debugTxt, "\tReceiving %s, %d bytes: %s...\n", clientAddrStr, (int)receivedBytes, receivedText);
            debugStep(debugTxt);
        }
	}

    if (DEBUG_ENABLE) {
        memset(debugTxt, 0, SIZE_BUFFER)
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
        memset(debugTxt, 0, SIZE_BUFFER)
        sprintf(debugTxt, "\nText length is: %d\n", txtLength);
        debugStep(debugTxt);
    }

    // Texto
    debugStep("[thread] Parsing ciphered text...\n");

    char cipheredText[txtLength];
    memcpy(cipheredText, buffer + SIZE_TXT_BYTES, txtLength);

    if (!validateLowerCaseString(cipheredText)) {
        memset(debugTxt, 0, SIZE_BUFFER)
        sprintf(debugTxt, "Invalid text '%s' sent by client...", cipheredText);
        logErrorAndDie(debugTxt);
    }

    if (DEBUG_ENABLE) {
        memset(debugTxt, 0, SIZE_BUFFER)
        sprintf(debugTxt, "\tCiphered text is: \"%s\"\n", cipheredText);
        debugStep(debugTxt);
    }

    // Chave da criptografia
    debugStep("[thread] Cipher key...\n");

    const int aux = txtLength + SIZE_TXT_BYTES;
    const int cipherKeyStrLength = receivedBytesAcc - aux;
    char key[cipherKeyStrLength];
    memcpy(cipheredText, buffer + aux, cipherKeyStrLength);

    if (!validateNumericString(cipherKeyStrLength)) {
        memset(debugTxt, 0, SIZE_BUFFER)
        sprintf(debugTxt, "Invalid cipher key '%s' sent by client...", cipherKeyStrLength);
        logErrorAndDie(debugTxt);
    }

    const int cipherKey = atoi(cipherKeyStrLength);

    if (DEBUG_ENABLE) {
        memset(debugTxt, 0, SIZE_BUFFER)
        sprintf(debugTxt, "\Cipher key is: %d\n", cipherKey);
        debugStep(debugTxt);
    }

    /*=================================================== */
    /*-- Decodificar texto ------------------------------ */

    debugStep("[thread] Decrypting text...\n");

	char text[msgLength];
	memset(text, 0, msgLength);
	caesarDecipher(cipheredText, msgLength, text, txtLength);

    debugStep("\tText successfully decrypted...\n");
    puts(text)

    /*=================================================== */
    /*-- Enviar string decodificada --------------------- */

    /**
     * TODO: 2021-06-02 - Continuar daqui...
     */

    // debugStep("Sending answer to client...\n");

	// const int bytesToSend = strlen(cipherKeyStr) + 1;
	// sentBytes = send(sock, cipherKeyStr, bytesToSend, 0); // Retorna qtd de bytes transmitidos (3o argumento serve para parametrizar o envio)

	// if (sentBytes != bytesToSend) {
	// 	logErrorAndDie("Failure as sending cipher key");
	// }

	// debugStep("\tEncryption key sent...\n");


    

    

    

    










    receivedBytes = send(clientData->socket, receivedText, strlen(receivedText) + 1, 0);
    if (receivedBytes != strlen(receivedText) + 1) {
        logexit("send");
    }

    // Encerra conexao
    close(clientData->socket);
    pthread_exit(EXIT_SUCCESS);
}

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
	debugStep("Starting...\n");

    /*=================================================== */
    /*-- Validar entrada -------------------------------- */

	debugStep("Validating input...\n");
    if (!validateInput(argc, argv)) {
        explainAndDie(argv);
    }
	debugStep("\tInput is valid!\n");

    /*=================================================== */
    /*-- Receber porto ---------------------------------- */

    /*=================================================== */
    /*-- Criar Thread ----------------------------------- */

    /*=================================================== */
    /*-- Receber chave da criptografia ------------------ */

    /*=================================================== */
    /*-- Decodificar texto ------------------------------ */

    /*=================================================== */
    /*-- Escrever string decodificado ------------------- */

    /*=================================================== */
    /*-- Enviar string decodificada --------------------- */

    /*=================================================== */
    /*-- Fechar conexao --------------------------------- */

    /*=================================================== */
    /*-- Terminar execucao ------------------------------ */
}