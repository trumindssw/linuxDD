#include <stdio.h>

#include <stdlib.h>

#include <sys/ipc.h>

#include <sys/shm.h>

#include <sys/sem.h>

#include <string.h>

#include <unistd.h>
 
#define SHM_KEY 0x1234  // Same key as used in writer

#define SEM_KEY 0x5678  // Same key as used in writer

#define SHM_SIZE 1024   // Size of shared memory
 
// Union for semaphore control operations

union semun {

    int val;

};
 
int main() {

    int shmid, semid;

    char *data;
 
    // Access shared memory

    shmid = shmget(SHM_KEY, SHM_SIZE, 0666); // Access shared memory created by writer

    if (shmid < 0) {

        perror("shmget");

        exit(1);

    }
 
    // Access semaphore

    semid = semget(SEM_KEY, 1, 0666); // Access semaphore created by writer

    if (semid < 0) {

        perror("semget");

        exit(1);

    }
 
    // Attach to shared memory

    data = (char *)shmat(shmid, NULL, 0); // Attach shared memory to reader's address space

    if (data == (char *)-1) {

        perror("shmat");

        exit(1);

    }
    // Access semaphore

    semid = semget(SEM_KEY, 1, 0666); // Access semaphore created by writer

    if (semid < 0) {

        perror("semget");

        exit(1);

    }
 
    // Attach to shared memory

    data = (char *)shmat(shmid, NULL, 0); // Attach shared memory to reader's address space

    if (data == (char *)-1) {

        perror("shmat");

        exit(1);

    }
 
 
    // Lock semaphore (P operation - decrement semaphore value)

    struct sembuf p = {0, -1, 0}; // Lock semaphore to ensure exclusive access

    semop(semid, &p, 1);
 
    // Read data from shared memory

    printf("Reader: Data read from shared memory: %s\n", data);

    // Unlock semaphore (V operation - increment semaphore value)

    struct sembuf v = {0, 1, 0}; // Release semaphore to allow other processes

    semop(semid, &v, 1);
 
    // Detach shared memory

    shmdt(data);
 
    return 0;

}
 
                                                      
 
                                                                                                                                                            
 
 
