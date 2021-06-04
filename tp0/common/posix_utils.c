#include "common.h"
#include "posix_utils.h"

#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>

#include <stdio.h>
#include <sys/select.h>
#include <errno.h>

enum FdActionEnum {
	FD_ACTION_RD = 10,
	FD_ACTION_WT
};

/**
 * TODO: 2021-06-04 - Ver listen_socket
 */

void posixReceive(const int socketFD, char *buffer, unsigned *recvBytesAcc, struct timeval *timeout) {
	while (1) {

		size_t receivedBytes = 0;
		if (posixIsActionAvailable(socketFD, FD_ACTION_RD, timeout))
			receivedBytes = recv(socketFD, buffer + *recvBytesAcc, BUF_SIZE - *recvBytesAcc, MSG_DONTWAIT | MSG_OOB);

		if (receivedBytes < 1) { // Connection terminated
			break;
		}
		*recvBytesAcc += receivedBytes;
	}
}

int posixSend(const int socketFD, const char *buffer, const int bytesToSend, struct timeval *timeout) {
	size_t sentBytes = 0;
	if (posixIsActionAvailable(socketFD, FD_ACTION_WT, timeout))
		sentBytes = send(socketFD, buffer, bytesToSend + 1, MSG_DONTWAIT | MSG_OOB);
	return (sentBytes >= bytesToSend) ? 1 : 0;
}

int posixAddressToString(const struct sockaddr *address, char *addressString) {
    struct sockaddr_in *addr4 = (struct sockaddr_in *)address;
    if (inet_ntop(AF_INET, &(addr4->sin_addr), addressString, INET_ADDRSTRLEN + 1)) { // network to presentation
        return 1;
    }
    return 0;
}

int posixIsActionAvailable(int socketFD, const enum FdActionEnum action, struct timeval *timeout) {

	if (!socketFD || !action)
		commonLogErrorAndDie("Invalid arguments for action availability check");

	while (1) {
		
		fd_set readFds;
		FD_ZERO(&readFds);

		fd_set writeFds;
		FD_ZERO(&writeFds);

		if (action == FD_ACTION_RD)
			FD_SET(socketFD, &readFds);
		else if (action == FD_ACTION_WT)
			FD_SET(socketFD, &writeFds);
		else
			commonLogErrorAndDie("Invalid action type for availability check");

		int isReady = select(socketFD + 1, &readFds, &writeFds, NULL, timeout);
		if (isReady > 0)
			return 1;

		if (isReady == 0) // Timeout
			return 0;

		if (errno != EINTR) // Erro: Interrupted System Call | TODO: Pq esse erro eh ignorado?
			commonLogErrorAndDie("Failure as running availability check");
	}
}