#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <arpa/inet.h>

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678
#define SHM_SIZE 1024
#define SERVER_PORT 12345
#define SERVER_IP "127.0.0.1"

union semun {
    int val;
};

int main() {
    // Access shared memory
    int shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    // Access semaphore
    int semid = semget(SEM_KEY, 1, 0666);
    if (semid < 0) {
        perror("semget");
        exit(1);
    }

    // Attach to shared memory
    char *data = (char *)shmat(shmid, NULL, 0);
    if (data == (char *)-1) {
        perror("shmat");
        exit(1);
    }

    // Lock semaphore (P operation)
    struct sembuf p = {0, -1, 0};
    if (semop(semid, &p, 1) == -1) {
        perror("semop - lock");
        exit(1);
    }

    // Read from shared memory
    printf("Reader: Data read from shared memory: %s\n", data);

    // Set up UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    struct sockaddr_in serv_addr;
    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    // Send data via UDP
    if (sendto(sockfd, data, strlen(data) + 1, 0,
               (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) {
        perror("sendto");
    } else {
        printf("Reader: Data sent via UDP to server\n");
    }

    // Unlock semaphore (V operation)
    struct sembuf v = {0, 1, 0};
    if (semop(semid, &v, 1) == -1) {
        perror("semop - unlock");
    }

    // Cleanup
    shmdt(data);
    close(sockfd);

    return 0;
}

