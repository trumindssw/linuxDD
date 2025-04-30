CC=gcc
CFLAGS=-Wall

all: writer reader udp_server inspect

writer: writer.c shm.c
	$(CC) $(CFLAGS) -o writer writer.c shm.c

reader: reader.c shm.c
	$(CC) $(CFLAGS) -o reader reader.c shm.c

udp_server: udp_server.c
	$(CC) $(CFLAGS) -o udp_server udp_server.c

inspect: inspect_shm.c shm.c
	$(CC) $(CFLAGS) -o inspect_shm inspect_shm.c shm.c

clean:
	rm -f writer reader udp_server inspect_shm

