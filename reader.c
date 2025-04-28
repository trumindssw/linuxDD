#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "common.h"
#include <unistd.h>

void sem_lock(int semid) {
    struct sembuf p = {0, -1, SEM_UNDO};
    semop(semid, &p, 1);
}

void sem_unlock(int semid) {
    struct sembuf v = {0, 1, SEM_UNDO};
    semop(semid, &v, 1);
}

int main() {
    int shmid, semid, sockfd;
    char *shm_ptr;
    struct sockaddr_in servaddr;

    // Access shared memory
    shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shmid < 0) {
        perror("shmget");
        exit(1);
    }

    shm_ptr = (char *)shmat(shmid, NULL, 0);
    if (shm_ptr == (char *)-1) {
        perror("shmat");
        exit(1);
    }

    // Access semaphore
    semid = semget(SEM_KEY, 1, 0666);
    if (semid < 0) {
        perror("semget");
        exit(1);
    }

    // Create UDP socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("socket");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(UDP_PORT);
    servaddr.sin_addr.s_addr = inet_addr("127.0.0.1");

    while (1) {
        sleep(1); // Polling every 1 second

        sem_lock(semid);
        if (strlen(shm_ptr) > 0) {
            printf("Read from shared memory: %s\n", shm_ptr);
            
            sendto(sockfd, shm_ptr, strlen(shm_ptr), 0,
                   (struct sockaddr *)&servaddr, sizeof(servaddr));

            memset(shm_ptr, 0, SHM_SIZE); // Clear memory after sending
        }
        sem_unlock(semid);
    }

    return 0;
}

