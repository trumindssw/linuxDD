#include "header.h"

void sem_wait(int semid) {
    struct sembuf sb = {0, -1, 0};
    semop(semid, &sb, 1);
}

void sem_signal(int semid) {
    struct sembuf sb = {0, 1, 0};
    semop(semid, &sb, 1);
}

int main() {
    key_t key = ftok("log.txt", 65);
    int shmid = shmget(key, 1024, IPC_CREAT | 0666);
    int semid = semget(key, 1, IPC_CREAT | 0666);

    if (shmid == -1) {
        perror("Error in shmget");
        exit(0);
    }
    if (semid == -1) {
        perror("Error in semget");
        exit(0);
    }

    union semun {
        int val;
    } sem_val;
    sem_val.val = 1;
    semctl(semid, 0, SETVAL, sem_val);

    struct shared_data *data = (struct shared_data *)shmat(shmid, NULL, 0);
    if (data == (void *)-1) {
        perror("Error in shmat");
        exit(0);
    }

    while(1) {
        printf("Enter a text: ");
        fgets(data->text, sizeof(data->text), stdin);
        data->text[strcspn(data->text, "\n")] = '\0'; // Remove newline character

        message1(data->text);

        sem_wait(semid);

        data->flag = 1;  // Mark data as ready

        sem_signal(semid);

        if(strcmp(data->text, "exit") == 0) {
            break;
        }
    }

    shmdt(data);

    return 0;
}

