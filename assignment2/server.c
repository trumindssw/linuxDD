#include "common.h"

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BUFFER_SIZE];

    // Create a TCP socket (IPv4, Stream)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        perror("Socket failed");
        exit(EXIT_FAILURE);
    }

    // Configure the server address
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = inet_addr("127.0.0.1");
    address.sin_port = htons(PORT); // Convert port to network byte order

    // Bind the socket to the IP and port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    // Start listening for incoming connections
    if (listen(server_fd, 1) < 0) {
        perror("Listen failed");
        exit(EXIT_FAILURE);
    }

    printf("[SERVER] Listening on 127.0.0.1:%d...\n", PORT);

    // Accept a client connection
    new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    if (new_socket < 0) {
        perror("Accept failed");
        exit(EXIT_FAILURE);
    }

    printf("[SERVER] Client connected.\n");

    while (1) {
        fd_set readfds;
        struct timeval timeout;

        // Initialize the set and add the client socket to it
        FD_ZERO(&readfds);
        FD_SET(new_socket, &readfds);

        // Set timeout for select (idle timeout)
        timeout.tv_sec = TIMEOUT_SECONDS;
        timeout.tv_usec = 0;

        // Wait for activity on the socket
        int activity = select(new_socket + 1, &readfds, NULL, NULL, &timeout);

        if (activity == -1) {
            perror("select");
            break;
        } else if (activity == 0) {
    
            printf("[SERVER] No message received in %d seconds. Idle timeout.\n", TIMEOUT_SECONDS);
            break;
        } else {
            // Clear buffer before reading
            memset(buffer, 0, BUFFER_SIZE);

            // Read the message from the client
            int valread = read(new_socket, buffer, BUFFER_SIZE - 1);
            if (valread <= 0) {
                printf("[SERVER] Client disconnected.\n");
                break;
            }

            printf("[SERVER] Received: %s\n", buffer);

            
            if (strncmp(buffer, "time", 4) == 0) {
                time_t t = time(NULL);
                struct tm *tm_info = localtime(&t);
                char time_str[64];
                strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);
                send(new_socket, time_str, strlen(time_str), 0);
            }
        
            else if (strncmp(buffer, "date", 4) == 0) {
                time_t t = time(NULL);
                struct tm *tm_info = localtime(&t);
                char date_str[64];
                strftime(date_str, sizeof(date_str), "%Y-%m-%d", tm_info);
                send(new_socket, date_str, strlen(date_str), 0);
            }
    
            else {
                char *msg = "Unknown command";
                send(new_socket, msg, strlen(msg), 0);
            }
        }
    }

    // Close client and server sockets
    close(new_socket);
    close(server_fd);
    return 0;
}
