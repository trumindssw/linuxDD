//Team_A_Assignment3_V0.1  
/*-----------------------------------------------------------------------------------------
  Assignment Three
  IPC Code - shmem with Sockets (with MAKEIFLE, C, .h files ONLY)
  1. Create a shared memory segment between two processes.
  2. Use semaphores to synchronize access (to avoid race conditions).
  3. One process writes a string into shared memory, the other reads it.
  4. All ALL TIMES, how do you chek externally what is IN THE SH MEM
  5. Same as ABOVE, except that add Sockets to the code also - Client and Server (UDP Sockets ONLY)
  6. when the 2nd process reads the data written in mmory, put it on the SOCKET, so that the server gets the mesage
  ---------------------------------------------------------------------------------------------------------------------*/

#include "header.h" 

void sem_wait(int semid) {
	struct sembuf sb = {0, -1, 0};   // Semaphore buffer: wait on semaphore 0
	semop(semid, &sb, 1);            // Apply the semaphore operation
}

void sem_signal(int semid) {
	struct sembuf sb = {0, 1, 0};    // Semaphore buffer: signal semaphore 0
	semop(semid, &sb, 1);            // Apply the semaphore operation
}

int main()
{
	// Generate unique key using file and ID
	key_t key = ftok("log.txt", 65);

	// Get shared memory ID and semaphore ID using the key
	int shmid = shmget(key, 1024, 0666);
	int semid = semget(key, 1, 0666);

	// Error handling for shared memory
	if (shmid == -1) {
		perror("Shmget");
		exit(0);
	}

	// Error handling for semaphore
	if (semid == -1) {
		perror("Semget");
		exit(0);
	}

	// Attach to shared memory
	struct shared_data *data = (struct shared_data *)shmat(shmid, NULL, 0);
	if (data == (void *)-1) {
		perror("Shmat");
		exit(0);
	}

	while (1) {
		sem_wait(semid);  // Acquire semaphore before accessing shared memory

		if (data->flag == 1) {  // Check if data is ready
			printf("Reader read: %s\n", data->text);

			// Create UDP socket
			int sockfd = socket(AF_INET, SOCK_DGRAM, 0);
			if (sockfd < 0) {
				perror("socket");
				exit(1);
			} else {
				// Set destination (UDP server) address
				struct sockaddr_in dest;
				memset(&dest, 0, sizeof(dest));
				dest.sin_family = AF_INET;
				dest.sin_port = htons(PORT);                     // Predefined macro for server port
				dest.sin_addr.s_addr = inet_addr(SERVER_IP);     // Predefined macro for server IP

				// Send the shared memory data via UDP
				ssize_t sent = sendto(sockfd, data->text, strlen(data->text) + 1, 0,
						(struct sockaddr*)&dest, sizeof(dest));
				if (sent < 0) {
					perror("UDP send failed");
				} else {
					printf("Reader sent message to UDP server: %s\n", data->text);
				}

				close(sockfd);  // Close UDP socket
			}

			message1(data->text);  // Log the received message with process ID

			if (strcmp(data->text, "exit") == 0)
				break;  // Exit loop if exit command is received

			data->flag = 0;  // Mark data as read
		}

		sem_signal(semid);  // Release semaphore
	}

	// Detach and clean up shared memory and semaphore
	shmdt(data);
	shmctl(shmid, IPC_RMID, NULL);   // Remove shared memory segment
	semctl(semid, 0, IPC_RMID);      // Remove semaphore

	return 0;
}

