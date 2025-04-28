#ifndef COMMON_H
#define COMMON_H

#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>

#define SHM_KEY 0x1234    // Shared memory key
#define SEM_KEY 0x5678    // Semaphore key
#define SHM_SIZE 1024     // Size of shared memory
#define UDP_PORT 5000     // UDP server port

// Union required for semctl initialization
union semun {
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

#endif

