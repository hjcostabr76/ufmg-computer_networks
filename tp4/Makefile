CC = gcc
CFLAGS = -W -Wall -g -Wextra

all: obj/common.o obj/server.o obj/equipment.o
	$(CC) $(CFLAGS) -o server obj/common.o obj/server.o
	$(CC) $(CFLAGS) -o equipment obj/common.o obj/equipment.o

#################################################################################

obj/server.o: server.c common.h
	$(CC) $(CFLAGS) -c server.c -o obj/server.o

obj/equipment.o: equipment.c common.h
	$(CC) $(CFLAGS) -c equipment.cpp -o obj/equipment.o

obj/common.o: common.c
	$(CC) $(CFLAGS) -c common.c -o obj/common.o

#################################################################################

clean:
	rm -rf obj/* server client