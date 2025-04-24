
all: write read
 
write: write.c
	gcc write.c -o write
 
read: read.c
	gcc read.c -o read
 
clean:
	rm -f write read shmem.txt
