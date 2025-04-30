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
    if (argc != 3)
    {
        fprintf(stderr, "Usage: %s <port> <server_ip>\n", argv[0]);
        return 1;
    }

    int fd1;
    struct sockaddr_in serveraddr;
    char buff[100];
    //create socket

    fd1 = socket(AF_INET, SOCK_STREAM, 0);
    if (fd1 < 0)
    {
        perror("Socket creation failed");
        return -1;
    }
    printf("Client socket created\n");
    //set server address parameters
    serveraddr.sin_family = AF_INET;
    serveraddr.sin_port = htons(atoi(argv[1]));
    serveraddr.sin_addr.s_addr = inet_addr(argv[2]);
    //connect server
    if (connect(fd1, (struct sockaddr *)&serveraddr, sizeof(serveraddr)) < 0)
    {
        perror("Connection failed");
        close(fd1);
        return -1;
    }

    printf("Connected to server successfully\n");
    puts("Initiating 3-Way Handshake with Server...");

    while (1)
    {
	printf("enter exit to terminate the program\n");
        printf("Enter command (time/date): ");
        fgets(buff, sizeof(buff), stdin);
	if(buff[strlen(buff)-1]==10)
	{
		buff[strlen(buff)-1]='\0';
	}
	if (strlen(buff) == 0)
        {
            puts("Empty input. Skipping.\n");
            continue;
        }
	//send command to server
	 send(fd1, buff, strlen(buff), 0);

        if (strcmp(buff, "exit") == 0)
        {
            puts("Exiting as requested.");
            break;
        }

        bzero(buff, sizeof(buff));
        int n = recv(fd1, buff, sizeof(buff), 0);
        if (n <= 0)
        {
            perror("Server closed or error");
            break;
        }

        printf("Server response: %s\n", buff);
    }

    puts("Client Terminating Connection With Server");
    close(fd1);
    return 0;
}

