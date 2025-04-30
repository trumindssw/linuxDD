

# Makefile

CC = gcc             # Compiler
CFLAGS = -Wall -g    # Compiler flags (Wall: warnings, g: debug info)

all: server client   # Default target: build both

server: server.o
	$(CC) $(CFLAGS) -o server server.o

client: client.o
	$(CC) $(CFLAGS) -o client client.o

server.o: server.c
	$(CC) $(CFLAGS) -c server.c

client.o: client.c
	$(CC) $(CFLAGS) -c client.c

clean:
	rm -f *.o server client

