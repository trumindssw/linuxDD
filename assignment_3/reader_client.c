#include "ipc_common.h"

int main() {
    // Attaching to existing shared memory segment
    int shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shmid < 0) {
        perror("shmget failed");
        exit(1);
    }

    char *shm = (char*) shmat(shmid, NULL, 0);
    if (shm == (char *) -1) {
        perror("shmat failed");
        exit(1);
    }

    // Get existing semaphore
    int semid = semget(SEM_KEY, 1, 0666);
    if (semid < 0) {
        perror("semget failed");
        exit(1);
    }


    // Wait (lock), then copy data
    sem_wait(semid);
    char message[100];

    int sem_value = semctl(semid, 0, GETVAL);
    printf("Semaphore value in lock: %d\n", sem_value);
    strcpy(message, shm);

    // Unlock the semaphore
    sem_signal(semid);
    sem_value = semctl(semid, 0, GETVAL);
    printf("Semaphore value after unlock: %d\n", sem_value);

    printf("Total content in the shared memory: \"%s\"\n", message);
    char* lastline = strrchr(shm, '\n');
    if (lastline != NULL){
    lastline++;
    printf("Last message in shared memory: %s\n", lastline);
    }
    else{
        printf("Last message in shared memory: %s\n", shm);
    }



    // Create UDP socket
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket creation failed");
        exit(1);
    }

    // Server address
    struct sockaddr_in serv_addr;
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(9000);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    // Send message 
    sendto(sock, message, strlen(message), 0,
           (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    // Client port
    struct sockaddr_in local_addr;
    socklen_t addr_len = sizeof(local_addr);
    getsockname(sock, (struct sockaddr *)&local_addr, &addr_len);
    printf("Client socket is using local port: %d\n", ntohs(local_addr.sin_port));

    printf("Sent via UDP to server on port 9000.\n");

    close(sock);
    return 0;
}
