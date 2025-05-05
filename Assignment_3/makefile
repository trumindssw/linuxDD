all: writer reader udp_server

writer: writer.c file.o
	$(CC) -o writer writer.c file.o

reader: reader.c file.o
	$(CC) -o reader reader.c file.o

udp_server: udp_server.c
	$(CC) -o udp_server udp_server.c

file.o: file.c header.h
	$(CC) -c file.c

clean:
	rm -f writer reader udp_server shem.txt

