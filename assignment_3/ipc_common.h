// ipc_common.h
#ifndef IPC_COMMON_H
#define IPC_COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <arpa/inet.h>

#define SHM_SIZE 100
#define MAX_INPUT 100 

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678

// Decrement Operation
void sem_wait(int semid) {
    struct sembuf sops = {0, -1, 0};
    semop(semid, &sops, 1);
}

// Increment Operation
void sem_signal(int semid) {
    struct sembuf sops = {0, 1, 0};
    semop(semid, &sops, 1);
}

#endif
