#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>

#include "shared.h"

static void trim_newline(char *s)
{
    size_t len = strlen(s);
    while (len > 0 && (s[len - 1] == '\n' || s[len - 1] == '\r'))
    {
        s[len - 1] = '\0';
        len--;
    }
}

static ssize_t send_all(int fd, const char *buf, size_t len)
{
    size_t sent = 0;
    while (sent < len)
    {
        ssize_t s = send(fd, buf + sent, len - sent, 0);
        if (s < 0)
        {
            if (errno == EINTR)
                continue;
            return -1;
        }
        sent += (size_t)s;
    }
    return (ssize_t)sent;
}

int main(void)
{
    int sockfd;
    struct sockaddr_in server_addr;
    char buf[BUF_SIZE];

    // Create socket 

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1)
    {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    // Server address 

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Connect to localhost 

    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) != 1)
    {
        fprintf(stderr, "inet_pton failed\n");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("connect");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("Connected to server on port %d\n", PORT);
    printf("Type commands: time | date | quit | anything else \n");

    while (1)
    {
        printf(" \n > ");
        fflush(stdout);

        if (!fgets(buf, sizeof(buf), stdin))
        {

            printf("Input closed.\n");
            break;
        }

        trim_newline(buf);

        if (strlen(buf) == 0)
        {
            continue;
        }

      //  Send to server 

        size_t cur_len = strlen(buf);
        if (cur_len + 1 < sizeof(buf))
        {
            buf[cur_len] = '\n';
            buf[cur_len + 1] = '\0';
        }
        else
        {

            buf[sizeof(buf) - 2] = '\n';
            buf[sizeof(buf) - 1] = '\0';
        }

        ssize_t n = send_all(sockfd, buf, strlen(buf));
        if (n == -1)
        {
            perror("send");
            break;
        }

      // If we typed quit, server will close soon 

        if (strncmp(buf, "quit", 4) == 0)
        {
            printf("Sent quit. \n Waiting for server message ...\n");
        }

        // Receive reply 

        n = recv(sockfd, buf, sizeof(buf) - 1, 0);
        if (n > 0)
        {
            buf[n] = '\0';
            printf("Server: %s", buf);
        }
        else if (n == 0)
        {
            printf("Server closed connection.\n");
            break;
        }
        else
        {
            perror("recv");
            break;
        }
    }

    close(sockfd);
    return 0;
}
