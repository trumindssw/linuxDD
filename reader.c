#include "header.h"

int main(int argc,char *argv[]) {
    int shm_id = shmget(SHM_KEY, SHM_SIZE, 0666);
    if (shm_id == -1) { perror("shmget"); exit(1); }

    char *data = (char *)shmat(shm_id, NULL, 0);
    if (data == (void *)-1) { perror("shmat"); exit(1); }

    int sem_id = semget(SEM_KEY, 1, 0666);
    if (sem_id == -1) { perror("semget"); exit(1); 
    }
	int clientSocket, nBytes;
	char buffer[1024];
	struct sockaddr_in serverAddr;
	socklen_t addr_size;

	if(argc<2)
	{
		puts("input:./udpCli(exe) server_ip\n");
		return 0;
	}

	/*Create UDP socket*/
	clientSocket = socket(PF_INET, SOCK_DGRAM, 0);

	/*Configure settings in address struct*/
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons((PORT_NUM));
	serverAddr.sin_addr.s_addr = inet_addr(argv[1]);
	
//	memset(serverAddr.sin_zero, '\0', sizeof serverAddr.sin_zero);  

	/*Initialize size variable to be used later on*/
	addr_size = sizeof (serverAddr);

        FILE *fp=fopen("shmem.txt","a+");

    while (1) 
    {
        sem_wait(sem_id);  // Wait for writer to write
			   //
        printf("Reader: Read -> %s\n", data);
	strcpy(buffer,data);
	nBytes = strlen(buffer)+1;
        
        write_into_file(fp,buffer);

	sendto(clientSocket,buffer,nBytes,0,(struct sockaddr *)&serverAddr,addr_size);
         
	if (strcmp(data, "quit") == 0)
        {
            printf("Reader: Received 'quit', cleaning up.\n");
            break;
        }

    }

    shmdt(data);
    shmctl(shm_id, IPC_RMID, NULL);  // Delete shared memory
    semctl(sem_id, 0, IPC_RMID);     // Delete semaphore
    fclose(fp);
    return 0;
}

void sem_wait(int sem_id) {
    struct sembuf op = {0, -1, 0};  // P operation
    semop(sem_id, &op, 1);
}


/* note:
 
   if semaphore value is 0 it is in the blocked state and if any process is making the semaphore value less than 0 kernal did not allow to make value less than 0.*/
