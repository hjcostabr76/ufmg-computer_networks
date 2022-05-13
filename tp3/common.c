#include "common.h"

#include <stdio.h>
#include <stdlib.h>

#include <sys/socket.h>
#include <arpa/inet.h>
// #include <sys/select.h>
// #include <unistd.h>

#include <string.h>
// #include <time.h>
// #include <errno.h>

// #include <pthread.h>
// #include <inttypes.h>

void comLogErrorAndDie(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

void comDebugStep(const char *text) {
	if (DEBUG_ENABLE)
		printf("%s", text);
}

int comValidateLCaseString(const char *string, const int strLength) {

	for (int i; i < strLength; i++) {
		if ((int)string[i] < ASCII_LC_LETTER_FIRST || (int)string[i] > ASCII_LC_LETTER_LAST)
			return 0;
	}

	return 1;
}

int comValidateNumericString(const char *string, const int strLength) {

	for (int i; i < strLength; i++) {
		if ((int)string[i] < ASCII_NUMBER_FIRST || (int)string[i] > ASCII_NUMBER_LAST)
			return 0;
	}

	return 1;
}

short int posixIsValidAddrFamily(const int addrFamily) {
	return (addrFamily == AF_INET || addrFamily == AF_INET6);
}

int posixListen(const int port, const struct timeval *timeout, const int maxConnections) {

	// Validate params
	if (!port)
		comLogErrorAndDie("Invalid listening socket port");

	if (!maxConnections)
		comLogErrorAndDie("Listening socket must accept at least one connection");

	// Create socket
	const int sock = socket(AF_INET6, SOCK_STREAM, 0);
    if (sock == -1)
        comLogErrorAndDie("Failure as creating listening socket [1]");

    // Avoid that port used in an execution become deactivated for 02 min after conclustion
    int enableAddrReuse = 1;
    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enableAddrReuse, sizeof(int)) != 0)
        comLogErrorAndDie("Failure as creating listening socket [2]");

	// Sets linstening timeout
    if (setsockopt(sock, SOL_SOCKET, SO_RCVTIMEO, timeout, sizeof(*timeout)) != 0)
		comLogErrorAndDie("Failure as creating listening socket [3]");

	// Enable ipv4 connections in ipv6 socket
	int no = 0;
	if (setsockopt(sock, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&no, sizeof(no)) != 0)
		comLogErrorAndDie("Failure as trying to enable ipv4 clients to ipv6 server");

	// Bind socket
	struct sockaddr_storage storage;
	memset(&storage, 0, sizeof(storage));

	struct sockaddr_in6 *addr = (struct sockaddr_in6 *)&storage;
	memset(addr, 0, sizeof(*addr));

	addr->sin6_family = AF_INET6;
	addr->sin6_addr = in6addr_any; // Set bindings to occur in any host available address
	addr->sin6_port = htons(port); // Host to network short

	if (bind(sock, (struct sockaddr *)addr, sizeof(*addr)) != 0)
		comLogErrorAndDie("Failure as biding listening socket");
    
	// Start listening
	if (listen(sock, maxConnections) != 0)
		comLogErrorAndDie("Failure as starting to listen");
	
	return sock;
}

// int posixConnect(const int port, const char *addrStr, const struct timeval *timeout) {

// 	// Valida endereco
// 	if (!port)
// 		comLogErrorAndDie("invalid connection port");

//     // Determina protocolo
// 	struct sockaddr_storage addrStorage;

//     struct in_addr inaddr4; // 32-bit IP address
// 	const int isIpv4 = inet_pton(AF_INET, addrStr, &inaddr4);

// 	struct in6_addr inaddr6; // 128-bit IPv6 address
// 	const int isIpv6 = !isIpv4 ? inet_pton(AF_INET6, addrStr, &inaddr6) : 0;

// 	if (!isIpv4 && !isIpv6)
// 		comLogErrorAndDie("Invalid address family to create connection socket");

// 	// Cria socket
// 	int socketFD = socket(isIpv4 ? AF_INET : AF_INET6, SOCK_STREAM, 0);
// 	if (socketFD == -1)
// 		comLogErrorAndDie("Failure as creating connection socket [1]");

// 	if (setsockopt(socketFD, SOL_SOCKET, SO_RCVTIMEO, timeout, sizeof(*timeout)) != 0)
// 		comLogErrorAndDie("Failure as creating connection socket [2]");

// 	// Trata ipv4
//     if (isIpv4) {
// 		struct sockaddr_in *addr4 = (struct sockaddr_in *)&addrStorage;
//         addr4->sin_family = AF_INET;
//         addr4->sin_port = htons(port);
//         addr4->sin_addr = inaddr4;

//     // Trata ipv6
//     } else {
// 		struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&addrStorage;
//         addr6->sin6_family = AF_INET6;
//         addr6->sin6_port = htons(port);
//         // addr6->sin6_addr = inaddr6
//         memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));
//     }

// 	// Cria conexao
// 	if (connect(socketFD, (struct sockaddr *)(&addrStorage), sizeof(addrStorage)) != 0)
// 		comLogErrorAndDie("Failure as creating connection socket [3]");

// 	return socketFD;
// }

char* posixGetSocketAddressString(int socket) {

	socklen_t* restrict socketLength;
	struct sockaddr *addr;
	const strLength = 200;
	char addrStr[strLength];
    
	memset(addr, 0, sizeof(struct sockaddr_in));
    memset(addrStr, 0, strLength);
    getsockname(socket, addr, socketLength);

	if (addr->sa_family == AF_INET)
		inet_ntop(AF_INET, &(((struct sockaddr_in *)addr)->sin_addr), addrStr, INET_ADDRSTRLEN + 1);
	if (addr->sa_family == AF_INET6)
		inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)addr)->sin6_addr), addrStr, INET6_ADDRSTRLEN + 1);

	return addrStr;
}
