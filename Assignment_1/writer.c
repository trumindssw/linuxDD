#include "common.h"

int main() {

    // Creating the shared memory block
    int shm_id = shmget(SHM_KEY, SHM_SIZE, 0666 | IPC_CREAT);
    // Checking shared memory is created or not
    if (shm_id < 0) {
        perror("shm id not found");
        exit(EXIT_FAILURE);
    }

    
    // Attaching process to the shared memory
    char *sharedmemory = (char *)shmat(shm_id, NULL, 0);

    // creating the semaphore to lock and unlock the process

    
    sem_t *sem = sem_open(SEM_NAME, O_CREAT , 0666, 1);
    // checking semaphore is getting created or not
    if (sem == SEM_FAILED) {
        perror("error in opening sem_open");
        exit(EXIT_FAILURE);
    }
 
    // taking input from the user to write in shared memory
    char input[SHM_SIZE];
    printf("Enter message to write in shared memory -> ");
    fgets(input, SHM_SIZE, stdin);
    
    // locking the process
    sem_wait(sem);
    
    // critical section ->

    // copying it in shared memory
    strcpy((char *)sharedmemory, input);

    // writing in the shmem.txt 
    FILE *log = fopen(SHARED_FILE, "a");
    if (log) {
        fprintf(log, "Writing Process ID : %d -> %s", getpid(), input);
        fclose(log);
    }

    // unlocking the process
    sem_post(sem);

    // detaching the process from the shared memory
    shmdt(sharedmemory);

    return 0;
}

