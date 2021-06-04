#pragma once

#include <arpa/inet.h>
#include <time.h>

/**
 * TODO: 2021-06-04 - ADD Descricao
 */
int posixListen(const int port, const struct timeval *timeout, const int maxConnections, char *boundAddrStr);

/**
 * TODO: 2021-06-04 - ADD Descricao
 */
int posixConnect(const int port, const char *addrStr, const struct timeval *timeout);

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
void posixRecv(const int socketFD, char *buffer, unsigned *recvBytesAcc, struct timeval *timeout);

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
int posixSend(const int socketFD, const char *buffer, const int bytesToSend, struct timeval *timeout);

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
int posixAddressToString(const struct sockaddr *addr, char *addrStr);
