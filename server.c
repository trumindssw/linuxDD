// server.c

//Team_ C_Assignment2_V0.2
//
/*
Assignment Two
IPC Code - Sockets (with MAKEIFLE, C, .h files ONLY)
1. Create a Socket - Client and Server code .
2. Add Grace ful command handling, so that on receive of a particular string like "Time" from the Server, the server responds with the Time. When "date" is sent, the server responds with "Date"
3. If ther is no message from the Client for X min (lets say 3 min), the server prints out a message, = will die in 2 min 
4. If the client doesnt send mesage in "X", the server with gracefully shutdown
// Use "strace" in the background to see how the processes are working   */
 



#include "header.h"

int main()
{
    int sockfd, new_sock;                               // Server socket and client socket
    struct sockaddr_in server_addr, client_addr;        // Server and client address structures
    socklen_t addr_size;                                // Size of client address structure
    char buffer[MAX_BUF];                               //   Buffer for receiving/sending data
    fd_set readfds;                                     // Set of file descriptors for select()
    struct timeval timeout;                            
    int activity;
    time_t last_activity, current_time;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
	    perror("Socket failed");                       // Error handling
	    exit(1);
    }

    server_addr.sin_family = AF_INET;                          // IPV4
    server_addr.sin_addr.s_addr = INADDR_ANY;                   // Listen all Interfaces
    server_addr.sin_port = htons(PORT); 

    if (bind(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");                             // Error On Bind
        exit(1);
    }

    if (listen(sockfd, 3) < 0)                    // Start listeneing for incoming connections
    {
        perror("Listen failed");
        exit(1);
    }
    printf("Server listening on port %d\n", PORT);

    addr_size = sizeof(client_addr);
    new_sock = accept(sockfd, (struct sockaddr*)&client_addr, &addr_size);
    if (new_sock < 0)
    {
	    perror("Accept failed");
	    exit(1);
    }

    printf("Connection accepted.\n");
    time(&last_activity);                  // Initialize last activity time

    while (1)
    {
        FD_ZERO(&readfds);
        FD_SET(new_sock, &readfds);
        
        timeout.tv_sec = 5; // Check every 5 seconds
        timeout.tv_usec = 0;

        activity = select(new_sock + 1, &readfds, NULL, NULL, &timeout);
        
        if (activity < 0 && errno != EINTR)
       	{
            perror("Select error");
        }
        
        if (activity == 0)
       	{ 
            // Timeout - No data, check idle time
            time(&current_time);
            if (difftime(current_time, last_activity) >= IDLE_TIME)
	    {
                printf("No message for 3 min... will die in 2 min\n");
            }
            if (difftime(current_time, last_activity) >= (IDLE_TIME + DIE_TIME))
	    {
                printf("No activity, server shutting down gracefully.\n");
                break;
            }
            continue;
        }

        memset(buffer, 0, MAX_BUF);                    // clear buffer before receiving
        int valread = recv(new_sock, buffer, MAX_BUF, 0);
        if (valread <= 0)
       	{
            printf("Client disconnected. Shutting down server.\n");
            break;
        }

        printf("Client: %s\n", buffer);      // Print Client Message

        if (strcasecmp(buffer, "time") == 0)
       	{
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            strftime(buffer, MAX_BUF, "%H:%M:%S", t);
            send(new_sock, buffer, strlen(buffer), 0);
        }
       	else if (strcasecmp(buffer, "date") == 0)
       	{
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            strftime(buffer, MAX_BUF, "%Y-%m-%d", t);
            send(new_sock, buffer, strlen(buffer), 0);
        }
       	else
       	{
            send(new_sock, "Unknown command", strlen("Unknown command"), 0);
        }

        time(&last_activity); // reset activity timer
    }

    close(new_sock);
    close(sockfd);
    return 0;
}

