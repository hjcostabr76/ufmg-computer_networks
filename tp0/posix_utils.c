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

		int selReturn = select(socketFD + 1, &readFds, &writeFds, NULL, timeout);
		
		if (selReturn == 0) // Timeout
			return 0;	
		
		if (selReturn == -1) {
			if (errno != EINTR) // Erro: Interrupted System Call | TODO: Pq esse erro eh ignorado?
				commonLogErrorAndDie("Failure as running availability check");
			continue;
		}
		
		if (selReturn != 1
			|| (action == FD_ACTION_RD && FD_ISSET(socketFD, &readFds) == 0)
			|| (action == FD_ACTION_WT && FD_ISSET(socketFD, &writeFds) == 0)
		)
			commonLogErrorAndDie("Availability check: Something wrong isn't right... D:");
		
		return 1;
	}
}

/**
 * Analisa 01 string contendo 01 endereco IP & retorna sua correspondente familia (ipv4 / ipv6), se  for valido.
 * Retorna -1, caso contrario.
 *
 * NOTE: Funcao 'privada'
 * 
 * TODO: 2021-06-08 - Verificar se essa funcao devera mesmo existir
 */
// int getAddrIPVersion(struct sockaddr_storage *addr, const char *addrStr) {

// 	int isIpv4 = inet_pton(AF_INET, addrStr, &addr);
// 	if (isIpv4)
// 		return AF_INET;

// 	int isIpv6 = inet_pton(AF_INET6, addrStr, &addr);
// 	if (isIpv6)
// 		return AF_INET6;

// 	return -1;
// }

/**
 * Conclui procedimento iniciado pela funcao {posixListen}:
 * - Operacos 'bind' + 'listen';
 * - Transcricao do endereco da conexao em string;
 * 
 * NOTE: Funcao 'privada'
 * 
 * TODO: 2021-06-08 - Verificar se essa funcao devera mesmo existir
 */
// void posixListenIpv4(const int socketFD, const int port, const struct sockaddr_storage *storage, const int maxConnections, char *boundAddrStr) {

// 	struct sockaddr_in *addr = (struct sockaddr_in *)storage;
// 	memset(addr, 0, sizeof(struct sockaddr_in));

// 	addr->sin_family = AF_INET;
// 	addr->sin_addr.s_addr = INADDR_ANY; // Significa que o bind deve ocorrer em qualquer endereco da maquiena que estiver disponivel (IPv4)
// 	addr->sin_port = htons(port); // Host to network short

// 	if (bind(socketFD, (struct sockaddr *)addr, sizeof(*addr)) != 0)
// 		commonLogErrorAndDie("Failure as biding listening socket [ipv4]");
    
// 	if (listen(socketFD, maxConnections) != 0)
// 		commonLogErrorAndDie("Failure as starting to listen [ipv4]");

// 	posixAddressToString((struct sockaddr *)addr, boundAddrStr);
// }

/**
 * Conclui procedimento iniciado pela funcao {posixListen}:
 * - Operacos 'bind' + 'listen';
 * - Transcricao do endereco da conexao em string;
 * 
 * NOTE: Funcao 'privada'
 * 
 * TODO: 2021-06-08 - Verificar se essa funcao devera mesmo existir
 */
// void posixListenIpv6(const int socketFD, const int port, const struct sockaddr_storage *storage, const int maxConnections, char *boundAddrStr) {

// 	struct sockaddr_in6 *addr = (struct sockaddr_in6 *)storage;
// 	memset(addr, 0, sizeof(struct sockaddr_in6));

// 	addr->sin6_family = AF_INET6;
// 	addr->sin6_addr = in6addr_any; // Significa que o bind deve ocorrer em qualquer endereco da maquina que estiver disponivel (IPv6)
// 	addr->sin6_port = htons(port); // Host to network short

// 	if (bind(socketFD, (struct sockaddr *)addr, sizeof(*addr)) != 0)
// 		commonLogErrorAndDie("Failure as biding listening socket [ipv6]");
    
// 	if (listen(socketFD, maxConnections) != 0)
// 		commonLogErrorAndDie("Failure as starting to listen [ipv6]");

// 	posixAddressToString((struct sockaddr *)addr, boundAddrStr);
// }

