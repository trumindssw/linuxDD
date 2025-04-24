// client.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "header.h"

void error_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE] = {0};

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) error_exit("Socket creation error");

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error_exit("Connection Failed");

    printf("Connected to server. Type 'Time' or 'Date' or Ctrl+C to quit.\n");

    while (1) {
        printf("> ");
        fgets(buffer, BUFFER_SIZE, stdin);
        buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline

        if (strlen(buffer) == 0) continue;

        send(sock, buffer, strlen(buffer), 0);
        int valread = read(sock, buffer, BUFFER_SIZE);
        buffer[valread] = '\0';

        printf("Server: %s\n", buffer);
        memset(buffer, 0, BUFFER_SIZE);
    }

    close(sock);
    return 0;
}

