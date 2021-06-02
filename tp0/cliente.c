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

#define ASCII_LC_LETTER_LAST 122
#define ASCII_LC_LETTER_FIRST 97


/**
 * TODO: 2021-05-31 - ADD Descricao
 * TODO: 2021-05-31 - Implementar
 */
int validateInput(int argc, char **argv);

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
void caesarCipher(const char *originalText, char **cipheredText, int key);

/**
 * TODO: 2021-05-31 - ADD Descricao
 */
void explainAndDie(int argc, char **argv);
void logErrorAndDie(const char *msg);

/**
 * ------------------------------------------------
 * == Programa CLIENTE ============================
 * ------------------------------------------------
 * 
 * TODO: 2021-05-27 - ADD Descricao
 * TODO: 2021-05-27 - Resolver todo's
 * 
 * O que fazer:
 * - ler IP do servidor;
 * - ler porta do servidor;
 * - ler string NAO criptografada;
 * - ler indice da cifra de cezar;
 * - Conectar com servidor;
 * - Enviar tamanho da string;
 * - Enviar string cifrada;
 * - Enviar indice da cifra;
 * - Aguardar resposta (string desencriptografada);
 * - imprimir resposta;
 * - fechar conexao com o servidor
 * 
 */
int main(int argc, char **argv) {

    /*== Ler IP do servidor ============================= */
    /*== Ler porta do servidor ========================== */
    /*== Ler string NAO criptografada =================== */
    /*== Ler indice da cifra de cezar =================== */

	// Validar entrada
	if (!validateInput(argc, argv)) {
    // if (argc != 5) {
        explainAndDie(argc, argv);
    }

	/*
		Define endereco do socket (ipv4 OU ipv6):

		- Struct sockaddr_storage equivale a uma 'super classe';
		- Permite alocar enderecos tanto ipv4 quanto ipv6;
		- sockaddr_in / sockaddr_in6;
	*/

	struct sockaddr_storage address;
	const char *addrStr = argv[1];
	const char *portStr = argv[2];

	if (parse_address(addrStr, portStr, &address) != 0) { // Funcao customizada
		explain_and_die(argc, argv);
	}
    
    /*== Conectar com servidor ========================== */

	// Cria & conecta socket
	int sock = socket(address.ss_family, SOCK_STREAM, 0); // socket tcp (existem outros tipos)
	if (sock == -1) {
		logErrorAndDie("socket");
	}

	/*
		Cria conexao no enderenco (IP + Porta) do socket
		- Struct sockaddr equivale a uma 'interface' implementada por sockaddr_in / sockaddr_in6;
	*/

	struct sockaddr *_address = (struct sockaddr *)(&address);
	if (0 != connect(sock, _address, sizeof(address))) {
		logErrorAndDie("Failure as connecting to server");
	}

	int ipVersion = address.ss_family == AF_INET ? 4 : 6;
	printf("Connected to address IPv%d %s %hu", ipVersion, addrStr, portStr);

    /*== Enviar tamanho da string ======================= */

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

    /*== Enviar string cifrada ===========================*/

	const int cipherKey = argv[4];
	const char cipheredMsg[msgLength];
	memset(cipheredMsg, 0, msgLength); // Inicializar buffer com 0
	caesarCipher(msg, &cipheredMsg, cipherKey);

	bytesToSend = msgLength + 1;
	sentBytes = send(sock, cipheredMsg, bytesToSend, 0); // Retorna qtd de bytes transmitidos (3o argumento serve para parametrizar o envio)

	if (sentBytes != bytesToSend) {
		logErrorAndDie("Failure as sending message");
	}

    /*== Enviar chave da cifra ========================== */

	char cipherKeyStr[SIZE_NUMBER_STR];
	memset(cipherKeyStr, 0, SIZE_NUMBER_STR);
	snprintf(cipherKeyStr, SIZE_NUMBER_STR, "%d", cipherKey);

	bytesToSend = strlen(cipherKeyStr) + 1;
	sentBytes = send(sock, cipherKeyStr, bytesToSend, 0); // Retorna qtd de bytes transmitidos (3o argumento serve para parametrizar o envio)

	if (sentBytes != bytesToSend) {
		logErrorAndDie("Failure as sending cipher key");
	}

    /*== Aguardar resposta (string desencriptografada) == */
	
	char answer[SIZE_BUFFER];
	memset(answer, 0, SIZE_BUFFER); // Inicializar buffer com 0
	
	unsigned receivedBytesAcc = 0;
	while (1) {
		const int receivedBytes = recv(sock, answer, SIZE_BUFFER - receivedBytesAcc, 0);
		if (receivedBytes == 0) { // Connection terminated
			break;
		}
		receivedBytesAcc += receivedBytes;
	}

    /*== Fechar conexao com o servidor ================== */

	close(sock);

    /*== Imprimir resposta ============================== */
	
	printf("Received %u bytes\n", receivedBytesAcc);
	puts(answer);
	exit(EXIT_SUCCESS);
}

int parseAddress(const char *addrStr, const char *portstr, struct sockaddr_storage *addr) {
    
    // Valida parametros
    if (addrStr == NULL || portstr == NULL) {
        return -1;
    }

    // Converte string da porta para numero da porta (em network byte order)
    uint16_t port = (uint16_t)atoi(portstr); // unsigned short
    if (port == 0) {
        return -1;
    }
    port = htons(port); // host to network short

    // Trata ipv4
    struct in_addr addrNumber4;
	int is_ipv4 = inet_pton(AF_INET, addrStr, &addrNumber4);

    if (is_ipv4) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = addrNumber4;
        return 0;
    }

    // Trata ipv6
    struct in6_addr addrNumber6;
	int is_ipv6 = inet_pton(AF_INET6, addrStr, &addrNumber6);

    if (is_ipv6) {
        
		struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;

		/*
			- Necessario copiar o conteudo de memorio 'na marra';
			- Atribuicao simples nao funciona aqui do mesmo jeito que para o ipv4;
			- Para o ipv4 o numero de endereco eh um numero (32b);
			- Para o ipv6 o numero de endereco eh um vetor (128b);
			- Como NAO existe atribuicao de vetor, eh necessario fazer a copia direta de memoria;
		*/

        // addr6->sin6_addr = addr_number6
        memcpy(&(addr6->sin6_addr), &addrNumber6, sizeof(addrNumber6));
        
		return 0;
    }

    return -1;
}

/**
 * TODO: 2021-05-27 - Implementar
 * TODO: 2021-05-27 - Renomear essa funcao
 */
void logErrorAndDie(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

/**
 * TODO: 2021-05-27 - Implementar
 * TODO: 2021-05-27 - Renomear essa funcao
 */
void explainAndDie(int argc, char **argv) {
    // printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    // printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

/**
 * TODO: 2021-05-31 - ADD Descricao
 */
void caesarCipher(const char *originalText, char **cipheredText, int key) {

	const int limit = ASCII_LC_LETTER_LAST - ASCII_LC_LETTER_FIRST;

	for (int i = 0; i < strlen(originalText); i++) {
		const currentCharCode = (int)originalText[i];
		const int aux = currentCharCode + key;
		const int page = floor(aux / limit);
		const char cipheredCharCode = aux - (page * limit);
		const char cipheredChar = (char)cipheredCharCode;
		cipheredText[i] = cipheredChar;
	}
}
