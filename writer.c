#include "ipc_common.h"

int main() {
    int shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    // Initialize semaphore to 1
    semctl(semid, 0, SETVAL, 1);

    if (shmid == -1) {
        perror("shmget failed");
        exit(1);
    }

    char *shm = (char *)shmat(shmid, NULL, 0);
    pid_t pid = getpid();

    if (shm == (char *)-1) {
        perror("shmat failed");
        exit(1);
    }

    
    int semval_before = semctl(semid, 0, GETVAL);


    printf("Semaphore value before locking: %d\n", semval_before);


    sem_wait(semid);
    int semval_during = semctl(semid, 0, GETVAL);

    printf("Semaphore value during process: %d\n", semval_during);
    // Get user input
    char input[MAX_INPUT];
    printf("Enter a message to write to shared memory: ");
    if (fgets(input, MAX_INPUT, stdin) == NULL) {
        perror("fgets failed");
        sem_signal(semid);
        shmdt(shm);
        exit(1);
    }

    // Remove trailing newline from fgets
    input[strcspn(input, "\n")] = '\0';

    size_t current_len = strlen(shm);
    size_t input_len = strlen(input);
    if (current_len + input_len + 2 > SHM_SIZE) { // +2 for delimiter and null terminator
        fprintf(stderr, "Shared memory full (max %d bytes)\n", SHM_SIZE - 1);
        sem_signal(semid);
        shmdt(shm);
        exit(1);
    }

    // Append input to shared memory with newline delimiter
    if (current_len > 0) {
        strcat(shm, "\n"); // Add delimiter if not first message
    }
    strcat(shm, input);    
    printf("Writer: Message written to shared memory.\n");

    FILE *file = fopen("shmem.txt", "a");
    fprintf(file, "Process ID: %d Message: %s\n", pid, input);
    fclose(file);


    sem_signal(semid);  // unlock

    int semval_after = semctl(semid, 0, GETVAL);
    printf("Semaphore value after unlock: %d\n", semval_after);
    shmdt(shm);  // Detach
    return 0;
}
