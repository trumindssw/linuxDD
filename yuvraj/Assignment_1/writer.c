#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>

#include "shared.h"

int main(void)
{
    int shmid;
    void *shared_memory;
    char input[SHM_SIZE];

    shmid = shmget(SHM_KEY, SHM_SIZE, 0666 | IPC_CREAT);
    if (shmid < 0)
    {
        perror("shmget");
        exit(1);
    }

    shared_memory = shmat(shmid, NULL, 0);
    if (shared_memory == (void *)-1)
    {
        perror("shmat"); 
        exit(1);
    }

    sem_t *mutex = sem_open(MUTEX_SEM, O_CREAT, 0666, 1);
    sem_t *data = sem_open(DATA_SEM, O_CREAT, 0666, 0);
    if (mutex == SEM_FAILED || data == SEM_FAILED)
    {
        perror("sem_open");
        exit(1);
    }

    printf("Enter message: ");
    if (fgets(input, SHM_SIZE, stdin) == NULL)
    {
        printf("No input\n");
        return 0;
    }
    input[strcspn(input, "\n")] = '\0';

    sem_wait(mutex); 

    strncpy((char *)shared_memory, input, SHM_SIZE);
    ((char *)shared_memory)[SHM_SIZE - 1] = '\0';

    FILE *fp = fopen(LOG_FILE, "a");
    if (fp)
    {
        fprintf(fp, "Writer PID %d: %s\n", getpid(), input);
        fclose(fp);
    }

    sem_post(mutex); 
    sem_post(data);  

    printf("Message written to shared memory.\n");

    shmdt(shared_memory);
    sem_close(mutex);
    sem_close(data);

    return 0;
}
