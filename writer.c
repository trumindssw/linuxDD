/*Team-E_Assignment2_version_0
 * IPC Code - shmem with Sockets (with MAKEIFLE, C, .h files ONLY)
1. Create a shared memory segment between two processes.
2. Use semaphores to synchronize access (to avoid race conditions).
3. One process writes a string into shared memory, the other reads it.
4. All ALL TIMES, how do you chek externally what is IN THE SH MEM
5. Same as ABOVE, except that add Sockets to the code also - Client and Server (UDP Sockets ONLY)
6. when the 2nd process reads the data written in mmory, put it on the SOCKET, so that the server gets the mesage
*/



#include <stdio.h>
#include <string.h>
#include "shm.h"

int main() {
    int shmid, semid;
    char *data;
    init_shared_memory(&shmid, &data);
    init_semaphore(&semid);

    char input[SHM_SIZE];
    printf("Enter message: ");
    fgets(input, SHM_SIZE, stdin);

    lock_semaphore(semid);
    strncpy(data, input, SHM_SIZE);
    unlock_semaphore(semid);

    printf("Message written to shared memory.\n");
    return 0;
}

