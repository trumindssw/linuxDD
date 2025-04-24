
# Compiler and flags
CC = gcc
CFLAGS = -Wall -g

# Source files
SRCS = writer.c reader.c udp_server.c ipc_shared_memory.c

# Object files
OBJS = writer.o reader.o udp_server.o ipc_shared_memory.o

# Header file
HEADER = ipc_shared_memory.h

# Executables
TARGETS = writer reader udp_server

# Default target
all: $(TARGETS)

# Writer binary
writer: writer.o ipc_shared_memory.o
	$(CC) $(CFLAGS) writer.o ipc_shared_memory.o -o writer -lrt

# Reader binary
reader: reader.o ipc_shared_memory.o
	$(CC) $(CFLAGS) reader.o ipc_shared_memory.o -o reader -lrt

# UDP Server binary
udp_server: udp_server.o
	$(CC) $(CFLAGS) udp_server.o -o udp_server

# Object file rules
writer.o: writer.c $(HEADER)
	$(CC) $(CFLAGS) -c writer.c

reader.o: reader.c $(HEADER)
	$(CC) $(CFLAGS) -c reader.c

udp_server.o: udp_server.c
	$(CC) $(CFLAGS) -c udp_server.c

ipc_shared_memory.o: ipc_shared_memory.c $(HEADER)
	$(CC) $(CFLAGS) -c ipc_shared_memory.c

# Clean target
clean:
	rm -f *.o $(TARGETS)

