#include "string_utils.h"
#include "posix_utils.h"
#include "common.h"

#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define RCV_VALIDATION_NUMERIC 1
#define RCV_VALIDATION_LCASE 2

#define RCV_ERR_BYTES_COUNT -1
#define RCV_ERR_VALIDATION_PARAM -2
#define RCV_ERR_VALIDATION_STR -3
#define RCV_ERR_SUCCESS_RETURN -4

#define RCV_SUCCESS 1

int serverInitSocket(const char *portStr, struct sockaddr_storage *address) {
    
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

int serverReceiveParam(const int sock, char *buffer, const unsigned bytesToReceive, const int validationType) {

    // Recebe valor do cliente
    unsigned receivedBytes = 0;
    posixReceive(sock, buffer, &receivedBytes);

    // Validar: Contagem de butes
    if (receivedBytes != bytesToReceive) {
        return RCV_ERR_BYTES_COUNT;
    }

    // Validar: Conteudo
    if (validationType != RCV_VALIDATION_NUMERIC && validationType != RCV_VALIDATION_LCASE) {
        return RCV_ERR_VALIDATION_PARAM;
    }

    const int bufferLength = sizeof(buffer);

    if (validationType == RCV_VALIDATION_NUMERIC) {
        if (!stringValidateNumericString(buffer, bufferLength)) {
            return RCV_ERR_VALIDATION_STR;
        }

    } else if (!stringValidateLCaseString(buffer, bufferLength)) {
        return RCV_ERR_VALIDATION_STR;
    }
    
    // Notifica sucesso no recebimento
    if (!posixSend(sock, "1", 1)) {
        return RCV_ERR_SUCCESS_RETURN;
    }
    
    return RCV_SUCCESS;
}

void serverSendFailureResponse(const int sock, char *errMsg) {
	posixSend(sock, "0", 1);
	close(sock);
    pthread_exit(EXIT_FAILURE);
	logErrorAndDie(errMsg);
}

int serverValidateInput(int argc, char **argv) {

	if (argc != 2) {
        debugStep("ERROR: Invalid argc!");
		return 0;
    }

	const char *portStr = argv[1];
	if (!stringValidateNumericString(portStr, strlen(portStr))) {
		debugStep("ERROR: Invalid Port!");
		return 0;
	}

	return 1;
}
