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
    shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666); // IPC_CREAT creates shared memory if it doesn't exist
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    // Create semaphore
    semid = semget(SEM_KEY, 1, IPC_CREAT | 0666); // IPC_CREAT creates the semaphore if it doesn't exist
    if (semid < 0) {
        perror("semget");
        exit(1);
    }

    // Initialize semaphore to 1 (available)
    union semun arg;
    arg.val = 1; // Initial value of semaphore (1 indicates resource is available)
    semctl(semid, 0, SETVAL, arg); // Set semaphore value

    // Attach to shared memory
    data = (char *)shmat(shmid, NULL, 0); // Attach the shared memory segment
    if (data == (char *)-1) {
        perror("shmat");
        exit(1);
    }

    // Lock semaphore (P operation - decrement semaphore value)
    struct sembuf p = {0, -1, 0}; // Lock semaphore to ensure exclusive access
    semop(semid, &p, 1);

    // Write to shared memory
    strcpy(data, "Message with Semaphore Sync");
    printf("Writer: Data written\n");
    sleep(30); 
    // Unlock semaphore (V operation - increment semaphore value)
    struct sembuf v = {0, 1, 0}; // Release semaphore to allow other processes
    semop(semid, &v, 1);

    // Detach shared memory
    shmdt(data);

    return 0;
}

