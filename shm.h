#ifndef SHM_H
#define SHM_H

#define SHM_KEY 0x1234
#define SEM_KEY 0x5678
#define SHM_SIZE 1024

void init_shared_memory(int *shmid, char **data);
void init_semaphore(int *semid);
void lock_semaphore(int semid);
void unlock_semaphore(int semid);
void cleanup_shared_memory(int shmid);
void cleanup_semaphore(int semid);

#endif

