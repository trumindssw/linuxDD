// sem2_writer.c
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <unistd.h>

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678
#define SHM_SIZE 1024

union semun { int val; };

int main() {
    int shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid < 0) { perror("shmget"); exit(1); }

    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid < 0) { perror("semget"); exit(1); }

    union semun arg; arg.val = 1;
    semctl(semid, 0, SETVAL, arg);

    char *data = (char *)shmat(shmid, NULL, 0);
    if (data == (char *)-1) { perror("shmat"); exit(1); }

    struct sembuf p = {0, -1, 0}; semop(semid, &p, 1);

    strcpy(data, "Hello from Writer using Shared Memory!");
    printf("Writer: Data written to shared memory\n");

    struct sembuf v = {0, 1, 0}; semop(semid, &v, 1);

    shmdt(data);
    return 0;
}

