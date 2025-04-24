#include"header.h"
union semun{
	int val;
};
int main()
{
	int shmid=shmget(SHM_KEY, 1024,0666 | IPC_CREAT);
	SharedData *data=(SharedData *)shmat(shmid,NULL,0);
	int semid = semget(SEM_KEY, 1, 0666 | IPC_CREAT);
	union semun arg;
	arg.val = 1;
	semctl(semid, 0, SETVAL, arg);
	struct sembuf lock = {0, -1, 0};
	struct sembuf unlock = {0, 1, 0};
	char input[MAX_MESSAGE_LEN];
	printf("Enter a message to send: ");
	fgets(input, MAX_MESSAGE_LEN, stdin);
	input[strcspn(input, "\n")] = '\0';
	semop(semid, &lock, 1);
	strncpy(data->message, input, MAX_MESSAGE_LEN);
	semop(semid, &unlock, 1);
	FILE *log = fopen("shmem.txt", "a");
	fprintf(log, "Process ID: %d: %s\n", getpid(), input);
	fclose(log);
	shmdt(data);
	return 0;
}
