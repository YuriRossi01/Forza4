
CFLAGS=-Wall -std=gnu99
INCLUDES=-I./inc

SERVER_SRCS=./errExit.c ./F4Server.c ./shared_memory.c
CLIENT_SRCS=./errExit.c ./F4Client.c

SERVER_OBJS=$(SERVER_SRCS:.c=.o)
CLIENT_OBJS=$(CLIENT_SRCS:.c=.o)

all: server client

server: $(SERVER_OBJS)
	@echo "Making executable: "$@
	@$(CC) $^ -o $@

client: $(CLIENT_OBJS)
	@echo "Making executable: "$@
	@$(CC) $^ -o $@

.c.o:
	@echo "Compiling: "$<
	@$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

.PHONY: clean

clean:
	@rm -f src/*.o client server
	@echo "Removed object files and executables..."
