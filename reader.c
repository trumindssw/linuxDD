/*Team-E_Assignment2_version_0
 * IPC Code - shmem with Sockets (with MAKEIFLE, C, .h files ONLY)
1. Create a shared memory segment between two processes.
2. Use semaphores to synchronize access (to avoid race conditions).
3. One process writes a string into shared memory, the other reads it.
4. All ALL TIMES, how do you chek externally what is IN THE SH MEM
5. Same as ABOVE, except that add Sockets to the code also - Client and Server (UDP Sockets ONLY)
6. when the 2nd process reads the data written in mmory, put it on the SOCKET, so that the server gets the mesage
*/



#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "shm.h"

#define SERVER_PORT 5000

int main() {
    int shmid, semid;
    char *data;
    init_shared_memory(&shmid, &data);
    init_semaphore(&semid);

    lock_semaphore(semid);
    char message[SHM_SIZE];
    strncpy(message, data, SHM_SIZE);
    unlock_semaphore(semid);

    printf("Reader read: %s\n", message);

    // UDP client
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server_addr = {0};
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    sendto(sock, message, strlen(message), 0, (struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("Message sent to UDP server.\n");

    return 0;
}

