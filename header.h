#include<stdio.h>
#include<sys/ipc.h>
#include<sys/shm.h>
#include<stdlib.h>
#include<sys/ipc.h>
#include<sys/sem.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<string.h>



//Shared Memory Constants 
#define SHM_KEY 12345
#define SHM_SIZE 1024

//Semaphore Operation Constants
#define SEM_KEY 54321



