all: writer reader

writer: writer.c
	gcc -Wall writer.c -o writer

reader: reader.c
	gcc -Wall reader.c -o reader

clean:
	rm -f writer reader shmem.txt
