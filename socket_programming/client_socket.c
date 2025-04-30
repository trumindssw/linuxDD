
#include "headers.h"

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Usage: %s <port> <server_ip>\n", argv[0]);
        return 1;
    }

    int sockfd;
    struct sockaddr_in server_addr;
    char buff[100];

    // Create socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Socket creation failed\n");
        return -1;
    }
    printf("Client socket created\n");

    // Set server address
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(atoi(argv[1]));
    server_addr.sin_addr.s_addr = inet_addr(argv[2]);

    // Connect to server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Connection failed\n");
        close(sockfd);
        return -1;
    }

    printf("Connected to server\n");

    while (1)
    {
        printf("Enter command (time / date / exit): ");
        fgets(buff, sizeof(buff), stdin);
        buff[strcspn(buff, "\n")] = '\0';  // Remove newline

        if (strlen(buff) == 0)
            continue;

        send(sockfd, buff, strlen(buff), 0);

        if (strcmp(buff, "exit") == 0)
        {
            printf("Exiting...\n");
            break;
        }

        memset(buff, 0, sizeof(buff));
        recv(sockfd, buff, sizeof(buff), 0);
        printf("Server response: %s\n", buff);
    }

    close(sockfd);
    return 0;
}

