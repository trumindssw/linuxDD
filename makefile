all: server client

server: server11.c headers11.h
	$(CC) server11.c -o server

client: client11.c headers11.h
	$(CC) client11.c -o client

clean:
	rm -f server client

