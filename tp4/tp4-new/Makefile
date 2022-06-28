CC = gcc
FLAGS = -W -Wall -g -Wextra
FLAGS_DBG = ${FLAGS} -g -o0

all: obj_common.o obj_server.o obj_equipment.o
	$(CC) $(FLAGS) -o server obj_common.o obj_server.o -lpthread
	$(CC) $(FLAGS) -o equipment obj_common.o obj_equipment.o -lpthread

tests: obj_common.o obj_test_utils.o obj_test_runners.o obj_tests.o
	$(CC) $(FLAGS) -o tests obj_common.o obj_test_utils.o obj_test_runners.o obj_tests.o

memcheck_equipment: obj_common.o obj_equipment.o
	$(CC) $(FLAGS_DBG) -o equipment_memcheck obj_common.o obj_equipment.o -lpthread

memcheck_server: obj_common.o obj_server.o
	$(CC) $(FLAGS_DBG) -o server_memcheck obj_common.o obj_server.o -lpthread

#################################################################################

obj_server.o: server.c common.h
	$(CC) $(FLAGS) -c server.c -o obj_server.o

obj_equipment.o: equipment.c common.h
	$(CC) $(FLAGS) -c equipment.c -o obj_equipment.o

obj_common.o: common.c common.h
	$(CC) $(FLAGS) -c common.c -o obj_common.o

#################################################################################

clean:
	rm -rf obj_* server equipment server_* equipment_*