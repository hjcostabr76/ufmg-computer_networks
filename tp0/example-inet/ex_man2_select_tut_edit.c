#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <errno.h>
#include <stdio.h>
#include <signal.h>

#undef max
#define max(x,y) ((x) > (y) ? (x) : (y))

#define SHUT_FD1 do {                               \
                    if (fd1 >= 0) {                 \
                        shutdown(fd1, SHUT_RDWR);   \
                        close(fd1);                 \
                        fd1 = -1;                   \
                    }                               \
                } while (0)

#define SHUT_FD2 do {                               \
                    if (fd2 >= 0) {                 \
                        shutdown(fd2, SHUT_RDWR);   \
                        close(fd2);                 \
                        fd2 = -1;                   \
                    }                               \
                } while (0)



#define BUF_SIZE 1024

static int forward_port;
static int listen_socket(int listen_port);
static int connect_socket(int connect_port, char *address);

/**
 * =================================================================================
 * === Rescrita (para entendimento) do programa de exmplo da documentacao linux: ===
 * ------- **** Transfere conteudo de 01 FD1 para 01 FD2 & vice-versa **** ---------
 * =================================================================================
 * 
 * ----------------------------------------------------------------
 * 
 * Here is an example that better demonstrates the true utility of select().
 * The listing below is a TCP forwarding program that forwards from one TCP port to another.
 * 
 * ----------------------------------------------------------------
 * 
 * The above program properly forwards most kinds of TCP connections including OOB signal data transmitted
 * by telnet servers. It handles the tricky problem of having data flow in both directions simultaneously.
 * You might think it more efficient to use a fork(2) call and devote a thread to each stream. This becomes
 * more tricky than you might suspect. Another idea is to set nonblocking I/O using fcntl(2). This also has
 * its problems because you end up using inefficient timeouts.
 * 
 * The program does not handle more than one simultaneous connection at a time, although it could easily be
 * extended to do this with a linked list of buffers—one for each connection. At the moment, new connections
 * cause the current connection to be dropped.
 * 
 * @see https://man7.org/linux/man-pages/man2/select_tut.2.html
 */
