//Vemulapalli Devi Naga Valli
//Assignment Two
//Team - B
//Question
/*Assignment Three
IPC Code - shmem with Sockets (with MAKEIFLE, C, .h files ONLY)
1. Create a shared memory segment between two processes.
2. Use semaphores to synchronize access (to avoid race conditions).
3. One process writes a string into shared memory, the other reads it.
4. All ALL TIMES, how do you chek externally what is IN THE SH MEM
5. Same as ABOVE, except that add Sockets to the code also - Client and Server (UDP Sockets ONLY)
6. when the 2nd process reads the data written in mmory, put it on the SOCKET, so that the server gets the mesage
*/

#include "headers.h"

int main(int argc, char *argv[]) 
{
	if (argc != 2) 
	{
		fprintf(stderr, "Usage: %s <port>\n", argv[0]);
		exit(EXIT_FAILURE);
	}

	int port = atoi(argv[1]);

	// Create UDP socket
	int udpSocket = socket(AF_INET, SOCK_DGRAM, 0);
	if (udpSocket < 0) 
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in sa, ca;
	socklen_t addr_len = sizeof(ca);
	char buffer[SHM_SIZE];

	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = htonl(INADDR_ANY); // Same as "0.0.0.0"

	//Bind UDP socket
	if (bind(udpSocket, (struct sockaddr *)&sa, sizeof(sa)) < 0) 
	{
		perror("bind");
		close(udpSocket);
		exit(EXIT_FAILURE);
	}

	printf("UDP server listening on port %d...\n", port);

	//Receiving data and print
	while (1) 
	{
		ssize_t n = recvfrom(udpSocket, buffer, SHM_SIZE - 1, 0, (struct sockaddr *)&ca, &addr_len);
		if (n < 0) 
		{
			perror("recvfrom");
			continue;
		}

		buffer[n] = '\0'; // Null-terminate received data

		printf("Received data from client: %s\n", buffer);

		//Exit on quit
		if (strcasecmp(buffer, "quit") == 0) 
		{
			break;
		}
	}

	//Cleanup
	close(udpSocket);
	printf("UDP server terminated.\n");

	return 0;
}

