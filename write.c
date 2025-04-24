#include "header.h"
//Union for semaphore control
union semun{
int val;
};
int main()
{
int shmid=shmget(SHM_KEY, 1024,0666 | IPC_CREAT);//to create shared memory segment
SharedData *data=(SharedData *)shmat(shmid,NULL,0); //attach to your process
  // Create or get a semaphore (1 semaphore in the set)
    int semid = semget(SEM_KEY, 1, 0666 | IPC_CREAT);
 
    // Initialize semaphore to 1 (unlocked state)
    union semun arg;
    arg.val = 1;
    semctl(semid, 0, SETVAL, arg);
      
    // Define P (lock) operation: subtract 1, wait if already 0
    struct sembuf lock = {0, -1, 0};
 
    // Define V (unlock) operation: add 1
    struct sembuf unlock = {0, 1, 0};
 
    // Input from user
    char input[MAX_MESSAGE_LEN];
    printf("Enter a message to send: ");
    fgets(input, MAX_MESSAGE_LEN, stdin);

    input[strcspn(input, "\n")] = '\0';  // Remove newline character.
 
    // Lock semaphore (enter critical section)
    semop(semid, &lock, 1);
 
    // Write to shared memory
    strncpy(data->message, input, MAX_MESSAGE_LEN);
 
    // Unlock semaphore (leave critical section)
    semop(semid, &unlock, 1);
// Log the message to shmem.txt
         FILE *log = fopen("shmem.txt", "a");
    fprintf(log, "Process ID: %d: %s\n", getpid(), input);
    fclose(log);
 
 
 
    // Detach from shared memory
    shmdt(data);
 
    return 0;
}
