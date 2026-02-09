#include "shared.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <errno.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/select.h>
#include <errno.h>

int main()
{
    int server_fd, client_fd;
    struct sockaddr_in address;
    socklen_t addrlen = sizeof(address);
    char buffer[BUF_SIZE];

    // 1. Create TCP socket
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    {
        int opt = 1;
        if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        {
            perror("setsockopt SO_REUSEADDR");
        }
    }

    signal(SIGPIPE, SIG_IGN);

    // 2. Set server address
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;

    if (inet_pton(AF_INET, "127.0.0.1", &address.sin_addr) != 1)
    {
        fprintf(stderr, "inet_pton failed\n");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    address.sin_port = htons(PORT);

    // 3. Bind socket to IP + PORT
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // 4. Start listening
    if (listen(server_fd, 1) < 0)
    {
        perror("Listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[SERVER] Listening on Port:%d...\n", PORT);

    // 5. Accept client
    client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    if (client_fd < 0)
    {
        perror("Accept failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("[SERVER] Client connected.\n");

    while (1)
    {
        fd_set readfds;
        struct timeval timeout;

        FD_ZERO(&readfds);
        FD_SET(client_fd, &readfds);

        timeout.tv_sec = TIMEOUT_SECONDS;
        timeout.tv_usec = 0;

        int activity = select(client_fd + 1, &readfds, NULL, NULL, &timeout);

        if (activity == -1)
        {
            if (errno == EINTR)
            {

                continue;
            }
            perror("select");
            break;
        }
        else if (activity == 0)
        {
            printf("[SERVER] No message for %d seconds. Idle timeout.\n",
                   TIMEOUT_SECONDS);
            break; 
        }

        // Client sent data
        memset(buffer, 0, BUF_SIZE);
        ssize_t valread = recv(client_fd, buffer, BUF_SIZE - 1, 0);

        if (valread <= 0)
        {
            if (valread == 0)
            {
                printf("[SERVER] Client disconnected.\n");
            }
            else
            {
                perror("recv");
            }
            break;
        }

        buffer[valread] = '\0';

        printf("SERVER Received: %s\n", buffer);

        if (strncasecmp(buffer, "time", 4) == 0)
        {
            time_t t = time(NULL);
            struct tm tm_info;
            localtime_r(&t, &tm_info);
            char timestr[64];
            strftime(timestr, sizeof(timestr), "%H:%M:%S", &tm_info);
            ssize_t sent = send(client_fd, timestr, strlen(timestr), 0);
            if (sent < 0)
                perror("send time");
        }
        else if (strncasecmp(buffer, "date", 4) == 0)
        {
            time_t t = time(NULL);
            struct tm tm_info;
            localtime_r(&t, &tm_info);
            char datestr[64];
            strftime(datestr, sizeof(datestr), "%Y-%m-%d", &tm_info);
            ssize_t sent = send(client_fd, datestr, strlen(datestr), 0);
            if (sent < 0)
                perror("send date");
        }
        else if (strncasecmp(buffer, "quit", 4) == 0)
        {
            const char *bye = "Goodbye\n";
            ssize_t sent = send(client_fd, bye, strlen(bye), 0);
            if (sent < 0)
                perror("send quit");
            printf("SERVER Received quit command.\n Shutting down gracefully.\n");
            break;
        }
        else
        {
            const char *msg = "Unknown command";
            ssize_t sent = send(client_fd, msg, strlen(msg), 0);
            if (sent < 0)
                perror("send random cmd");
        }
    }

    close(client_fd);
    close(server_fd);
    printf("[SERVER] Shutdown complete.\n");
    return 0;
}
