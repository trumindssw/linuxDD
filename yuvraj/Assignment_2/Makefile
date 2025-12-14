# CC = gcc
# CFLAGS = -Wall -Wextra

all: server client

server: server.c shared.h
	$(CC) $(CFLAGS) -o server server.c

client: client.c shared.h
	$(CC) $(CFLAGS) -o client client.c

clean:
	rm -f server client
