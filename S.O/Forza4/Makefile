GCC = gcc
AS_FLAGS= --32 -gstabs
LD_FLAGS=-m elf_i386
FLAGS = -g -c

all: Server Client

Server: obj/errExit.o obj/F4Server.o obj/shared_memory.o obj/semaphore.o
	$(GCC) obj/errExit.o obj/F4Server.o obj/semaphore.o obj/shared_memory.o -o exc/F4Server

Client: obj/errExit.o obj/F4Client.o obj/shared_memory.o obj/semaphore.o
	$(GCC) obj/errExit.o obj/F4Client.o obj/semaphore.o obj/shared_memory.o -o exc/F4Client

obj/errExit.o: src/errExit.c
	$(GCC) $(FLAGS) src/errExit.c	 -o obj/errExit.o

obj/F4Server.o: src/F4Server.c
	$(GCC) $(FLAGS) src/F4Server.c -o obj/F4Server.o

obj/F4Client.o: src/F4Client.c
	$(GCC) $(FLAGS) src/F4Client.c -o obj/F4Client.o

obj/matrix.o: src/matrix.c
	$(GCC) $(FLAGS) src/matrix.c -o obj/matrix.o

obj/order.o: src/order.c
	$(GCC) $(FLAGS) src/order.c -o obj/order.o

obj/prova.o: src/prova.c
	$(GCC) $(FLAGS) src/prova.c -o obj/prova.o

obj/semaphore.o: src/semaphore.c
	$(GCC) $(FLAGS) src/semaphore.c -o obj/semaphore.o

obj/shared_memory.o: src/shared_memory.c
	$(GCC) $(FLAGS) src/shared_memory.c -o obj/shared_memory.o

obj/sleep.o: src/sleep.c
	$(GCC) $(FLAGS) src/sleep.c -o obj/sleep.o

.PHONY: clean

clean:
	@rm -f obj/*.o exc/*
	@echo "Removed object files and executables..."
	