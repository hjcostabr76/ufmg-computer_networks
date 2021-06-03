#include "common.h"

#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
void posixReceive(const int sock, char *buffer, unsigned *receivedBytesAcc) {
	while (1) {
		size_t receivedBytes = recv(sock, buffer + *receivedBytesAcc, SIZE_BUFFER - *receivedBytesAcc, 0);
		if (receivedBytes == 0) { // Connection terminated
			break;
		}
		*receivedBytesAcc += receivedBytes;
	}
}

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
int posixSend(const int sock, char *buffer, const int bytesToSend) {
	size_t sentBytes = send(sock, buffer, bytesToSend, 0); // Retorna qtd de bytes transmitidos (3o argumento serve para parametrizar o envio)
	return (sentBytes == bytesToSend) ? 1 : 0;
}

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
int addressToString(const struct sockaddr *address, char *addressString) {
    struct sockaddr_in *addr4 = (struct sockaddr_in *)address;
    if (inet_ntop(AF_INET, &(addr4->sin_addr), addressString, INET_ADDRSTRLEN + 1)) { // network to presentation
        return 1;
    }
    return 0;
}