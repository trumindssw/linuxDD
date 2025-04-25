// server.c

#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <unistd.h>

#include <arpa/inet.h>
 
#define SERVER_PORT 12345

#define BUFFER_SIZE 1024
 
int main() {

    int sockfd;

    char buffer[BUFFER_SIZE];

    struct sockaddr_in server_addr, client_addr;

    socklen_t client_len = sizeof(client_addr);
 
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd < 0) { perror("socket"); exit(1); }
 
    server_addr.sin_family = AF_INET;

    server_addr.sin_addr.s_addr = INADDR_ANY;

    server_addr.sin_port = htons(SERVER_PORT);
 
    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {

        perror("bind");

        exit(1);

    }
 
    printf("Server: Waiting for message...\n");

    recvfrom(sockfd, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&client_addr, &client_len);

    printf("Server: Received message: %s\n", buffer);
 
    close(sockfd);

    return 0;

}
 
 
