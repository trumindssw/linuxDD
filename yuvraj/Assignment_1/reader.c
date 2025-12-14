#include <stdio.h>
#include <stdlib.h>
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

    char *message = (char *)shared_memory;

    sem_t *mutex = sem_open(MUTEX_SEM, O_CREAT, 0666, 1);
    sem_t *data = sem_open(DATA_SEM, O_CREAT, 0666, 0);
    if (mutex == SEM_FAILED || data == SEM_FAILED)
    {
        perror("sem_open");
        exit(1);
    }

    printf("Reader [ pid -> %d ] waiting ...\n", getpid());


    sem_wait(data); 

    sem_wait(mutex); 
    printf("Reader got -> %s\n", message);

    FILE *fp = fopen(LOG_FILE, "a");
    if (fp)
    {
        fprintf(fp, "Reader PID %d: %s\n", getpid(), message);
        fclose(fp);
    }

    sem_post(mutex); 

    shmdt(shared_memory);
    sem_close(mutex);
    sem_close(data);

    return 0;
}
