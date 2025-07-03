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

#define SHM_KEY 1234
#define SEM_KEY 5678
#define SHM_SIZE 1024
#define MAX_INPUT 100 

// Semaphore wait (P)
// Deceremnt operation

void sem_wait(int semid) {
    struct sembuf sops = {0, -1, 0};
    // defining the semaphore
    // Arg1: 0 : semaphore number ranging from 0 to nsems-1
    // Arg2: -1 : semaphore operation (decrement)
    // Agr3: 0 : operation flags (none set)
    semop(semid, &sops, 1);
    // operation of the semaphore 
    // semid : id of the semaphore 
    // &sops : pointer to the struct of sembuf (variable named sops)
    // 1 : no.of semaphores in the semaphore array
}

// Semaphore signal (V)
// Increment operation
void sem_signal(int semid) {
    struct sembuf sops = {0, 1, 0};
    semop(semid, &sops, 1);
}

#endif
