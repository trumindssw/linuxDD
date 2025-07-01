#include "shared.h"

int main() {
    int shmid;
    void *shared_memory;

    shmid = shmget(SHM_KEY, SHM_SIZE, 0666);

    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    shared_memory = shmat(shmid, NULL, 0);
    char *message = (char *)shared_memory;

    sem_t *sem = sem_open(SEM_NAME, 0);

    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    sem_wait(sem);
    

    FILE *fp = fopen(LOG_FILE, "a");
    if (fp) {
        fprintf(fp, "Process ID: %d: %s", getpid(), message);
        fclose(fp);
    }

    sem_post(sem);

    printf("Read message: %s\n", message);

    shmdt(shared_memory);
    sem_close(sem);
    shmctl(shmid,IPC_RMID,NULL);

    return 0;
}
