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

#define SERVER_PORT 5000
#define BUFSIZE 1024

int main() {
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    struct sockaddr_in server_addr = {0}, client_addr;
    char buffer[BUFSIZE];

    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(SERVER_PORT);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    bind(sock, (struct sockaddr *)&server_addr, sizeof(server_addr));
    printf("UDP Server listening on port %d\n", SERVER_PORT);

    socklen_t addr_len = sizeof(client_addr);
    int n = recvfrom(sock, buffer, BUFSIZE, 0, (struct sockaddr *)&client_addr, &addr_len);
    buffer[n] = '\0';
    printf("Received: %s\n", buffer);

    return 0;
}

