//Team_A_Assignment2_V0.2
/*IPC Code - Sockets (with MAKEIFLE, C, .h files ONLY)
1. Create a Socket - Client and Server code .
2. Add Grace ful command handling, so that on receive of a particular string like "Time" from the Server, the server responds with the Time. When "date" is sent, the server responds with "Date"
3. If ther is no message from the Client for X min (lets say 3 min), the server prints out a message, = will die in 2 min 
4. If the client doesnt send mesage in "X", the server with gracefully shutdown
Use "strace" in the background to see how the processes are working */
#include "myheader.h"
int main(int argc, char *argv[])
{
    char buff[100];
    int fd, client_fd;
    struct sockaddr_in address, client_addr;
    socklen_t addrlen = sizeof(client_addr);
    //socket creation
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        perror("Socket creation failed");
        return -1;
    }
    printf("Server socket created\n");
    //set server address parameters
    address.sin_family = AF_INET;
    address.sin_port = htons(atoi(argv[1]));
    address.sin_addr.s_addr = INADDR_ANY;
    //binding the socket to specified parameters
    if (bind(fd, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("Binding failed");
        close(fd);
        return -1;
    }
    printf("Binding successful\n");
    //start listening parameters
    if (listen(fd, 3) < 0)
    {
        perror("Listening failed");
        close(fd);
        return -1;
    }
    printf("Server is listening\n");
    //accept client connections
    client_fd = accept(fd, (struct sockaddr *)&client_addr, &addrlen);
    if (client_fd < 0)
    {
        perror("Connection not accepted");
        close(fd);
        return -1;
    }
    printf("Client connected successfully\n");

    char *clientIP = inet_ntoa(client_addr.sin_addr);
    int clientPort = ntohs(client_addr.sin_port);
    printf("Client IP: %s, Port: %d\n", clientIP, clientPort);

    fd_set readfds;
    struct timeval timeout;
    int idle_warning_given = 0;

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(client_fd, &readfds);
// Set timeout depending on whether idle warning has been given
	timeout.tv_sec = idle_warning_given ? SECOND_TIMEOUT : FIRST_TIMEOUT;
        timeout.tv_usec = 0;

        int activity = select(client_fd + 1, &readfds, NULL, NULL, &timeout);
        if (activity == 0)
        {
            if (!idle_warning_given)
            {
                puts("No message received in 3 min. Will shut down in 2 more min...");
                idle_warning_given = 1;
                continue;
            }
            else
            {
                puts("No activity. Server shutting down.");
                break;
            }
        }
	//recieve message from client
        bzero(buff, sizeof(buff));
        int recv_len = recv(client_fd, buff, sizeof(buff), 0);
        if (recv_len <= 0)
        {
            puts("Client disconnected or error. Server shutting down.");
            break;
        }

        printf("Received: %s\n", buff);
        idle_warning_given = 0;

        if (strcmp(buff, "exit") == 0)
        {
            puts("Client sent 'exit'. Server terminating.");
            break;
        }

        if (strcmp(buff, "time") == 0)
        {
            time_t now = time(NULL);
	    struct tm *tm_info = localtime(&now);
	    char time_str[20];
	    strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);//printing time
	    send(client_fd, time_str, strlen(time_str), 0);

        }
        else if (strcmp(buff, "date") == 0)
        {
            time_t now = time(NULL);
            struct tm *tm_info = localtime(&now);
            char date_str[50];
            strftime(date_str, sizeof(date_str), "%Y-%m-%d", tm_info);//printing date
            send(client_fd, date_str, strlen(date_str), 0);
        }
        else
        {
            send(client_fd, "Invalid command", 16, 0);
        }
    }
    //close sockets

    close(client_fd);
    close(fd);

    return 0;
}

