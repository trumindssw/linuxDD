#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include "client.h"

#define PORT 8080
#define BUFFER_SIZE 1024

void communicate_with_server(int client_sock) {
    char buffer[BUFFER_SIZE], server_reply[BUFFER_SIZE];
    int read_size;

    while (1) {
        printf("Enter command (time/date): ");
        fgets(buffer, BUFFER_SIZE, stdin);
        send(client_sock, buffer, strlen(buffer), 0);

        if ((read_size = recv(client_sock, server_reply, sizeof(server_reply)-1, 0)) <= 0) {
            printf("Server closed connection.\n");
            break;
        }
        server_reply[read_size] = '\0';
        printf("Server: %s", server_reply);
    }
}

int main() {
    int client_sock;
    struct sockaddr_in server_address;
    struct sockaddr_in client_address;

    client_sock = socket(AF_INET, SOCK_STREAM, 0);

    server_address.sin_family = AF_INET;
    server_address.sin_port = htons(PORT);
    server_address.sin_addr.s_addr = inet_addr("127.0.0.1");

    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(50000);     // fixed local port
    client_address.sin_addr.s_addr = INADDR_ANY;  // any local IP
    // we are manually binding the address. By default, the connect call takes care of this.
    bind(client_sock, (struct sockaddr *)&client_address, sizeof(client_address));

    connect(client_sock, (struct sockaddr *)&server_address, sizeof(server_address));

    printf("Connected to server.\n");

    // printing the port, the client is using.
    socklen_t addr_len = sizeof(client_address);
    getsockname(client_sock, (struct sockaddr *)&client_address, &addr_len);
    printf("Client is using local port: %d\n", ntohs(client_address.sin_port));

    communicate_with_server(client_sock);

    close(client_sock);
    return 0;
}
