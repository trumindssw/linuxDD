//here all capital letters words are global variables


CC       := gcc                                   //giving gcc compiler to CC                                 
CFLAGS   := -Wall -g                   //Wall for include warnings and -g for enable debugging symbols
SHARED_OBJS := write_into_file.o       //this .o file is we used in 2 other reader and writer files
ALL_PROGS := writer reader udp_server   

.PHONY: all clean run_writer run_reader run_udp   //.PHONY is for make while executing if any file name with the mentiones names matched on the disk ,just ignore as they are not the real files

all: $(ALL_PROGS)

# Compile shared code into write_into_file.o
write_into_file.o: write_into_file.c
	$(CC) $(CFLAGS) -c write_into_file.c -o write_into_file.o

# Link writer and reader with the shared object
writer: writer.c $(SHARED_OBJS)
	$(CC) $(CFLAGS) writer.c $(SHARED_OBJS) -o writer

reader: reader.c $(SHARED_OBJS)
	$(CC) $(CFLAGS) reader.c $(SHARED_OBJS) -o reader

# UDP server does not use the shared object
udp_server: udp_server.c
	$(CC) $(CFLAGS) udp_server.c -o udp_server

/*The run targets in the Makefile (e.g. run_reader, run_writer) are purely for your convenience—they let you:

Automatically rebuild the program if its sources have changed

Embed runtime arguments (like $(ARG)) in one place

Avoid typing two commands (make reader then ./reader foo) every time

*/

# Run targets
run_writer: writer
	./writer

run_reader: reader
ifndef ARG
	$(error Please provide ARG. Usage: make run_reader ARG="your_argument")
endif
	./reader $(ARG)

run_udp: udp_server
	./udp_server

# Clean up all binaries and object files
clean:
	rm -f $(ALL_PROGS) $(SHARED_OBJS)

