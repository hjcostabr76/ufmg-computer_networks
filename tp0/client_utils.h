#pragma once

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