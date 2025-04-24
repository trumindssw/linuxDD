// server.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netinet/in.h>
#include <time.h>
#include <sys/socket.h>
#include <sys/select.h>
#include "header.h"

void error_exit(const char *msg) {
    perror(msg);
    exit(EXIT_FAILURE);
}

void send_time(int client_socket) {
    time_t now = time(NULL);
    char buffer[BUFFER_SIZE];
    strftime(buffer, sizeof(buffer), "%H:%M:%S", localtime(&now));
    send(client_socket, buffer, strlen(buffer), 0);
}

void send_date(int client_socket) {
    time_t now = time(NULL);
    char buffer[BUFFER_SIZE];
    strftime(buffer, sizeof(buffer), "%Y-%m-%d", localtime(&now));
    send(client_socket, buffer, strlen(buffer), 0);
}

int main() {
    int server_fd, client_socket;
    struct sockaddr_in address;
    socklen_t addr_len = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == 0) error_exit("Socket failed");

    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
        error_exit("Bind failed");

    if (listen(server_fd, 3) < 0) error_exit("Listen failed");

    printf("Server listening on port %d...\n", PORT);
    client_socket = accept(server_fd, (struct sockaddr *)&address, &addr_len);
    if (client_socket < 0) error_exit("Accept failed");

    fd_set readfds;
    struct timeval timeout;

    while (1) {
        FD_ZERO(&readfds);
        FD_SET(client_socket, &readfds);
        timeout.tv_sec = TIMEOUT_SECS;
        timeout.tv_usec = 0;

        int activity = select(client_socket + 1, &readfds, NULL, NULL, &timeout);

        if (activity == 0) {
            printf("No message from client in %d sec. Will shutdown in 2 min.\n", TIMEOUT_SECS);
            sleep(120);
            printf("Graceful shutdown.\n");
            break;
        }

        int valread = read(client_socket, buffer, BUFFER_SIZE);
        if (valread <= 0) {
            printf("Client disconnected.\n");
            break;
        }

        buffer[valread] = '\0';
        printf("Received: %s\n", buffer);

        if (strcasecmp(buffer, "time") == 0)
            send_time(client_socket);
        else if (strcasecmp(buffer, "date") == 0)
            send_date(client_socket);
        else
            send(client_socket, "Unknown command", 15, 0);

        memset(buffer, 0, BUFFER_SIZE);
    }

    close(client_socket);
    close(server_fd);
    return 0;
}

