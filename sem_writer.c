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

// Union for semaphore control operations
union semun {
    int val;
};

int main() {
    int shmid, semid;
    char *data;

    // Create shared memory
    shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    // Create semaphore
    semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid < 0) {
        perror("semget");
        exit(1);
    }

    // Initialize semaphore to 1 (available)
    union semun arg;
    arg.val = 1;
    semctl(semid, 0, SETVAL, arg);

    // Attach to shared memory
    data = (char *)shmat(shmid, NULL, 0);
    if (data == (char *)-1) {
        perror("shmat");
        exit(1);
    }

    // Lock semaphore (P operation)
    struct sembuf p = {0, -1, 0};
    semop(semid, &p, 1);

    // Write to shared memory
    strcpy(data, "Message with Semaphore Sync");
    printf("Writer: Data written\n");

    sleep(30); // Simulate long write operation

    // Unlock semaphore (V operation)
    struct sembuf v = {0, 1, 0};
    semop(semid, &v, 1);

    // Detach shared memory
    shmdt(data);

    return 0;
}

