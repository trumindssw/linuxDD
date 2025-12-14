#ifndef SHARED_H
#define SHARED_H

#include <sys/ipc.h>
#include <sys/shm.h>

#define SHM_KEY   1234
#define SHM_SIZE  256

#define MUTEX_SEM "/mutex_sem"
#define DATA_SEM "/data_sem"

#define LOG_FILE  "shmem.txt"

#endif
