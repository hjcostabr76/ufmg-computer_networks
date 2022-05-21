all:
	gcc -Wall -c common.c
	gcc -Wall tests.c common.o -o tests
	gcc -Wall client.c common.o -o client
	gcc -Wall server.c common.o -o server

clean:
	rm common.o client server tests
