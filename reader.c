#include "ipc_common.h"

int main() {
    int shmid = shmget(SHM_KEY, SHM_SIZE, 0666);

    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);

    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    char *shm = (char *)shmat(shmid, NULL, 0);

    if (shm == (char *)-1) {
        perror("shmat failed");
        exit(1);
    }
    pid_t pid = getpid();

    //what is pid_t a datatype?

    int semval_before = semctl(semid, 0, GETVAL);
    printf("Semaphore value before locking: %d\n", semval_before);


    sem_wait(semid);


    int semval_during = semctl(semid, 0, GETVAL);
    printf("Semaphore value during process: %d\n", semval_during);


    printf("Reader: Read from shared memory: %s\n", shm);

    // Read from shmem.txt

    FILE *file = fopen("shmem.txt", "r");
    fprintf(file, "Process ID: %d Message: %s\n", pid, shm);
    fclose(file);

    sem_signal(semid);  // unlock


    int semval_after = semctl(semid, 0, GETVAL);
    printf("Semaphore value after unlock: %d\n", semval_after);


    shmdt(shm);  // Detach
    // shmctl(shmid, IPC_RMID, NULL);
    return 0;
}
