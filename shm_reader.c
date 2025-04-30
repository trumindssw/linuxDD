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
	if (argc != 3) 
	{
		fprintf(stderr, "Usage: %s <port> <ip_addr>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	int port = atoi(argv[1]);

	// Create shared memory
	int shmid = shmget(SHM_KEY, SHM_SIZE, 0666);
	if (shmid == -1) 
	{
		perror("shmget");
		exit(EXIT_FAILURE);
	}
	//Attach to shared memory
	char *data = (char *)shmat(shmid, NULL, 0);
	if (data == (char *)-1) 
	{
		perror("shmat");
		exit(EXIT_FAILURE);
	}

	// Semaphore for synchronization access
	int semid = semget(SEM_KEY, 1, 0666);
	if (semid == -1) 
	{
		perror("semget");
		exit(EXIT_FAILURE);
	}

	// Create UDP socket
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) 
	{
		perror("socket");
		exit(EXIT_FAILURE);
	}

	struct sockaddr_in sa;
	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_port = htons(port);
	sa.sin_addr.s_addr = inet_addr(argv[2]);

	//Semaphore wait (P operation)
	struct sembuf p = {0, -1, 0};

	while (1) 
	{
		// Wait for writer signal
		semop(semid, &p, 1);

		printf("Message from shared memory: %s\n", data);

		// Send message to UDP server
		sendto(fd, data, strlen(data), 0, (struct sockaddr *)&sa, sizeof(sa));
		printf("Message sent to server on port %d via UDP.\n", port);

		//Exit on quit
		if (strcasecmp(data, "quit") == 0) 
		{
			break;
		}
	}

	// Cleanup
	close(fd);
	//Detach from shared memory
	shmdt(data);
	return 0;
}

