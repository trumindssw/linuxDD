all:sender reciever
sender:sender.c file.c
	gcc -o sender sender.c file.c
reciever:reciever.c file.c
	gcc -o reciever reciever.c file.c
clean:
	rm -f sender reciever shm.txt 
