all: writer reader

writer: writer.c shared.h
	gcc -o writer writer.c -pthread

reader: reader.c shared.h
	gcc -o reader reader.c -pthread

clean:
	rm -f writer reader shmem.txt

