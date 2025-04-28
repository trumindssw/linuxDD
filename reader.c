#include "header.h"

void sem_wait(int semid) {
    struct sembuf sb = {0, -1, 0};
    semop(semid, &sb, 1);
}

void sem_signal(int semid) {
    struct sembuf sb = {0, 1, 0};
    semop(semid, &sb, 1);
}

int main()
{
    key_t key = ftok("log.txt",65);
    int shmid = shmget(key,1024,0666);
    int semid = semget(key,1,0666);

    if(shmid==-1) {
        perror("Shmget");
        exit(0);
    }
    if(semid==-1) {
        perror("Semget");
        exit(0);
    }

    struct shared_data *data = (struct shared_data *)shmat(shmid, NULL, 0);
    if(data == (void *)-1) {
        perror("Shmat");
        exit(0);
    }

    while(1) {
        sem_wait(semid);

        if(data->flag == 1) {
            printf("Reader read: %s\n", data->text);
            message1(data->text);
            if(strcmp(data->text, "exit") == 0)
                break;

            data->flag = 0;
        }

        sem_signal(semid);
    }

    shmdt(data);
    shmctl(shmid, IPC_RMID, NULL);
    semctl(semid, 0, IPC_RMID);

    return 0;
}

