//Team_A_Assignment1_V0.1
/* Assignment One
IPC Code - shared memory (with MAKEIFLE, C, .h files ONLY)
 
1. Create a shared memory segment between two processes.
 
2. Use semaphores to synchronize access (to avoid race conditions).
 
3. One process writes a string into shared memory, the other reads it.
 
4. At ALL TIMES, how do you chek externally what is IN THE SH MEM
 
4A. Create a common file called "shmem.txt"
 
5. Open the file "shmem.txt" and keep writing "Process ID: $PID: $Message"
 
6. basically, each process writes its Process ID and then the message sent by the other process in the SAME FILE
 
// Use "strace" in the background to see how the processes are working
---------------------------------------------------------------------------*/

#include "header.h"  

void sem_wait(int semid) {
    struct sembuf sb = {0, -1, 0};  // Operate on semaphore 0, decrement by 1
    semop(semid, &sb, 1);           // Perform the operation
}

// Semaphore signal (V operation): Increments the semaphore value.
// If other processes are waiting, one is unblocked.
void sem_signal(int semid) {
    struct sembuf sb = {0, 1, 0};   // Operate on semaphore 0, increment by 1
    semop(semid, &sb, 1);           // Perform the operation
}

int main()
{
    // Generate a unique key using the file "log.txt" and project id 65
    key_t key = ftok("log.txt", 65);

    // Get the shared memory segment ID
    int shmid = shmget(key, 1024, 0666);

    // Get the semaphore ID
    int semid = semget(key, 1, 0666);

    // Check if shared memory allocation failed
    if (shmid == -1) {
        perror("Shmget");
        exit(0);
    }

    // Check if semaphore allocation failed
    if (semid == -1) {
        perror("Semget");
        exit(0);
    }

    // Attach the shared memory segment to the process address space
    struct shared_data *data = (struct shared_data *)shmat(shmid, NULL, 0);
    if (data == (void *)-1) {
        perror("Shmat");
        exit(0);
    }

    // Start reading loop
    while (1) {
        sem_wait(semid);  // Acquire lock

        if (data->flag == 1) {  // Check if data is ready to be read
            printf("Size of data read: %zu bytes\n", strlen(data->text));
            printf("Reader read: %s\n", data->text);
            message1(data->text);  // Optional processing function

            // If received message is "exit", break the loop
            if (strcmp(data->text, "exit") == 0)
                break;

            data->flag = 0;  // Reset flag to indicate that  data is consumed
        }

        sem_signal(semid);  // Release the lock
    }

    // Detach shared memory from process
    shmdt(data);

    // Removing the  shared memory and semaphore resources
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);

    return 0;
}

