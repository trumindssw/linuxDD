// sem1_reader.c
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

union semun { int val; };

int main() {
    int shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shmid < 0) { perror("shmget"); exit(1); }

    int semid = semget(SEM_KEY, 1, 0666);
    if (semid < 0) { perror("semget"); exit(1); }

    char *data = (char *)shmat(shmid, NULL, 0);
    if (data == (char *)-1) { perror("shmat"); exit(1); }

    struct sembuf p = {0, -1, 0}; semop(semid, &p, 1);

    printf("Reader: Data read from shared memory: %s\n", data);

    // UDP socket
    int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) { perror("socket"); exit(1); }

    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(SERVER_PORT);
    serv_addr.sin_addr.s_addr = inet_addr(SERVER_IP);

    sendto(sockfd, data, strlen(data)+1, 0, (struct sockaddr*)&serv_addr, sizeof(serv_addr));
    printf("Reader: Data sent via UDP to server\n");

    struct sembuf v = {0, 1, 0}; semop(semid, &v, 1);

    shmdt(data);
    close(sockfd);
    return 0;
}

