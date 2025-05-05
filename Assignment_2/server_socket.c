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
    char buff[100];                          // Buffer for receiving messages
    int fd, client_fd;                       // File descriptors for server socket, client_fd for accepted connection
    struct sockaddr_in server_addr, client_addr;  // Structures to hold server and client addresses
    socklen_t addr_len = sizeof(client_addr);     // Length of the client address

    // Create a TCP socket (IPv4, stream-based)
    fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0)
    {
        printf("Socket creation failed\n");
        return -1;
    }
    printf("Server socket created\n");

    server_addr.sin_family = AF_INET;                    // IPv4
    server_addr.sin_port = htons(atoi(argv[1]));         // Port number passed as argument, converted to network byte order
    server_addr.sin_addr.s_addr = INADDR_ANY;            // Accept connections from any IP address

    // Bind socket to the specified address and port
    if (bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        printf("Binding failed\n");
        close(fd);      // Close socket before exiting
        return -1;
    }
    printf("Binding successful\n");

    // Listen for incoming connections (backlog = 3)
    if (listen(fd, 3) < 0)
    {
        printf("Listening failed\n");
        close(fd);
        return -1;
    }
    printf("Server is listening\n");

    // Accept a client connection (blocking call)
    client_fd = accept(fd, (struct sockaddr *)&client_addr, &addr_len);
    if (client_fd < 0)
    {
        printf("Connection not accepted\n");
        close(fd);
        return -1;
    }
    printf("Client connected\n");

    while (1)
    {
        memset(buff, 0, sizeof(buff));                  // Clear the buffer before receiving
        recv(client_fd, buff, sizeof(buff), 0);         // Receive message from client
        printf("Client: %s\n", buff);                   // Display received message

        // Handle 'exit' message
        if (strcmp(buff, "exit") == 0)
        {
            printf("Client sent 'exit'. Closing server.\n");
            break;
        }
        // Handle 'time' request
        else if (strcmp(buff, "time") == 0)
        {
            time_t now = time(NULL);
            struct tm *tm_info = localtime(&now);
            char time_str[20];
            strftime(time_str, sizeof(time_str), "%H:%M:%S", tm_info);  // Format time as HH:MM:SS
            send(client_fd, time_str, strlen(time_str), 0);             // Send time to client
        }
        // Handle 'date' request
        else if (strcmp(buff, "date") == 0)
        {
            time_t now = time(NULL);
            struct tm *tm_info = localtime(&now);
            char date_str[20];
            strftime(date_str, sizeof(date_str), "%Y-%m-%d", tm_info);  // Format date as YYYY-MM-DD
            send(client_fd, date_str, strlen(date_str), 0);             // Send date to client
        }

        // Additional commands can be added here
    }

    // Close client and server sockets
    close(client_fd);
    close(fd);

    return 0;
}
