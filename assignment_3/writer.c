#include "ipc_common.h"

int main() {
    // Creating shared memory segment
    int shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget failed");
        exit(1);
    }

    // Attaching to shared memory
    char *shm = (char*) shmat(shmid, NULL, 0);
    if (shm == (char *) -1) {
        perror("shmat failed");
        exit(1);
    }

    // Creating semaphore
    int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid < 0) {
        perror("semget failed");
        exit(1);
    }

    // Initialize semaphore to 1
    semctl(semid, 0, SETVAL, 1);

    // Checking current value of semaphore
    int sem_value = semctl(semid, 0, GETVAL);
    printf("Semaphore initial value: %d\n", sem_value);

    // Check current contents of shared memory
    printf("Current contents in shared memory: \"%s\"\n", shm);
    size_t current_len = strlen(shm);
    printf("String length of shm currently: %ld\n", current_len);

    // Take semaphore lock (wait)
    sem_wait(semid);

    sem_value = semctl(semid, 0, GETVAL);
    printf("Semaphore initial value: %d\n", sem_value);

    // Input from user
    char input[MAX_INPUT];
    printf("Enter a message to write to shared memory: ");
    fgets(input, MAX_INPUT, stdin);
    input[strcspn(input, "\n")] = '\0';  // remove newline

    // Checking if enough space in shared memory
    size_t input_len = strlen(input);
    if (current_len + input_len + 2 > 100) {
        fprintf(stderr, "Shared memory full (max %d bytes)\n", 100 - 1);
        sem_signal(semid); // release lock
        shmdt(shm);
        exit(1);
    }

    // Append to shared memory
    if (current_len > 0) {
        strcat(shm, "\n");
    }
    strcat(shm, input);

    // Unlock
    sem_signal(semid);

    printf("Message written to shared memory.\n");

    // Show final semaphore state
    sem_value = semctl(semid, 0, GETVAL);
    printf("Semaphore value after write: %d\n", sem_value);

    return 0;
}
