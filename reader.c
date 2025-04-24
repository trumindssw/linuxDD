#include "header.h"
void sem_wait(int semid) {
    struct sembuf sb = {0, -1, 0};
    semop(semid, &sb, 1);
}

void sem_signal(int semid) {
    struct sembuf sb = {0, 1, 0};
    semop(semid, &sb, 1);
}
int main()
{
	key_t key=ftok("log.txt",65);
	int shmid=shmget(key,1024,0666);
	int semid=semget(key,1,0666);
	if(shmid==-1)
	{
		perror("Shmget");
		exit(0);
	}
	if(semid==-1)
	{
		perror("semid");
		exit(0);
	}
	char *data=(char *)shmat(shmid,NULL,0);
	if(data==(char *)-1)
	{
		perror("Shmat");
		exit(0);
	}
	sem_wait(semid);
	printf("Reader read:%s",data);
	message1(data);
	sem_signal(semid);
	shmdt(data);
	semctl(semid, 0, IPC_RMID);
	shmctl(shmid,IPC_RMID,NULL);
	return 0;
}