int
main(int argc, char *argv[]) {

	if (argc != 4) {
		fprintf(stderr, "Usage\n\tfwd <listen-port> " "<forward-to-port> <forward-to-ip-address>\n");
		exit(EXIT_FAILURE);
	}
	
	// TODO: wtf?
	int fd1 = -1;
	int buf1_avail = 0;
	int buf1_written = 0;

	// TODO: wtf?
	int fd2 = -1;
	int buf2_avail = 0;
	int buf2_written = 0;
	
	// TODO: wtf?
	char buf1[BUF_SIZE];
	char buf2[BUF_SIZE];

	// TODO: wtf?
	signal(SIGPIPE, SIG_IGN);
	forward_port = atoi(argv[2]);

	// Inicia 01 socket de servidor
	int h = listen_socket(atoi(argv[1]));
	if (h == -1)
		exit(EXIT_FAILURE);

	while (1) {

		ssize_t nbytes;
		
		fd_set readfds;
		fd_set writefds;
		fd_set exceptfds;

		// Reseta file descriptors
		FD_ZERO(&readfds);
		FD_ZERO(&writefds);
		FD_ZERO(&exceptfds);
		FD_SET(h, &readfds);

		// int nfds = 0; // TODO: Qual a diferenca dessa sintaxe para a de baixo!?
		// nfds = max(nfds, h);
		int nfds = h; // TODO: wtf?

		/** ======================================================== */
		/** -- Atualiza disponibilidade dos FDs para i/o  ---------- */
		/** ======================================================== */

		if (fd1 > 0) {

			if (buf1_avail < BUF_SIZE) // Leitura
				FD_SET(fd1, &readfds); /* Note: nfds is updated below, when fd1 is added to exceptfds. */

			if (buf2_avail - buf2_written > 0) // Escrita
				FD_SET(fd1, &writefds);
			
			FD_SET(fd1, &exceptfds); // TODO: wff?
				nfds = max(nfds, fd1);
		}

		if (fd2 > 0) {

			if (buf2_avail < BUF_SIZE) // Leitura
				FD_SET(fd2, &readfds);

			if (buf1_avail - buf1_written > 0) // Escrita
				FD_SET(fd2, &writefds);
			
			FD_SET(fd2, &exceptfds); // TODO: wff?
				nfds = max(nfds, fd2);
		}

		/** ======================================================== */
		/** -- Verifica se ha FDs disponiveis para alguma acao ----- */
		/** ======================================================== */

		int ready = select(nfds + 1, &readfds, &writefds, &exceptfds, NULL);
		if (ready == -1) { // Erro
			if (errno == EINTR) // Erro: Interrupted System Call | TODO: Pq esse erro eh ignorado?
				continue;
			perror("select()");
			exit(EXIT_FAILURE);
		}

		/** ======================================================== */
		/** -- Faz stream de FD2 (cliente) -> FD1 (servidor) ------- */
		/** ======================================================== */

		if (FD_ISSET(h, &readfds)) {

			struct sockaddr_in client_addr;
			socklen_t addrlen = sizeof(client_addr);
			memset(&client_addr, 0, addrlen);

			int fd = accept(h, (struct sockaddr *)&client_addr, &addrlen);
			if (fd != -1) {

				// Reset dos FDs
				SHUT_FD1; // TODO: Verificar se eh isso mesmo
				SHUT_FD2;
				buf1_avail = buf1_written = 0;
				buf2_avail = buf2_written = 0;

				/*
					FD1: Socket servidor
					FD2: Socket cliente
				*/

				fd1 = fd;
				fd2 = connect_socket(forward_port, argv[3]);

				if (fd2 == -1)
					SHUT_FD1; // Encerra FD1
				else /* Skip any events on the old, closed file descriptors. */
					printf("connect from %s\n", inet_ntoa(client_addr.sin_addr));

				continue;
			}

			perror("accept()");
		}

		/* NB: read OOB data before normal reads. */ // TODO: wff!?

		/** == TODO: WFF? ========================================== */

		if (fd1 > 0) {

			// TODO: wff!?
			if (FD_ISSET(fd1, &exceptfds)) {
				char c;

				nbytes = recv(fd1, &c, 1, MSG_OOB); // Solicita alerta de 'out of band data'
				if (nbytes < 1)
					SHUT_FD1;
				else
					send(fd2, &c, 1, MSG_OOB);	
			}

			// TODO: wff!?
			if (FD_ISSET(fd1, &readfds)) {
				nbytes = read(fd1, buf1 + buf1_avail, BUF_SIZE - buf1_avail);
				if (nbytes < 1)
					SHUT_FD1;
				else
					buf1_avail += nbytes;
			}

			// TODO: wff!?
			if (fd1 > 0 && FD_ISSET(fd1, &writefds) && buf2_avail > 0) {
				nbytes = write(fd1, buf2 + buf2_written, buf2_avail - buf2_written);
				if (nbytes < 1)
					SHUT_FD1;
				else
					buf2_written += nbytes;
			}
		}

		if (fd2 > 0) {

			// TODO: wff!?
			if (FD_ISSET(fd2, &exceptfds)) {
				char c;
				nbytes = recv(fd2, &c, 1, MSG_OOB);
				if (nbytes < 1)
					SHUT_FD1;
				else
					send(fd1, &c, 1, MSG_OOB);	
			}

			// TODO: wff!?
			if (FD_ISSET(fd2, &readfds)) {
				nbytes = read(fd2, buf2 + buf2_avail, BUF_SIZE - buf2_avail);
				if (nbytes < 1)
					SHUT_FD1;
				else
					buf2_avail += nbytes;
			}

			// TODO: wff!?
			if (FD_ISSET(fd2, &writefds) && buf1_avail > 0) {
				nbytes = write(fd2, buf1 + buf1_written, buf1_avail - buf1_written);
				if (nbytes < 1)
					SHUT_FD1;
				else
					buf1_written += nbytes;
			}
		}

		/** == TODO: WFF? ========================================== */
		/* Check if write data has caught read data. */ // TODO: wtf!?

		if (buf1_written == buf1_avail)
			buf1_written = buf1_avail = 0;
		if (buf2_written == buf2_avail)
			buf2_written = buf2_avail = 0;

		/** == TODO: WFF? ========================================== */
		/* One side has closed the connection, keep writing to the other side until empty. */ // TODO: wtf!?

		if (fd1 < 0 && buf1_avail - buf1_written == 0)
			SHUT_FD2;
		if (fd2 < 0 && buf2_avail - buf2_written == 0)
			SHUT_FD1;
	}
}

static int
listen_socket(int listen_port) {

    struct sockaddr_in addr;
    int lfd;
    int yes;

    lfd = socket(AF_INET, SOCK_STREAM, 0);
    if (lfd == -1) {
        perror("socket");
        return -1;
    }

    yes = 1;
    if (setsockopt(lfd, SOL_SOCKET, SO_REUSEADDR,
            &yes, sizeof(yes)) == -1) {
        perror("setsockopt");
        close(lfd);
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_port = htons(listen_port);
    addr.sin_family = AF_INET;
    if (bind(lfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("bind");
        close(lfd);
        return -1;
    }

    printf("accepting connections on port %d\n", listen_port);
    listen(lfd, 10);
    return lfd;
}

static int
connect_socket(int connect_port, char *address)
{
    struct sockaddr_in addr;
    int cfd;

    cfd = socket(AF_INET, SOCK_STREAM, 0);
    if (cfd == -1) {
        perror("socket");
        return -1;
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_port = htons(connect_port);
    addr.sin_family = AF_INET;

    if (!inet_aton(address, (struct in_addr *) &addr.sin_addr.s_addr)) {
        fprintf(stderr, "inet_aton(): bad IP address format\n");
        close(cfd);
        return -1;
    }

    if (connect(cfd, (struct sockaddr *) &addr, sizeof(addr)) == -1) {
        perror("connect()");
        shutdown(cfd, SHUT_RDWR);
        close(cfd);
        return -1;
    }
    return cfd;
}