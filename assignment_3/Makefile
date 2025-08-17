all: writer reader_client udp_server

writer: writer.c
	gcc writer.c -o writer

reader_client: reader_client.c
	gcc reader_client.c -o reader_client

udp_server: udp_server.c
	gcc udp_server.c -o udp_server

clean:
	rm -f writer reader_client udp_server
