#define SHM_key     1234
#define SEM_key     5678
#define SIZE    sizeof(struct Data)
#define file        "shm.txt"
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>
void lock_semaphore(int semid);
void unlock_semaphore(int semid);
void message1(char *message);
struct Data
{
    char msg[1000];
};


