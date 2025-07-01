#include "shared.h"

int main() {
    int shmid;
    void *shared_memory;
    char input[SHM_SIZE];

    shmid = shmget(SHM_KEY, SHM_SIZE, 0666 | IPC_CREAT);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    shared_memory = shmat(shmid, NULL, 0);

    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) {
        perror("sem_open");
        exit(1);
    }

    printf("Enter message: ");
    fgets(input, SHM_SIZE, stdin);
    strcpy((char *)shared_memory, input);

    sem_wait(sem);

    FILE *fp = fopen(LOG_FILE, "a");
    if (fp) {
        fprintf(fp, "Process ID: %d: %s", getpid(), input);
        fclose(fp);
    }

    sem_post(sem);

    printf("Message written to shared memory.\n");

    shmdt(shared_memory);
    sem_close(sem);

    return 0;
}
