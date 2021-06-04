#pragma once

#include <arpa/inet.h>
#include <time.h>

/**
 * TODO: 2021-06-04 - ADD Descricao
 */
enum ServerRecvValidationEnum {
    RCV_VALIDATION_NUMERIC = 1,
    RCV_VALIDATION_LCASE
};

/**
 * TODO: 2021-06-04 - ADD Descricao
 */
enum ServerRecvReturnEnum {
    RCV_ERR_BYTES_COUNT = 1,
    RCV_ERR_VALIDATION_PARAM,
    RCV_ERR_VALIDATION_STR,
    RCV_ERR_SUCCESS_RETURN,
    RCV_SUCCESS
};

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
int serverInitSocket(const char *portStr, struct sockaddr_storage *address);

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
void serverRecvParam(
    const int socketFD,
    char *buffer,
    const unsigned bytesToRecv,
    const enum ServerRecvValidationEnum validationType,
    struct timeval *timeout,
    const char *opLabel
);

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
void serverSendFailureResponse(const int sock, char *errMsg, struct timeval *timeout);

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
int serverValidateInput(int argc, char **argv);
