#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include "shm.h"

union semun {
    int val;
};

void init_shared_memory(int *shmid, char **data) {
    *shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (*shmid < 0) { perror("shmget"); exit(1); }
    *data = (char *)shmat(*shmid, NULL, 0);
    if (*data == (char *)-1) { perror("shmat"); exit(1); }
}

void init_semaphore(int *semid) {
    *semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (*semid < 0) { perror("semget"); exit(1); }
    union semun arg = { .val = 1 };
    semctl(*semid, 0, SETVAL, arg);
}

void lock_semaphore(int semid) {
    struct sembuf sb = {0, -1, 0};
    semop(semid, &sb, 1);
}

void unlock_semaphore(int semid) {
    struct sembuf sb = {0, 1, 0};
    semop(semid, &sb, 1);
}

void cleanup_shared_memory(int shmid) {
    shmctl(shmid, IPC_RMID, NULL);
}

void cleanup_semaphore(int semid) {
    semctl(semid, 0, IPC_RMID);
}

