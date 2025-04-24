#include "myheader.h"

int main()
{
    
    int shmid = shmget(SHM_key, SIZE, 0666);
    if (shmid == -1)
    {
        perror("shmget");
        exit(1);
    }
    struct Data *data = (struct Data *)shmat(shmid, NULL, 0);
    if (data == (void *)-1)
    {
        perror("shmat");
        exit(1);
    }
    int semid = semget(SEM_key, 1, 0666);
    if (semid == -1)
    {
        perror("semget");
        exit(1);
    }
    lock_semaphore(semid);
    printf("Message received: %s\n", data->msg);
    message1(data->msg);
    shmdt(data);
    return 0;
}

