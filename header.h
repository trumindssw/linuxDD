#include<stdio.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<unistd.h>
struct shared_data{
	int flag;
	char text[1024];
};
void sem_wait(int semid);
void sem_signal(int semid);
void message1(char *message);
