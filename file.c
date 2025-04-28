#include "myheader.h"
void lock_semaphore(int semid)
{
    struct sembuf op = {0, -1, 0};
    semop(semid, &op, 1);
}
void unlock_semaphore(int semid)
{
        struct sembuf op={0, 1, 0};
        semop(semid,&op,1);
}

void message1(char *message)
{
	FILE *fp=fopen(file,"a");
	if(fp==NULL)
	{
		perror("file not created");
		exit(1);
	}
	struct sembuf op_lock={0, -1, 0};
	semop(SEM_key,&op_lock,1);
	fprintf(fp, "Process ID: %d: %s\n", getpid(), message);
	struct sembuf op_unlock={0, 1, 0};
	semop(SEM_key,&op_unlock,1);
	fclose(fp);
}




