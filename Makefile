CC=gcc
CFLAGS=-Wall -g

all: writer reader server

writer: writer.c
	$(CC) $(CFLAGS) -o writer writer.c

reader: reader.c
	$(CC) $(CFLAGS) -o reader reader.c

server: server.c
	$(CC) $(CFLAGS) -o server server.c

clean:
	rm -f writer reader server
	
