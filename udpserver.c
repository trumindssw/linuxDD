#include"header.h"

int main() {
    int sockfd;
    char buffer[SHM_SIZE];
    struct sockaddr_in recvaddr;
    socklen_t addrlen = sizeof(recvaddr);

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&recvaddr, 0, sizeof(recvaddr));
    recvaddr.sin_family = AF_INET;
    recvaddr.sin_addr.s_addr = INADDR_ANY;
    recvaddr.sin_port = htons(8080);

    if (bind(sockfd, (struct sockaddr *)&recvaddr, sizeof(recvaddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("UDP Receiver listening on port 8080...\n");

    while (1) {
        memset(buffer, 0, SHM_SIZE);
        recvfrom(sockfd, buffer, SHM_SIZE, 0,
                 (struct sockaddr *)&recvaddr, &addrlen);
        printf("Received UDP message: %s\n", buffer);

        if (strcmp(buffer, "exit") == 0)
            break;
    }

    close(sockfd);
    return 0;
}

