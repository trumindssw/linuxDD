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
void sem_wait(int semid) {
    struct sembuf sops = {0, -1, 0};
    semop(semid, &sops, 1);
}

// Semaphore signal (V)
void sem_signal(int semid) {
    struct sembuf sops = {0, 1, 0};
    semop(semid, &sops, 1);
}

#endif
