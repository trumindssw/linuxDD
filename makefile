# Default target
all: shm_writer shm_reader udpserver

# Compile shm_writer
shm_writer: shm_writer.c headers.h
	gcc -o shm_writer shm_writer.c

# Compile shm_reader
shm_reader: shm_reader.c headers.h
	gcc -o shm_reader shm_reader.c

# Compile udpserver
udpserver: udpserver.c headers.h
	gcc -o udpserver udpserver.c

# Clean target
clean:
	rm -f shm_writer shm_reader udpserver

