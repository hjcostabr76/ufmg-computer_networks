#pragma once

#define RCV_VALIDATION_NUMERIC 1
#define RCV_VALIDATION_LCASE 2

#define RCV_ERR_BYTES_COUNT -1
#define RCV_ERR_VALIDATION_PARAM -2
#define RCV_ERR_VALIDATION_STR -3
#define RCV_ERR_SUCCESS_RETURN -4
#define RCV_SUCCESS 1

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
int serverInitSocket(const char *portStr, struct sockaddr_storage *address);

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
int serverReceiveParam(const int sock, char *buffer, const unsigned bytesToReceive, const int validationType);

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
void serverSendFailureResponse(const int sock, char *errMsg);

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
int serverValidateInput(int argc, char **argv);
