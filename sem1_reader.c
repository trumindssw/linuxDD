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
    int shmid, semid;
    char *shm_data;
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char udp_buffer[SHM_SIZE];

    // Create/get shared memory
    shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    // Create/get semaphore
    semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (semid < 0) {
        perror("semget");
        exit(1);
    }

    // Attach to shared memory
    shm_data = (char *)shmat(shmid, NULL, 0);
    if (shm_data == (char *)-1) {
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
    server_addr.sin_addr.s_addr = INADDR_ANY;

    // Bind to receive UDP messages
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        perror("bind");
        close(sockfd);
        exit(1);
    }

    // Lock semaphore (P operation)
    struct sembuf p = {0, -1, 0};
    if (semop(semid, &p, 1) == -1) {
        perror("semop - lock");
        exit(1);
    }

    // Read from shared memory
    printf("Reader: Data read from shared memory: %s\n", shm_data);

    // Unlock semaphore (V operation)
    struct sembuf v = {0, 1, 0};
    if (semop(semid, &v, 1) == -1) {
        perror("semop - unlock");
    }

    // Receive data from UDP (writer)
    client_len = sizeof(client_addr);
    ssize_t recv_len = recvfrom(sockfd, udp_buffer, SHM_SIZE - 1, 0,
                                (struct sockaddr *)&client_addr, &client_len);
    if (recv_len < 0) {
        perror("recvfrom");
    } else {
        udp_buffer[recv_len] = '\0';  // Ensure null-termination
        printf("Reader: Data received from UDP: %s\n", udp_buffer);
    }

    // Cleanup
    shmdt(shm_data);
    close(sockfd);

    return 0;
}

