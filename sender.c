#include "myheader.h"

int main()
{
	//shared memory creation

    int shmid = shmget(SHM_key, SIZE, IPC_CREAT | 0666);
    if (shmid == -1)
    {
	    perror("shmget");
	    exit(1);
    }
	//attach shared memory
    struct Data *data = (struct Data *)shmat(shmid, NULL, 0);
    if (data == (void *)-1)
    {
	    perror("shmat");
	    exit(1);
    }
	//semaphore creation
    int semid = semget(SEM_key, 1, IPC_CREAT | 0666);
    if (semid == -1)
    {
	    perror("semget");
	    exit(1);
    }
	//set semaphore value
    semctl(semid, 0, SETVAL, 0);
    char msg2[1000];
    printf("Enter message to send to receiver: ");
    fgets(msg2, sizeof(msg2), stdin);
    if(msg2[strlen(msg2)-1]==10)
    {
	    msg2[strlen(msg2)-1]='\0';
    }
    strcpy(data->msg, msg2);
    unlock_semaphore(semid);
    message1(data->msg);
	//detach shared memory
    shmdt(data);
    printf("Message sent: %s\n", msg2);
    return 0;
}



