#include "common.h"

int main() {

    // getting the same shared memory block with the help of same key which writer created it
    int shm_id = shmget(SHM_KEY, SHM_SIZE, 0666) ;
    // checking we got the shared memory block or not
    if (shm_id < 0) {
        perror("shm_id not found");
        exit(EXIT_FAILURE);
    }

    // pointing to the shared memory to read from it
    char *sharedmemory = (char *)shmat(shm_id, NULL, 0);
    // checking we got the shared memory or not
    if (sharedmemory == (char *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }

    // getting the semaphore
    sem_t *sem = sem_open(SEM_NAME, 0);
    if (sem == SEM_FAILED) {
        perror("error in opening sem_open");
        exit(EXIT_FAILURE);
    }
  
    // locking the process
    sem_wait(sem);

    // critical section ->
    FILE *log = fopen(SHARED_FILE, "a");
    if (log) {
        // printing in shmem.txt
        fprintf(log, "Reading Process ID : %d -> %s", getpid(), sharedmemory);
        fclose(log);
    }
    // reading from the shared memory
    printf("Message received from writer ->  %s", sharedmemory);

    // unlocking the process
    sem_post(sem);

    // detaching from the shared memory block
    shmdt(sharedmemory);
    // deleting the mem block and the semaphore
    shmctl(shm_id, IPC_RMID, NULL);
    sem_unlink("/my_phore");

    return 0;
}

