#include "common.h"


int main() {

    int shm_id = shmget(SHM_KEY, SHM_SIZE, 0666) ;

    if (shm_id < 0) {
        perror("shm_id not found");
        exit(EXIT_FAILURE);
    }


    char *sharedmemory = (char *)shmat(shm_id, NULL, 0);
    
    if (sharedmemory == (char *)-1) {
        perror("shmat");
        exit(EXIT_FAILURE);
    }


    sem_t *sem = sem_open(SEM_NAME, 0);
    if (sem == SEM_FAILED) {
        perror("error in opening sem_open");
        exit(EXIT_FAILURE);
    }
  

    sem_wait(sem);

    FILE *log = fopen(SHARED_FILE, "a");
    if (log) {
        fprintf(log, "Reading Process ID : %d -> %s", getpid(), sharedmemory);
        fclose(log);
    }

    printf("Message received from writer ->  %s", sharedmemory);

    sem_post(sem);
    shmdt(sharedmemory);
    shmctl(shm_id, IPC_RMID, NULL);

    return 0;
}

