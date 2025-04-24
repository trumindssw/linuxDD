

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>


#define SHM_KEY 5
#define SEM_KEY 1
#define SHM_SIZE 1024
#define PORT_NUM 2222
void sem_wait(int sem_id);
void sem_signal(int sem_id);
void write_into_file(FILE *,char *);
