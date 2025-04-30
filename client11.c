//KARUMANCHI NAVEEN
//ASSIGNMENT TWO
//Assignment Two
//IPC Code - Sockets (with MAKEIFLE, C, .h files ONLY)
//1. Create a Socket - Client and Server code .
//2. Add Grace ful command handling, so that on receive of a particular string like "Time" from the Server, the server responds with the Time. When "date" is sent, the server responds with "Date"
//3. If ther is no message from the Client for X min (lets say 3 min), the server prints out a message, = will die in 2 min
//4. If the client doesnt send mesage in "X", the server with gracefully shutdown
// Use "strace" in the background to see how the processes are working

#include "headers11.h"

// Utility function to print error message and exit
void error_exit(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

int main() {
	int sock = 0;
	struct sockaddr_in serv_addr;
	char buffer[BUFFER_SIZE] = {0};

	sock = socket(AF_INET, SOCK_STREAM, 0);
	if (sock < 0) error_exit("Socket creation error");

	//  Set up server address
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(PORT);
	serv_addr.sin_addr.s_addr = inet_addr("127.0.0.1");

	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		error_exit("Connection Failed");

	printf("Connected to server. Type 'Time' or 'Date' or 'exit' to quit.\n");
        //  Communication loop
	while (1) {
		fgets(buffer, BUFFER_SIZE, stdin);
		buffer[strcspn(buffer, "\n")] = '\0'; // Remove newline

		if (strlen(buffer) == 0) continue;
		//Exit if user types"exit"
		if(strcasecmp(buffer,"exit")==0)
		{
			printf("Exiting client\n");
			break;
		}	
		send(sock, buffer, strlen(buffer), 0);
		int valread = read(sock, buffer, BUFFER_SIZE);
		buffer[valread] = '\0';
		if (strcasecmp(buffer, "Unknown command") == 0) {
			printf("Server did not understand your command.\n");
		} else {
			printf(" Server Response: %s\n", buffer);
		}
		// Clear buffer for next use
		memset(buffer, 0, BUFFER_SIZE);
	}
	//  Close socket and exit

	close(sock);
	return 0;
}
