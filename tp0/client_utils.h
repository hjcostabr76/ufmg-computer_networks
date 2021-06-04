#pragma once

#include <time.h>

/**
 * TODO: 2021-05-31 - ADD Descricao
 */
enum ClientFooEnum {
    CLI_SEND_PARAM_NUM = 1,
    CLI_SEND_PARAM_STR
};

/**
 * TODO: 2021-05-31 - ADD Descricao
 */
void clientSendParam(
	const int socketFD,
	char *buffer,
	const char *valueToSend,
	const struct timeval *timeout,
	const enum ClientFooEnum varType,
	const int opNum,
	const short int mustWaitForAnswer
);

/**
 * TODO: 2021-05-31 - ADD Descricao
 */
int clientValidateInput(int argc, char **argv);

/**
 * - Recebe struct ainda a ser preenchida com dados de ipv4;
 * - O tipo sockaddr_storage eh 'super classe' de sockaddr_in & sockaddr_in6;
 * 
 * NOTE: inet_pton: internet presentation to network
 * NOTE: inet_ntop: internet network to presentation
 * NOTE: AF: Address Family
 * 
 * @param addr_number_str: String com numero do endereco ipv4;
 * @param portstr: String com numero da porta;
 * @param addr: Struct generica a ser preenchida com dados de conexao ipv4;
 */
int clientParseAddress(const char *addrstr, const char *portstr, struct sockaddr_storage *addr);
