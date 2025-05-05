//Team_A_Assignment2_V0.1
/*-------------------------------------------------------------------------------------------------------
Assignment Two
IPC Code - Sockets (with MAKEIFLE, C, .h files ONLY)
1. Create a Socket - Client and Server code .
2. Add Grace ful command handling, so that on receive of a particular string like "Time" from the Server, the server responds with the Time. When "date" is sent, the server responds with "Date"
3. If ther is no message from the Client for X min (lets say 3 min), the server prints out a message, = will die in 2 min 
4. If the client doesnt send mesage in "X", the server with gracefully shutdown
Use "strace" in the background to see how the processes are working
----------------------------------------------------------------------------------------------------------*/
#include "headers.h" 

int main(int argc, char *argv[])
{
    // Check for correct number of arguments: program name, port, server IP
    if (argc != 3)
    {
        printf("Usage: %s <port> <server_ip>\n", argv[0]);
        return 1;
    }

    int sockfd;
    struct sockaddr_in server_addr;
    char buff[100];  // Buffer for input and receiving server responses

    // Create socket (IPv4, TCP)
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
    {
        printf("Socket creation failed\n");
        return -1;
    }
    printf("Client socket created\n");

    // Set up server address details
    server_addr.sin_family = AF_INET;                 // IPv4
    server_addr.sin_port = htons(atoi(argv[1]));      // Port (from command line)
    server_addr.sin_addr.s_addr = inet_addr(argv[2]); // Server IP (from command line)

    // Attempt to connect to the server
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Connection failed\n");
        close(sockfd);
        return -1;
    }

    printf("Connected to server\n");

    // Communication loop
    while (1)
    {
        printf("Enter command (time / date / exit): ");
        fgets(buff, sizeof(buff), stdin);                  // Get user input
        buff[strcspn(buff, "\n")] = '\0';                  // Remove trailing newline

        if (strlen(buff) == 0)                             // Ignore empty input
            continue;

        send(sockfd, buff, strlen(buff), 0);               // Send command to server

        if (strcmp(buff, "exit") == 0)                     // Handle exit condition
        {
            printf("Exiting...\n");
            break;
        }

        memset(buff, 0, sizeof(buff));                     // Clear buffer
        recv(sockfd, buff, sizeof(buff), 0);               // Receive server response
        printf("Server response: %s\n", buff);             // Print response
    }

    // Close the socket
    close(sockfd);
    return 0;
}

