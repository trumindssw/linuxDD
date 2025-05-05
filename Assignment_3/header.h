#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<sys/sem.h>
#include<unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define PORT 5050
#define SERVER_IP "127.0.0.1"

struct shared_data{
        int flag;
        char text[1024];
};
void sem_wait(int semid);
void sem_signal(int semid);
void message1(char *message);

