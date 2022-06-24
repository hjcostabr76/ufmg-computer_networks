CC = gcc
CFLAGS = -W -Wall -g -Wextra

all: obj_common.o obj_server.o obj_equipment.o
	$(CC) $(CFLAGS) -o server obj_common.o obj_server.o -lpthread
	$(CC) $(CFLAGS) -o equipment obj_common.o obj_equipment.o

tests: obj_common.o obj_test_utils.o obj_tests.o
	$(CC) $(CFLAGS) -o tests obj_common.o obj_test_utils.o obj_tests.o
	$(CC) $(CFLAGS) -o equipment obj_common.o obj_equipment.o

#################################################################################

obj_server.o: server.c common.h
	$(CC) $(CFLAGS) -c server.c -o obj_server.o

obj_equipment.o: equipment.c common.h
	$(CC) $(CFLAGS) -c equipment.c -o obj_equipment.o

obj_common.o: common.c common.h
	$(CC) $(CFLAGS) -c common.c -o obj_common.o

#################################################################################

obj_tests.o: tests.c test_utils.h common.h
	$(CC) $(CFLAGS) -c tests.c -o obj_tests.o

obj_test_utils.o: test_utils.c test_utils.h
	$(CC) $(CFLAGS) -c test_utils.c -o obj_test_utils.o

#################################################################################

clean:
	rm -rf obj_* server equipment