int posixListen(const int port, const int addrFamily, const struct timeval *timeout, const int maxConnections, char *boundAddrStr) {

	// Validar entrada
	if (!port)
		commonLogErrorAndDie("Invalid listening socket port");

	if (!maxConnections)
		commonLogErrorAndDie("Listening socket must accept at least one connection");

	if (!posixIsValidAddrFamily(addrFamily))
		commonLogErrorAndDie("Invalid address family");

	/**
	 * TODO: 2021-06-08 - Corrigir este trecho
	 */

	// Conclui distinguindo por tipo de IP
	// struct sockaddr_storage storage;
	// memset(&storage, 0, sizeof(storage));

	// if (addrFamily == AF_INET)
	// 	posixListenIpv4(socketFD, port, &storage, maxConnections, boundAddrStr);
	// else
	// 	posixListenIpv6(socketFD, port, &storage, maxConnections, boundAddrStr);

	struct sockaddr_storage storage;
	memset(&storage, 0, sizeof(storage));

	struct sockaddr_in6 *addr = (struct sockaddr_in6 *)&storage;
	memset(addr, 0, sizeof(*addr));

	addr->sin6_family = AF_INET6;
	addr->sin6_addr = in6addr_any; // Significa que o bind deve ocorrer em qualquer endereco da maquina que estiver disponivel (IPv6)
	addr->sin6_port = htons(port); // Host to network short

	// Criar socket
	const int socketFD = socket(addrFamily, SOCK_STREAM, 0);
    if (socketFD == -1)
        commonLogErrorAndDie("Failure as creating listening socket [1]");

    // Evitar que porta utlizada numa execucao fique 02 min inativa apos sua conclusao
    int enableAddrReuse = 1;
    if (setsockopt(socketFD, SOL_SOCKET, SO_REUSEADDR, &enableAddrReuse, sizeof(int)) != 0)
        commonLogErrorAndDie("Failure as creating listening socket [2]");

	// Define timeout de escuta
    if (setsockopt(socketFD, SOL_SOCKET, SO_RCVTIMEO, timeout, sizeof(*timeout)) != 0)
		commonLogErrorAndDie("Failure as creating listening socket [3]");

	// Habilitar conexoes ipv4 em sockets ipv6
	if (addrFamily == AF_INET6) {
		int no = 0;
		if (setsockopt(socketFD, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&no, sizeof(no)) != 0)
        	commonLogErrorAndDie("Failure as trying to enable ipv4 clients to ipv6 server");
	}

	if (bind(socketFD, (struct sockaddr *)addr, sizeof(*addr)) != 0)
		commonLogErrorAndDie("Failure as biding listening socket [ipv6]");
    
	if (listen(socketFD, maxConnections) != 0)
		commonLogErrorAndDie("Failure as starting to listen [ipv6]");

	posixAddressToString((struct sockaddr *)addr, boundAddrStr);

	return socketFD;
}

int posixConnect(const int port, const char *addrStr, const struct timeval *timeout) {

	// Valida endereco
	if (!port)
		commonLogErrorAndDie("invalid connection port");

    // Determina protocolo
	struct sockaddr_storage addrStorage;

    struct in_addr inaddr4; // 32-bit IP address
	const int isIpv4 = inet_pton(AF_INET, addrStr, &inaddr4);

	struct in6_addr inaddr6; // 128-bit IPv6 address
	const int isIpv6 = !isIpv4 ? inet_pton(AF_INET6, addrStr, &inaddr6) : 0;

	if (!isIpv4 && !isIpv6)
		commonLogErrorAndDie("Invalid address family to create connection socket");

	// Cria socket
	int socketFD = socket(isIpv4 ? AF_INET : AF_INET6, SOCK_STREAM, 0);
	if (socketFD == -1)
		commonLogErrorAndDie("Failure as creating connection socket [1]");

	if (setsockopt(socketFD, SOL_SOCKET, SO_RCVTIMEO, timeout, sizeof(*timeout)) != 0)
		commonLogErrorAndDie("Failure as creating connection socket [2]");

	// Trata ipv4
    if (isIpv4) {
		struct sockaddr_in *addr4 = (struct sockaddr_in *)&addrStorage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = htons(port);
        addr4->sin_addr = inaddr4;

    // Trata ipv6
    } else {
		struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)&addrStorage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = htons(port);
        // addr6->sin6_addr = inaddr6
        memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));
    }

	// Cria conexao
	if (connect(socketFD, (struct sockaddr *)(&addrStorage), sizeof(addrStorage)) != 0)
		commonLogErrorAndDie("Failure as creating connection socket [3]");

	return socketFD;
}

ssize_t posixRecv(const int socketFD, char *buffer, struct timeval *timeout) {

	size_t acc = 0;

	while (1) {
		
		ssize_t recvReturn = 0;
		const short int haveExpired = !posixIsActionAvailable(socketFD, FD_ACTION_RD, timeout);

		recvReturn = recv(socketFD, buffer + acc, BUF_SIZE - acc, MSG_DONTWAIT);
		if (recvReturn == 0)
			break;

		if (recvReturn == -1)
			return (errno == EAGAIN || errno == EWOULDBLOCK) ? acc : -1;

		acc += recvReturn;
		if (!haveExpired)
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
		
		if (sentBytes == 0)
			break;

		acc += sentBytes;
	}

	return (acc >= bytesToSend) ? 1 : 0;
}

short int posixAddressToString(const struct sockaddr *addr, char *addrStr) {

	if (addr->sa_family == AF_INET)
		return inet_ntop(AF_INET, &(((struct sockaddr_in *)addr)->sin_addr), addrStr, INET_ADDRSTRLEN + 1) ? 1 : 0;

	if (addr->sa_family == AF_INET6)
		return inet_ntop(AF_INET6, &(((struct sockaddr_in6 *)addr)->sin6_addr), addrStr, INET6_ADDRSTRLEN + 1) ? 1 : 0;
    
	return 0;
}

short int posixIsValidAddrFamily(const int addrFamily) {
	return (addrFamily == AF_INET || addrFamily == AF_INET6);
}