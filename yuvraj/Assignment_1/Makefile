# CC = gcc
# CFLAGS = -Wall -pthread

all: writer reader

writer: writer.o
	$(CC) $(CFLAGS) -o writer writer.o

reader: reader.o
	$(CC) $(CFLAGS) -o reader reader.o

writer.o: writer.c shared.h
reader.o: reader.c shared.h

clean:
	rm -f writer reader *.o shmem.txt
