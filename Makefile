
# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Target to build both server and client
all: s c

# Rule to compile server.c into server.o (includes headers.h)
server.o: server.c headers.h
	$(CC) $(CFLAGS) -c server.c

# Rule to compile client.c into client.o (includes headers.h)
client.o: client.c headers.h
	$(CC) $(CFLAGS) -c client.c

# Link server.o to create the executable 's'
s: server.o
	$(CC) $(CFLAGS) -o s server.o

# Link client.o to create the executable 'c'
c: client.o
	$(CC) $(CFLAGS) -o c client.o

# Clean rule to remove compiled files
clean:
	rm -f s c server.o client.o
	
