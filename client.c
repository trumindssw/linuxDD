#include "headers.h"

int main(int argc, char **argv)
{
    char buff[100];
    int fd;
    struct sockaddr_in sa;

    // Create socket for client
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1)
    {
        perror("socket");
        return 0;
    }
    puts("Socket created for client...\n");

    // Initialize server address
    sa.sin_family = AF_INET;
    sa.sin_port = htons(atoi(argv[1]));
    sa.sin_addr.s_addr = inet_addr(argv[2]);

    // Connect to server
    if (connect(fd, (const struct sockaddr *)&sa, sizeof(sa)) == -1)
    {
        perror("connect");
        return 0;
    }
    puts("Connected to server...\n");

    // Send commands to server
    while (1)
    {
        printf("Enter command (time, date, exit): ");
        fgets(buff, sizeof(buff), stdin);
        buff[strcspn(buff, "\n")] = 0;  // Remove newline character

        if (strcmp(buff, "exit") == 0)
        {
            send(fd, buff, strlen(buff), 0);
            break;
        }

        send(fd, buff, strlen(buff), 0);

        memset(buff, 0, sizeof(buff));
        recv(fd, buff, sizeof(buff), 0);
        printf("Server response: %s\n", buff);
    }

    close(fd);
    return 0;
}

