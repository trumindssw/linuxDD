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
    struct sembuf sb = {0, -1, 0};//sem_num=0,sem_op=-1(wait),sem_flag=0
    semop(semid, &sb, 1);//performing semaphore operation on semid
}

void sem_signal(int semid) {
    struct sembuf sb = {0, 1, 0};//sem)num=0,sem_op=1(signal),sem_flag=0
    semop(semid, &sb, 1);//performing semaphore operation on semid
}

int main() {
    key_t key = ftok("log.txt", 65);//Generating a key for semaphore and shared memory
    int shmid = shmget(key, 1024, IPC_CREAT | 0666);//creating shared memory with 1024 bytes of memory and 666 permisssions
    int semid = semget(key, 1, IPC_CREAT | 0666);//creating semaphore set with 1 semaphore
    //error handling for shared memory
    if (shmid == -1) {
        perror("Error in shmget");
        exit(0);
    }
    //Error handling for semaphore
    if (semid == -1) {
        perror("Error in semget");
        exit(0);
    }

    union semun {//used for initializing the semaphore
        int val;
    } sem_val;
    sem_val.val = 1;//set the initial value of semaphore to 1
    semctl(semid, 0, SETVAL, sem_val);//initialize semaphore
   //Attaching the shared memory to the process's address space
    struct shared_data *data = (struct shared_data *)shmat(shmid, NULL, 0);
    if (data == (void *)-1) {
        perror("Error in shmat");
        exit(0);
    }

    while(1) {
        printf("Enter a text: ");
        fgets(data->text, sizeof(data->text), stdin);
        data->text[strcspn(data->text, "\n")] = '\0'; // Remove newline character
        printf("size of data written: %zu bytes\n",strlen(data->text));//displaying the sixe of the data
        message1(data->text);//calling the function

        sem_wait(semid);//acquire the semaphore before accessing shared resource

        data->flag = 1;  // Mark data as ready

        sem_signal(semid);//releasing the semaphore

        if(strcmp(data->text, "exit") == 0) {//exiting loop
            break;
        }
    }

    shmdt(data);//detaching the shared memory

    return 0;
}

