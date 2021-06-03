#pragma once

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
void posixReceive(const int sock, char *buffer, unsigned *receivedBytesAcc);

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
int posixSend(const int sock, char *buffer, const int bytesToSend);

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
int addressToString(const struct sockaddr *address, char *addressString);
