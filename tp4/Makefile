CC = gcc
CFLAGS = -W -Wall -g -Wextra

all: obj_common.o obj_server.o obj_equipment.o
	$(CC) $(CFLAGS) -o server obj_common.o obj_server.o -lpthread
	$(CC) $(CFLAGS) -o equipment obj_common.o obj_equipment.o

#################################################################################

obj_server.o: server.c common.h
	$(CC) $(CFLAGS) -c server.c -o obj_server.o

obj_equipment.o: equipment.c common.h
	$(CC) $(CFLAGS) -c equipment.c -o obj_equipment.o

obj_common.o: common.c
	$(CC) $(CFLAGS) -c common.c -o obj_common.o

#################################################################################

clean:
	rm -rf obj_* server equipment