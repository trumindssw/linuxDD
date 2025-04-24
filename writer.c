// Writer.c Acts a Client ...


#include "ipc_shared_memory.h"

int main()
{


    key_t shmkey = ftok("shmfile", 65);
    int shmid = create_shared_memory(shmkey);
    char *shm_ptr = attach_shared_memory(shmid);
    int semid = create_semaphore(SEM_KEY);

    char input[SHM_SIZE];
    printf("Enter a message to write to shared memory: ");
    fgets(input, sizeof(input), stdin);    // Read user input
    input[strcspn(input, "\n")] = '\0';    // Remove newline if any

    sem_wait(semid);
    strcpy(shm_ptr, input);                // Write input to shared memory
    sem_signal(semid);

    printf("Writer wrote: %s\n", shm_ptr);

    read_from_shared_memory_and_send(semid, shm_ptr);

    shmdt(shm_ptr);
    return 0;



}


