#include "header.h"

int main() {
    int shm_id = shmget(SHM_KEY, SHM_SIZE, IPC_CREAT | 0666);
    if (shm_id == -1) { perror("shmget"); exit(1); }

    char *data = (char *)shmat(shm_id, NULL, 0);
    if (data ==(char *) -1) { perror("shmat"); exit(1); }

    int sem_id = semget(SEM_KEY, 1, IPC_CREAT | 0666);
    if (sem_id == -1) { perror("semget"); exit(1); }

    // Initialize semaphore to 0 
    semctl(sem_id, 0, SETVAL, 0);
    
     //opening file in append plus mode.so that we can add our data to the file  
    FILE *fp=fopen("shmem.txt","a+");

    write_into_file(fp,"The message logs starts from here for this trial:");


    while (1) {
        printf("Writer: Enter message (or 'quit'): ");
        fgets(data, SHM_SIZE, stdin);

        data[strcspn(data, "\n")] = 0;  // Remove newline

	//semaphore Goes from 0 → (writer adds 1) → wakes up reader → reader subtracts 1 → ends at 0
        sem_signal(sem_id);  // Signal reader 
        
	//writer writes data into the file
	write_into_file(fp,data);

        if (strcmp(data, "quit") == 0) break;
        sleep(1);
    }

    shmdt(data);
    return 0;
}

void sem_signal(int sem_id) {
    struct sembuf op = {0, 1, 0};  // V operation
    semop(sem_id, &op, 1);  //1 tells the kernel “There’s exactly one operation to perform, in 3rd argument we given
}

