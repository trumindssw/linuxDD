#include "ipc_shared_memory.h"

// Create shared memory segment
int create_shared_memory(key_t key)
{
    int shmid = shmget(key, SHM_SIZE, IPC_CREAT | 0666);
    if (shmid == -1)
    {
        perror("shmget failed");
        exit(1);
    }
    return shmid;
}

// Attach shared memory to process
char* attach_shared_memory(int shmid)
{
    char *shm_ptr = (char*) shmat(shmid, NULL, 0);
    if (shm_ptr == (char*) -1)
    {
        perror("shmat failed");
        exit(1);
    }
    return shm_ptr;
}

// Create semaphore
int create_semaphore(int sem_key)
{
    int semid = semget(sem_key, 1, IPC_CREAT | 0666);
    if (semid == -1)
    {
        perror("semget failed");
        exit(1);
    }

    // Initialize semaphore to 1 if newly created
    union semun arg;
    arg.val = 1;
    if (semctl(semid, 0, SETVAL, arg) == -1)
    {
        perror("semctl failed");
        exit(1);
    }

    return semid;
}

// Wait operation (P)
void sem_wait(int semid)
{
    struct sembuf sb = {0, -1, 0};
    if (semop(semid, &sb, 1) == -1)
    {
        perror("semop wait failed");
        exit(1);
    }
}

// Signal operation (V)
void sem_signal(int semid)
{
    struct sembuf sb = {0, 1, 0};
    if (semop(semid, &sb, 1) == -1)
    {
        perror("semop signal failed");
        exit(1);
    }
}

// Send data to UDP server
void send_data_to_server(char* data, const char* server_ip)
{
    int sockfd;
    struct sockaddr_in servaddr;

    // Create socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("socket creation failed");
        exit(1);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // Set server info
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(UDP_PORT);
    servaddr.sin_addr.s_addr = inet_addr(server_ip);

    // Send message to server
    sendto(sockfd, data, strlen(data), 0, (const struct sockaddr*)&servaddr, sizeof(servaddr));
    close(sockfd);
}

// Read from shared memory and send to UDP server
void read_from_shared_memory_and_send(int semid, char* shm_ptr)
{
    sem_wait(semid);
    printf("Data read from shared memory: %s\n", shm_ptr);
    sem_signal(semid);

    send_data_to_server(shm_ptr, "127.0.0.1");  // Localhost server IP
}

