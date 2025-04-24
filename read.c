#include "header.h"
int main() {
    int shmid = shmget(SHM_KEY, sizeof(SharedData), 0666);
 
    SharedData *data = (SharedData *)shmat(shmid, NULL, 0);
 
    int semid = semget(SEM_KEY, 1, 0666);
 
    // Define P (lock) operation
    struct sembuf lock = {0, -1, 0};
 
    // Define V (unlock) operation
    struct sembuf unlock = {0, 1, 0};
 
    semop(semid, &lock, 1);
 
    char received[MAX_MESSAGE_LEN];
    strncpy(received, data->message, MAX_MESSAGE_LEN);
 
    semop(semid, &unlock, 1);
 
    printf("Received: %s\n", received);
 
    // Log to the shared log file
    FILE *ptr = fopen("shmem.txt", "a");
    fprintf(ptr, "Process ID: %d: %s\n", getpid(), received);
    fclose(ptr);
 
    // Detach from shared memory
    shmdt(data);
    shmctl(shmid,IPC_RMID,NULL);
    semctl(semid,0,IPC_RMID);
 
    return 0;
}
 
