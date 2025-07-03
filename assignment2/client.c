#include "common.h"

int main() {
    int sock = 0;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];

    // Create a TCP socket
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket creation failed");
        return -1;
    }

    // Set up server address (IP + Port)
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);         

    // Convert "127.0.0.1" to binary and store in serv_addr
    inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr);

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("Connection Failed");
        return -1;
    }

    printf("[CLIENT] Connected to server.\n");

    while (1) {
        printf("[CLIENT] Enter command (time/date/exit): ");
        fgets(buffer, BUFFER_SIZE, stdin);

        buffer[strcspn(buffer, "\n")] = '\0';

        if (strcmp(buffer, "exit") == 0) {
            printf("[CLIENT] Exiting...\n");
            break;
        }

        send(sock, buffer, strlen(buffer), 0);

        memset(buffer, 0, BUFFER_SIZE);

        int valread = read(sock, buffer, BUFFER_SIZE - 1);
        if (valread <= 0) {
            printf("[CLIENT] Server closed connection.\n");
            break;
        }

        printf("[CLIENT] Server reply: %s\n", buffer);
    }

    close(sock);
    return 0;
}
