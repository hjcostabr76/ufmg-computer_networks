#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

void usage(int argc, char **argv) {
	printf("usage: %s <server IP> <server port>\n", argv[0]);
	printf("example: %s 127.0.0.1 51511\n", argv[0]);
	exit(EXIT_FAILURE);
}

#define BUFSZ 1024

int main(int argc, char **argv) {
	
	// Valida parametros de linha de comando
	if (argc < 3) {
		usage(argc, argv);
	}

	// Define endereco do socket (ipv4 OU ipv6)
	struct sockaddr_storage storage;
	if (0 != addrparse(argv[1], argv[2], &storage)) { // Funcao customizada
		usage(argc, argv);
	}

	// Cria & conecta socket
	int s;
	s = socket(storage.ss_family, SOCK_STREAM, 0); // socket tcp (existem outros tipos)
	if (s == -1) {
		logexit("socket");
	}

	struct sockaddr *addr = (struct sockaddr *)(&storage);
	if (0 != connect(s, addr, sizeof(storage))) {
		logexit("connect");
	}

	// Exibe endereco no qual o socket se conectou
	char addrstr[BUFSZ];
	addrtostr(addr, addrstr, BUFSZ);

	printf("connected to %s\n", addrstr);

	// Envia pro servidor a mensagem recebida via terminal
	char buf[BUFSZ];
	memset(buf, 0, BUFSZ); // Inicializar buffer com 0 (acho que essa funcao preenche a memoria com o valor definido)
	
	printf("mensagem> ");
	fgets(buf, BUFSZ-1, stdin);
	
	size_t count = send(s, buf, strlen(buf)+1, 0); // Retorna qtd de bytes transmitidos (3o argumento serve para parametrizar o envio)
	if (count != strlen(buf)+1) {
		logexit("send");
	}

	// Recebe dados do servidor
	memset(buf, 0, BUFSZ);
	unsigned total = 0;

	while (1) {
		count = recv(s, buf + total, BUFSZ - total, 0);
		if (count == 0) {
			// Connection terminated.
			break;
		}
		total += count;
	}

	close(s);

	printf("received %u bytes\n", total);
	puts(buf);

	exit(EXIT_SUCCESS);
}