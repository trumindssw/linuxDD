#include <stdio.h>      
#include <stdlib.h>     
#include <string.h>    
#include <unistd.h>   
#include <arpa/inet.h> 
#include <signal.h>     
#include <time.h>       
#include <sys/time.h>   
#include "server.h"    

#define PORT 8080           
#define BUFFER_SIZE 1024
#define IDLE_WARNING 180  // 3 min
#define IDLE_SHUTDOWN 300 // 5 min (3 + 2)

time_t last_activity;       

// Function to handle communication with the client
void handle_client(int client_sock, int* warned) {
    char buffer[BUFFER_SIZE]; 
    int read_size;
    last_activity = time(NULL);  
    *warned = 0;

    // Loop to receive data from client
    if ((read_size = recv(client_sock, buffer, sizeof(buffer)-1, 0)) > 0) {
        buffer[read_size] = '\0';   
        last_activity = time(NULL);  
        if (strcasecmp(buffer, "time\n") == 0) {
            time_t now = time(NULL);
            char* time_str = ctime(&now);  
            send(client_sock, time_str, strlen(time_str), 0);
        }

        else if (strcasecmp(buffer, "date\n") == 0) {
            time_t now = time(NULL);
            struct tm *t = localtime(&now);
            char date[100];
            strftime(date, sizeof(date), "%Y-%m-%d\n", t);
            send(client_sock, date, strlen(date), 0);
        }
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

// Entry point
int main() {
    int server_fd, client_fd;
    struct sockaddr_in address;  
    int warned = 0;
    // Struct to hold address (IP + port)

    int opt = 1;                  
    socklen_t addrlen = sizeof(address);

    signal(SIGINT, graceful_exit);  
    server_fd = socket(AF_INET, SOCK_STREAM, 0);

    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

    // Fill out the address struct
    address.sin_family = AF_INET;          // IPv4
    address.sin_addr.s_addr = INADDR_ANY;  
    address.sin_port = htons(PORT);        
    bind(server_fd, (struct sockaddr *)&address, sizeof(address));

    printf("Server is using local port: %d\n", ntohs(address.sin_port));

    listen(server_fd, 3);

    printf("Server file descriptor: %d\n", server_fd);

    // Accept a client connection
    client_fd = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    printf("Client file descriptor: %d\n", client_fd);


    last_activity = time(NULL);  
    warned = 0;
    fd_set readfds;
    struct timeval timeout;

while (1) {
    FD_ZERO(&readfds);
    FD_SET(client_fd, &readfds);

    timeout.tv_sec = 1; 
    timeout.tv_usec = 0;

    int activity = select(client_fd + 1, &readfds, NULL, NULL, &timeout);

    if (activity < 0) perror("select error");
    else if (activity == 0) {
        double idle = difftime(time(NULL), last_activity);

    if (idle >= IDLE_SHUTDOWN) {
    printf("Server shutting down due to inactivity.\n");
    break;
} else if (idle >= IDLE_WARNING && !warned) {
    printf("No message from client for 3 min, will die in 2 min\n");
    warned = 1;
}

    } else {
        handle_client(client_fd, &warned);
    }
}

    // Cleanup
    close(client_fd);
    close(server_fd);

    return 0;
}
