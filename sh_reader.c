#include <stdio.h>

#include <stdlib.h>

#include <sys/ipc.h>

#include <sys/shm.h>

#include <string.h>

#include <unistd.h>
 
#define SHM_KEY 0x1234

#define SHM_SIZE 1024
 
int main() {

    int shmid;

    char *data;
 
    // Step 1: Access existing shared memory

    shmid = shmget(SHM_KEY, SHM_SIZE, 0666);

    if (shmid < 0) {

        perror("shmget");

        exit(1);

    }
 
    // Step 2: Attach

    data = (char *) shmat(shmid, NULL, 0);

    if (data == (char *) -1) {

        perror("shmat");

        exit(1);

    }
 
    // Step 3: Read data

    printf("Reader read: %s\n", data);
 
    // Step 4: Detach

   // shmdt(data);
 
    // Step 5 (Optional): Remove shared memory

   // shmctl(shmid, IPC_RMID, NULL);
 
    return 0;

}
 
 
