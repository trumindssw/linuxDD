CC=gcc
CFLAGS=-Wall

all: client server udpserver

client: client.c
	$(CC) $(CFLAGS) -o client client.c

server: server.c
	$(CC) $(CFLAGS) -o server server.c

receiver: udpserver.c
	$(CC) $(CFLAGS) -o udpserver udpserver.c

clean:
	rm -f client server udpserver

