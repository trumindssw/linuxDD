# Compiler and flags
CC = gcc
CFLAGS = -Wall

# Default target
all: tcp_server tcp_client

# Compile server
tcp_server: tcp_server.c
	$(CC) $(CFLAGS) -o tcp_server tcp_server.c

# Compile client
tcp_client: tcp_client.c
	$(CC) $(CFLAGS) -o tcp_client tcp_client.c

# Clean build files
clean:
	rm -f tcp_server tcp_client

