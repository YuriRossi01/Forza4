GCC = gcc
FLAGS = -g -c

all: Server Client

Server: obj/errExit.o obj/F4Server.o obj/shared_memory.o obj/semaphore.o obj/matrix.o
	$(GCC) obj/errExit.o obj/F4Server.o obj/semaphore.o obj/shared_memory.o obj/matrix.o -o exc/F4Server

Client: obj/errExit.o obj/F4Client.o obj/shared_memory.o obj/semaphore.o obj/matrix.o
	$(GCC) obj/errExit.o obj/F4Client.o obj/semaphore.o obj/shared_memory.o obj/matrix.o -o exc/F4Client

obj/errExit.o: src/errExit.c
	$(GCC) $(FLAGS) src/errExit.c	 -o obj/errExit.o

obj/F4Server.o: src/F4Server.c
	$(GCC) $(FLAGS) src/F4Server.c -o obj/F4Server.o

obj/F4Client.o: src/F4Client.c
	$(GCC) $(FLAGS) src/F4Client.c -o obj/F4Client.o

obj/matrix.o: src/matrix.c
	$(GCC) $(FLAGS) src/matrix.c -o obj/matrix.o

obj/semaphore.o: src/semaphore.c
	$(GCC) $(FLAGS) src/semaphore.c -o obj/semaphore.o

obj/shared_memory.o: src/shared_memory.c
	$(GCC) $(FLAGS) src/shared_memory.c -o obj/shared_memory.o

.PHONY: clean

clean:
	@rm -f obj/*.o exc/*
	@echo "Removed object files and executables..."
	