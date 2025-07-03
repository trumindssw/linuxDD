#ifndef SHARED_H
#define SHARED_H

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <semaphore.h>
#include <string.h>
#include <sys/stat.h>

#define SHM_KEY 7077
#define SHM_SIZE 1024
#define SEM_NAME "/mysem"
#define LOG_FILE "shmem.txt"


#endif
