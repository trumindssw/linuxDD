
#include "ipc_shared_memory.h"

int main()
{

    key_t shmkey = ftok("shmfile", 65);           // Create a unique key for shared memory
    int shmid = create_shared_memory(shmkey);      // Create shared memory segment
    char *shm_ptr = attach_shared_memory(shmid);   // Attach shared memory to process

    // Create semaphore
    int semid = create_semaphore(SEM_KEY);

    // Wait and read data from shared memory
    sem_wait(semid);  // Wait for semaphore (synchronize)
    printf("Reader read from shared memory: %s\n", shm_ptr);  // Read from shared memory
    sem_signal(semid);  // Release semaphore

    shmdt(shm_ptr);  // Detach shared memory
    return 0;
    
}

