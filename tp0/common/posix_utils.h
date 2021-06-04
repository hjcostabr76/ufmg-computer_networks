#pragma once

#include <arpa/inet.h>
#include <time.h>

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
void posixReceive(const int socketFD, char *buffer, unsigned *recvBytesAcc, struct timeval *timeout);

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
int posixSend(const int socketFD, const char *buffer, const int bytesToSend, struct timeval *timeout);

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
int posixAddressToString(const struct sockaddr *address, char *addressString);
