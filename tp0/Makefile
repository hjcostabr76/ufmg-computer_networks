all:
	gcc -Wall -c common/common.c
	gcc -Wall -c common/caesar_cipher.c
	gcc -Wall -c common/posix_utils.c
	gcc -Wall -c common/string_utils.c
	gcc -Wall -c client_utils.c
	gcc -Wall cliente.c caesar_cipher.o posix_utils.o string_utils.o common.o client_utils.o -o cliente
	gcc -Wall -c server_utils.c
	gcc -Wall servidor.c caesar_cipher.o posix_utils.o string_utils.o common.o server_utils.o -lpthread -o servidor

clean:
	rm common.o client server server-mt
