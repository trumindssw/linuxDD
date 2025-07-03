# Makefile

CC = gcc
CFLAGS = -Wall

all: writer reader

writer: writer.c
	$(CC) $(CFLAGS) writer.c -o writer

reader: reader.c
	$(CC) $(CFLAGS) reader.c -o reader

clean:
	rm -f writer reader shmem.txt
