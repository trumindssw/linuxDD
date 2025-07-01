#include "common.h"


int main() {

    int shm_id = shmget(SHM_KEY, SHM_SIZE, 0666 | IPC_CREAT);
    
    if (shm_id < 0) {
        perror("shm id not found");
        exit(EXIT_FAILURE);
    }

    char *sharedmemory = (char *)shmat(shm_id, NULL, 0);

    sem_t *sem = sem_open(SEM_NAME, O_CREAT , 0666, 1);
    if (sem == SEM_FAILED) {
        perror("error in opening sem_open");
        exit(EXIT_FAILURE);
    }
 

    char input[SHM_SIZE];
    printf("Enter message to write in shared memory -> ");
    fgets(input, SHM_SIZE, stdin);

    strcpy((char *)sharedmemory, input);

    sem_wait(sem);


    FILE *log = fopen(SHARED_FILE, "a");
    if (log) {
        fprintf(log, "Writing Process ID : %d -> %s", getpid(), input);
        fclose(log);
    }


    sem_post(sem);

    shmdt(sharedmemory);

    return 0;
}


