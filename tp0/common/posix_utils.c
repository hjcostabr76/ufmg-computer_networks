#include "common.h"
#include "posix_utils.h"

#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/select.h>
#include <errno.h>

#include <unistd.h>
#include <stdio.h>

// #include<fcntl.h>

enum FdActionEnum { FD_ACTION_RD = 10, FD_ACTION_WT };

/**
 * NOTE: Funcao 'privada'
 */
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
		if (isReady > 0) {
			if ((action == FD_ACTION_RD && FD_ISSET(socketFD, &readFds)) || (action == FD_ACTION_WT && FD_ISSET(socketFD, &writeFds)))
				return 1;
			commonLogErrorAndDie("Availability check: Something wrong isn't right... D:");
		}

		if (isReady == 0) // Timeout
			return 0;

		if (errno != EINTR) // Erro: Interrupted System Call | TODO: Pq esse erro eh ignorado?
			commonLogErrorAndDie("Failure as running availability check");
	}
}

int posixListen(const int port, const struct timeval *timeout, const int maxConnections, char *boundAddrStr) {

	// Validar entrada
	if (!port)
		commonLogErrorAndDie("Invalid listening socket port");

	if (!maxConnections)
		commonLogErrorAndDie("Listening socket must accept at least one connection");

	// Criar socket
	int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if (socketFD == -1)
        commonLogErrorAndDie("Failure as creating listening socket [1]");

    // Evitar que porta utlizada numa execucao fique 02 min inativa apos sua conclusao
    int enableAddrReuse = 1;
    if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &enableAddrReuse, sizeof(int)) != 0)
        commonLogErrorAndDie("Failure as creating listening socket [2]");

	// Define timeout de escuta
    if (setsockopt(socketFD, SOL_SOCKET, SO_RCVTIMEO, timeout, sizeof(*timeout)) != 0) {
        commonLogErrorAndDie("Failure as creating listening socket [3]");
	}

	// Iniciar escuta
	struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY; // Bind ocorre em qualquer endereco disponivel na maquina
    addr.sin_port = htons(port); // Host to network short

    if (bind(socketFD, (struct sockaddr *)(&addr), sizeof(addr)) != 0) {
        commonLogErrorAndDie("Failure as biding listening socket");
	}
    
	if (listen(socketFD, maxConnections) != 0) {
        commonLogErrorAndDie("Failure as starting to listen");
	}

	posixAddressToString((struct sockaddr *)&addr, boundAddrStr);
	return socketFD;
}

int posixConnect(const int port, const char *addrStr, const struct timeval *timeout) {

	// Valida endereco
	if (!port)
		commonLogErrorAndDie("invalid connection port");

	struct in_addr addrNumber; // Esse struct contem apenas 01 numero mesmo
	int isIpv4 = inet_pton(AF_INET, addrStr, &addrNumber);
	if (!isIpv4)
		commonLogErrorAndDie("invalid connection address");

	// Cria socket
	int socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if (socketFD == -1)
		commonLogErrorAndDie("Failure as creating connection socket [1]");

	if (setsockopt(socketFD, SOL_SOCKET, SO_RCVTIMEO, timeout, sizeof(*timeout)) != 0)
		commonLogErrorAndDie("Failure as creating connection socket [2]");

	// Cria conexao
	struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    addr.sin_addr = addrNumber;

	if (connect(socketFD, (struct sockaddr *)&addr, sizeof(addr)) != 0)
		commonLogErrorAndDie("Failure as trying to connect");

	return socketFD;
}

ssize_t posixRecv(const int socketFD, char *buffer, struct timeval *timeout) {

	size_t acc = 0;

	while (1) {
		
		ssize_t recvReturn = 0;
		const short int haveExpired = posixIsActionAvailable(socketFD, FD_ACTION_RD, timeout);

		recvReturn = recv(socketFD, buffer + acc, BUF_SIZE - acc, MSG_DONTWAIT);
		if (recvReturn == 0)
			break;

		if (recvReturn == -1)
			return (errno == EAGAIN || errno == EWOULDBLOCK) ? acc : -1;

		acc += recvReturn;
		if (haveExpired)
			break;
	}

	return acc;
}

short int posixSend(const int socketFD, const char *buffer, const unsigned bytesToSend, struct timeval *timeout) {

	ssize_t acc = 0;

	while (1) {

		ssize_t sentBytes = send(socketFD, buffer + acc, bytesToSend - acc, 0/* MSG_DONTWAIT */);
		if (sentBytes == -1)
			return 0;

		acc += sentBytes;
		if (sentBytes == 0)
			break;
	}

	return (acc >= bytesToSend) ? 1 : 0;
}

short int posixAddressToString(const struct sockaddr *addr, char *addrStr) {
    struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
    return inet_ntop(AF_INET, &(addr4->sin_addr), addrStr, INET_ADDRSTRLEN + 1) ? 1 : 0;
}
