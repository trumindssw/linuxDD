#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <string.h>
#include <unistd.h>
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
    int shmid, semid;
    char *data;
    int sockfd;
    struct sockaddr_in server_addr;

    // Create shared memory
    shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    // Create semaphore
    semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid < 0) {
        perror("semget");
        exit(1);
    }

    // Initialize semaphore to 1 (if first time)
    union semun arg;
    arg.val = 1;
    semctl(semid, 0, SETVAL, arg);

    // Attach shared memory
    data = (char *)shmat(shmid, NULL, 0);
    if (data == (char *)-1) {
        perror("shmat");
        exit(1);
    }

    // Set up UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    if (inet_pton(AF_INET, SERVER_IP, &server_addr.sin_addr) <= 0) {
        perror("inet_pton");
        exit(1);
    }

    // Lock semaphore (P)
    struct sembuf p = {0, -1, 0};
    semop(semid, &p, 1);

    // Write to shared memory
    strcpy(data, "Message with Semaphore Sync");
    printf("Writer: Data written to shared memory\n");

    // Send the same message via UDP
    sendto(sockfd, data, strlen(data) + 1, 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("Writer: Data sent via UDP to server\n");

    // Unlock semaphore (V)
    struct sembuf v = {0, 1, 0};
    semop(semid, &v, 1);

    // Cleanup
    shmdt(data);
    close(sockfd);

    return 0;
}

