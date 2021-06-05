#pragma once

#include <time.h>

struct ClientData {
    int socket;
    struct sockaddr_storage address;
    struct timeval timeout;
};

/**
 * TODO: 2021-06-04 - ADD Descricao
 */
enum ServerRecvValidationEnum {
    RCV_VALIDATION_NUMERIC = 1,
    RCV_VALIDATION_LCASE
};

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
void serverRecvParam(
    struct ClientData *client,
    char *buffer,
    const unsigned bytesToRecv,
    const enum ServerRecvValidationEnum validationType,
    const char *opLabel
);

/**
 * TODO: 2021-06-03 - ADD Descricao
 */
int serverValidateInput(int argc, char **argv);
