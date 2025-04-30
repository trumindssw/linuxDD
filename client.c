
// client.c



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
    int sockfd;                           // Socket File Descriptor
    struct sockaddr_in server_addr;       //  Server Address structure
    char buffer[MAX_BUF];                 // Buffer to receive srever response
    char input[MAX_BUF];                  // Buffer to store User Input

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
	    perror("Socket failed");          // Print error if socket creation fails
	    exit(1);                          // Exit the Program
    }

    server_addr.sin_family = AF_INET;                              //ipv4
    server_addr.sin_port = htons(PORT);                           // server port
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1");         // Server IP Adrress
								  //

	// Connect to the Srver
    if (connect(sockfd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Connect failed");
        exit(1);
    }
    printf("Connected to server.\n");

    // Communication Loop
    while (1)
    {
        printf("Enter command (time/date/exit): ");
        fgets(input, MAX_BUF, stdin);
        input[strcspn(input, "\n")] = 0; // Remove newline

        if (strcasecmp(input, "exit") == 0)
       	{
            break;
        }

        send(sockfd, input, strlen(input), 0);  // Send Command to server

        memset(buffer, 0, MAX_BUF);             // Clear receive buffer
        recv(sockfd, buffer, MAX_BUF, 0);       // receive Server Response

        printf("Server: %s\n", buffer);
    }

    close(sockfd);        // Close the socket
    return 0;              // Exit the Program


}

