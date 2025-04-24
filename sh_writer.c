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

    // Step 1: Create shared memory
    shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    // Step 2: Attach to shared memory
    data = (char *) shmat(shmid, NULL, 0);
    if (data == (char *) -1) {
        perror("shmat");
        exit(1);
    }

    // Step 3: Write to shared memory
    strcpy(data, "Hello from Writer Process!");
    printf("Writer wrote: %s\n", data);

    // Step 4: Detach
   // shmdt(data);

    return 0;
}

