#include <stdio.h>      // For printf
#include <stdlib.h>     // For exit
#include <string.h>     // For memset, strcmp etc
#include <unistd.h>     // For close, read, write, sleep
#include <arpa/inet.h>  // For sockaddr_in, inet_ntoa etc
#include <signal.h>     // For signal handling
#include <time.h>       // For time functions
#include <sys/time.h>   // For time-related structs
#include "server.h"     // Your own header file if you have it (else ignore)

#define PORT 8080           // The port number server listens on
#define BUFFER_SIZE 1024    // Size of buffer for receiving data
#define IDLE_TIMEOUT 180    // 3 min idle timeout (seconds)

time_t last_activity;       // To track last time we received something from client

// Function to handle communication with the client
void handle_client(int client_sock) {
    char buffer[BUFFER_SIZE];  // buffer to hold incoming data
    int read_size;

    last_activity = time(NULL);  // update last activity time

    // Loop to receive data from client
    while ((read_size = recv(client_sock, buffer, sizeof(buffer)-1, 0)) > 0) {
        buffer[read_size] = '\0';    // null-terminate the string
        last_activity = time(NULL);  // update last activity time

        // If client sends "time", send back current time string
        if (strcasecmp(buffer, "time\n") == 0) {
            time_t now = time(NULL);
            // time(NULL) returns int like 1720123456
            char* time_str = ctime(&now);  // ctime returns string like "Mon Jul 1 19:40:00 2025\n"
            send(client_sock, time_str, strlen(time_str), 0);
        }
        // If client sends "date", send back current date in YYYY-MM-DD format
        else if (strcasecmp(buffer, "date\n") == 0) {
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            char date[100];
            strftime(date, sizeof(date), "%Y-%m-%d\n", t);
            send(client_sock, date, strlen(date), 0);
        }
        // Otherwise send unknown command
        else {
            char *msg = "Unknown command\n";
            send(client_sock, msg, strlen(msg), 0);
        }
    }
}

// Handle Ctrl+C signal to exit gracefully
void graceful_exit(int signum) {
    printf("\nServer exiting gracefully.\n");
    exit(0);
}

int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;  
    // Struct to hold address (IP + port)
    int opt = 1;                  
    // Option value for setsockopt
    socklen_t addrlen = sizeof(address);

    signal(SIGINT, graceful_exit);  
    // Interrupt signal, when this process gets a signint (ex: ctrl+c), run this function instead of exiting immediately.
    // Register signal handler for Ctrl+C

    // Create socket
    // AF_INET = IPv4
    // SOCK_STREAM = TCP
    // 0 = default protocol (TCP)
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
    // Here we are adding an option to the socket using the setsocketopt function, we are adding the reuse address
    // If your server crashed or was stopped, and you restart it immediately, without this option, bind() might fail with Address already in use.
    // SOL_SOCKET = level to apply option (socket level)
    // SO_REUSEADDR = allow reuse of local addresses
    // &opt	Pointer to the option value (typically int opt = 1).
    // sizeof(opt)	Size of the option value.

    // Fill out the address struct
    address.sin_family = AF_INET;          // IPv4
    address.sin_addr.s_addr = INADDR_ANY;  // Listen on all available network interfaces (0.0.0.0)
    address.sin_port = htons(PORT);        // Convert port to network byte order (big endian)

    // Bind the socket to the IP + port
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));

    printf("Server is using local port: %d\n", ntohs(address.sin_port));
    // The bind() function binds the socket_fd with the address and port number specified in addr(custom data structure)
    // sever_fd: server file descriptor created using the socket() function.
    // bind() expects a generic pointer to struct sockaddr.But we have a struct sockaddr_in (IPv4-specific).So we cast: to match the function prototype.

    // Listen for incoming connections
    // 3 = backlog (number of connections allowed in pending queue)
    listen(server_fd, 3);

    printf("Server listening on port %d\n", PORT);
    printf("Server file descriptor: %d\n", server_fd);

    // Accept a client connection
    // This will block until a client connects
    client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    // Now the address contains the client address
    // we are only using the address struct as a variable
    // returns a new fd referring to that socket.

    printf("Client file descriptor: %d\n", client_fd);


    last_activity = time(NULL);  // start tracking time after client connects

    // Main loop: handle client & check for idle timeout
    while (1) {
        handle_client(client_fd);

        // If no data from client for IDLE_TIMEOUT seconds
        if (difftime(time(NULL), last_activity) > IDLE_TIMEOUT) {
            printf("No message from client for 3 min, will die in 2 min\n");
            sleep(120);  // Wait 2 min more
            printf("Shutting down due to inactivity.\n");
            break;
        }
    }

    // Cleanup
    close(client_fd);
    close(server_fd);

    return 0;
}
