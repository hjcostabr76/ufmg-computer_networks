#include <inttypes.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <arpa/inet.h>

void logexit(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

/**
 * inet_pton: internet presentation to network
 * inet_ntop: internet network to presentation
 */

int addrparse(const char *addrstr, const char *portstr, struct sockaddr_storage *storage) {
    
    // Valida parametros
    if (addrstr == NULL || portstr == NULL) {
        return -1;
    }

    // Converte string da porta para numero da porta (em network byte order)
    uint16_t port = (uint16_t)atoi(portstr); // unsigned short
    if (port == 0) {
        return -1;
    }
    port = htons(port); // host to network short

    // Testa se endereco esta em ipv4
    struct in_addr inaddr4; // 32-bit IP address
    if (inet_pton(AF_INET, addrstr, &inaddr4)) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_port = port;
        addr4->sin_addr = inaddr4;
        return 0;
    }

    // Testa se endereco esta em ipv4
    struct in6_addr inaddr6; // 128-bit IPv6 address
    if (inet_pton(AF_INET6, addrstr, &inaddr6)) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_port = port;
        // addr6->sin6_addr = inaddr6
        memcpy(&(addr6->sin6_addr), &inaddr6, sizeof(inaddr6));
        return 0;
    }

    return -1;
}

/**
 * https://youtu.be/DGWlxey644c?t=873
 */
void addrtostr(const struct sockaddr *addr, char *str, size_t strsize) {
    
    int version;
    char addrstr[INET6_ADDRSTRLEN + 1] = "";
    uint16_t port;

    if (addr->sa_family == AF_INET) { // ipv4
        
        version = 4;
        
        struct sockaddr_in *addr4 = (struct sockaddr_in *)addr;
        if (!inet_ntop(AF_INET, &(addr4->sin_addr), addrstr, INET6_ADDRSTRLEN + 1)) {
            logexit("ntop");
        }
        
        port = ntohs(addr4->sin_port); // network to host short

    } else if (addr->sa_family == AF_INET6) { // ipv6
        
        version = 6;
        
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)addr;
        if (!inet_ntop(AF_INET6, &(addr6->sin6_addr), addrstr, INET6_ADDRSTRLEN + 1)) {
            logexit("ntop");
        }
        
        port = ntohs(addr6->sin6_port); // network to host short

    } else {
        logexit("unknown protocol family.");
    }

    if (str) {
        snprintf(str, strsize, "IPv%d %s %hu", version, addrstr, port);
    }
}

// proto: protocolo

int server_sockaddr_init(const char *proto, const char *portstr, struct sockaddr_storage *storage) {
    
    uint16_t port = (uint16_t)atoi(portstr); // unsigned short
    if (port == 0) {
        return -1;
    }
    
    port = htons(port); // host to network short

    memset(storage, 0, sizeof(*storage));
    if (0 == strcmp(proto, "v4")) {
        struct sockaddr_in *addr4 = (struct sockaddr_in *)storage;
        addr4->sin_family = AF_INET;
        addr4->sin_addr.s_addr = INADDR_ANY; // Significa que o bind deve ocorrer em qualquer endereco da maquiena que estiver disponivel (IPv4)
        addr4->sin_port = port;
        return 0;

    } else if (0 == strcmp(proto, "v6")) {
        struct sockaddr_in6 *addr6 = (struct sockaddr_in6 *)storage;
        addr6->sin6_family = AF_INET6;
        addr6->sin6_addr = in6addr_any; // Significa que o bind deve ocorrer em qualquer endereco da maquiena que estiver disponivel (IPv6)
        addr6->sin6_port = port;
        return 0;
    }

    return -1;
}
