#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/mman.h>


#define SHM_KEY 0x12348
#define SEM_NAME "/my_phore"
#define SHM_NAME "/sharedMemory"
#define SHM_SIZE 1024
#define SHARED_FILE "shmem.txt"


#endif
