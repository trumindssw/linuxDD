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
        struct sembuf wait_op, signal_op;
	

	char buffer[SHM_SIZE];


	while(1)
	{
	//Write data into shared memory
	printf("Enter a message to share: ");
	fgets(shm_ptr, SHM_SIZE, stdin);

	//Remove newline
	buffer[strcspn(buffer, "\n")]=0;

	if(strcmp(shm_ptr, "exit") == 0)
        break;

	//Synchronize access (release semaphore after writing)
	
	wait_op.sem_num=0;
	wait_op.sem_op=-1; //wait
	wait_op.sem_flg=0;
	semop(sem_id,&wait_op,1);

	strncpy(shm_ptr, buffer, SHM_SIZE);


	signal_op.sem_num=0;
	signal_op.sem_op=1; //signal
	signal_op.sem_flg=0;
	semop(sem_id,&signal_op,1);

	sleep(1);

	}
	
	shmdt(shm_ptr);
	return 0;

}






