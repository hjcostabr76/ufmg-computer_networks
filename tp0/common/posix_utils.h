#pragma once

#include <arpa/inet.h>
#include <time.h>

enum FdActionEnum {
	FD_ACTION_RD = 10,
	FD_ACTION_WT
};

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
int posixIsActionAvailable(int socketFD, const enum FdActionEnum action, struct timeval *timeout);

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
