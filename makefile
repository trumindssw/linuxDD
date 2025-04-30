all: server client

server: server.c headers.h
	$(CC) server.c -o server

client: client.c headers.h
	$(CC) client.c -o client

clean:
	rm -f server client

