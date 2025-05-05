//Team_A_Assignment3_V0.1  
/*-----------------------------------------------------------------------------------------
Assignment Three
IPC Code - shmem with Sockets (with MAKEIFLE, C, .h files ONLY)
1. Create a shared memory segment between two processes.
2. Use semaphores to synchronize access (to avoid race conditions).
3. One process writes a string into shared memory, the other reads it.
4. All ALL TIMES, how do you chek externally what is IN THE SH MEM
5. Same as ABOVE, except that add Sockets to the code also - Client and Server (UDP Sockets ONLY)
6. when the 2nd process reads the data written in mmory, put it on the SOCKET, so that the server gets the mesage
---------------------------------------------------------------------------------------------------------------------*/

#include "header.h"  

void sem_wait(int semid) {
    struct sembuf sb = {0, -1, 0};   // Semaphore buffer: sem_num=0, sem_op=-1, sem_flg=0
    semop(semid, &sb, 1);            // Perform the wait (decrement) operation
}

void sem_signal(int semid) {
    struct sembuf sb = {0, 1, 0};    // Semaphore buffer: sem_num=0, sem_op=1, sem_flg=0
    semop(semid, &sb, 1);            // Perform the signal (increment) operation
}

int main() {
                                    
     key_t key = ftok("log.txt", 65);  // Generate a unique key for shared memory and semaphore using a file and ID


   int shmid = shmget(key, 1024, IPC_CREAT | 0666); // Create shared memory segment of size 1024 bytes with read-write permissions
  

   int semid = semget(key, 1, IPC_CREAT | 0666); // Create a semaphore set with 1 semaphore
   

    // Checking for errors in shared memory creation
    if (shmid == -1) {
        perror("Error in shmget");
        exit(0);
    }

    // Checking for errors in semaphore creation
    if (semid == -1) {
        perror("Error in semget");
        exit(0);
    }

    // Define union for semaphore initialization
    union semun {
        int val;
    } sem_val;

    sem_val.val = 1;  // Initialize semaphore value to 1 (binary semaphore)
    semctl(semid, 0, SETVAL, sem_val);  // Set semaphore value

    // Attach to shared memory
    struct shared_data *data = (struct shared_data *)shmat(shmid, NULL, 0);
    if (data == (void *)-1) {
        perror("Error in shmat");
        exit(0);
    }

    // Main loop for writing to shared memory
    while (1) {
        printf("Enter a text: ");
        fgets(data->text, sizeof(data->text), stdin);                 // Read user input
        data->text[strcspn(data->text, "\n")] = '\0';                 // Remove newline character

        message1(data->text);                                         // Log message to file with PID

        sem_wait(semid);                                              // Lock semaphore

        data->flag = 1;                                               // Indicate data is ready for reader

        sem_signal(semid);                                            // Unlock semaphore

        if (strcmp(data->text, "exit") == 0) {
            break;                                                    // Exit loop on "exit" input
        }
    }

    // Detach from shared memory
    shmdt(data);

    return 0;
}

