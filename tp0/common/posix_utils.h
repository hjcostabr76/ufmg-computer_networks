#pragma once

#include <arpa/inet.h>

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
void posixReceive(const int sock, char *buffer, unsigned *receivedBytesAcc);
// void posixReceive(const int sock, char *buffer);

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
int posixSend(const int sock, const char *buffer, const int bytesToSend);

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
int posixAddressToString(const struct sockaddr *address, char *addressString);
