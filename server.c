#include"header.h"

int main()
{
	//Create shared memory and attach it
        int shm_id = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
        if(shm_id < 0)
        {
                perror("shared memory creation failed");
                exit(1);
        }

        char *shm_ptr = shmat(shm_id, NULL, 0);
        if(shm_ptr == (void *)-1)
        {
                perror("shared memory attatchment failed");
                exit(1);
        }


        //Create semaphore to synchronize access
        int sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666);
        if(sem_id < 0)
        {
            perror("Semaphore creation failed");
                exit(0);
        }

        //Initialize semaphore value to 1 (binary semaphore)
        semctl(sem_id, 0, SETVAL, 1);


	//create socket
	int sockfd;
	struct  sockaddr_in sa,ca;
	socklen_t addr_len = sizeof(ca);
	char buffer[SHM_SIZE];
	struct sembuf wait_op, signal_op;
	
	sockfd = socket(AF_INET, SOCK_DGRAM, 0);

	sa.sin_family = AF_INET;
	sa.sin_port = htons(8080);
	sa.sin_addr.s_addr=INADDR_ANY;
	
	bind(sockfd,(struct sockaddr*)&sa,sizeof(sa));
	
	while(1)
	{

	//Lock
        wait_op.sem_num=0;
        wait_op.sem_op=-1; //wait
        wait_op.sem_flg=0;
        semop(sem_id,&wait_op,1);

	if (strlen(shm_ptr) > 0) 
	{
            strncpy(buffer, shm_ptr, SHM_SIZE);
            shm_ptr[0] = '\0';  // Clear shared memory

	       //UnLock
        	signal_op.sem_num=0;
        	signal_op.sem_op=1; //signal
        	signal_op.sem_flg=0;
        	semop(sem_id,&signal_op,1);

		sendto(sockfd, buffer, strlen(buffer), 0,(struct sockaddr *)&sa, sizeof(sa));
                printf("Sent via UDP: %s\n", buffer);

                if (strcmp(buffer, "exit") == 0)
                break;
	}
       	else 
	{
		signal_op.sem_num=0;
                signal_op.sem_op=1; //signal
                signal_op.sem_flg=0;
                semop(sem_id,&signal_op,1);

        }
        sleep(1);
        }


	close(sockfd);
	shmdt(shm_ptr);
        shmctl(shm_id, IPC_RMID, NULL);
        semctl(sem_id, 0, IPC_RMID);
	return 0;

}
