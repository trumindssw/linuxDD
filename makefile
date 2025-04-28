all: writer reader 

writer: writer.c file.c
	gcc -o writer writer.c file.c

reader: reader.c file.c
	gcc -o reader reader.c file.c

clean:
	rm -f writer reader shem.txt

