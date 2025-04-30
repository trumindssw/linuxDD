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

// Semaphore union for initializing semaphore value
union semun 
{
	int val;
};

int main() 
{
	// Create shared memory
	int shmid = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
	if (shmid == -1) 
	{
		perror("shmget");  // Failed to create/get shared memory
		exit(EXIT_FAILURE);
	}

	// Attach to shared memory
	char *data = (char *)shmat(shmid, NULL, 0);
	if (data == (char *)-1) 
	{
		perror("shmat");  // Failed to attach to shared memory
		exit(EXIT_FAILURE);
	}

	// Create semaphore for synchronizing access 
	int semid = semget(SEM_KEY, 1, IPC_CREAT | 0666);
	if (semid == -1) 
	{
		perror("semget");
		exit(EXIT_FAILURE);
	}

	// Initialize semaphore value to 0
	union semun sem_val;
	sem_val.val = 0;
	semctl(semid, 0, SETVAL, sem_val);

	// Semaphore signal (V operation)
	struct sembuf v = {0, 1, 0};

	while (1) 
	{
		// Get input and store in shared memory
		printf("Enter a message: ");
		char *input = fgets(data, SHM_SIZE, stdin);
		if (!input) 
		{
			perror("fgets");
			break;
		}

		// Remove newline if present
		if (data[strlen(data) - 1] == '\n') 
		{
			data[strlen(data) - 1] = '\0';
		}

		// Signal reader
		semop(semid, &v, 1);

		printf("Message written to shared memory\n");

		// Exit on quit
		if (strcasecmp(data, "quit") == 0) 
		{
			break;
		}
	}

	// Detach from shared memory
	shmdt(data);

	// Remove shared memory segment
	shmctl(shmid, IPC_RMID, NULL);

	// Remove semaphore
	semctl(semid, 0, IPC_RMID);

	return 0;
}

