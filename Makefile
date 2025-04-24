all: writer reader sem_writer sem_reader sem1_writer sem1_reader sem2_writer sem2_reader server

writer: sh_writer.c
	gcc -o writer sh_writer.c

reader: sh_reader.c
	gcc -o reader sh_reader.c

sem_writer: sem_writer.c
	gcc -o sem_writer sem_writer.c

sem_reader: sem_reader.c
	gcc -o sem_reader sem_reader.c

sem1_writer: sem1_writer.c
	gcc -o sem1_writer sem1_writer.c

sem1_reader: sem1_reader.c
	gcc -o sem1_reader sem1_reader.c

sem2_writer: sem2_writer.c
	gcc -o sem2_writer sem2_writer.c

sem2_reader: sem2_reader.c
	gcc -o sem2_reader sem2_reader.c

server: server.c
	gcc -o server server.c

clean:
	rm -f writer reader sem_writer sem_reader sem1_writer sem1_reader sem2_writer sem2_reader server

