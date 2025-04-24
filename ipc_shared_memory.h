// Headers.h 

#ifndef IPC_SHARED_MEMORY_H
#define IPC_SHARED_MEMORY_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define SHM_SIZE 1024
#define SEM_KEY 1234
#define UDP_PORT 12345
#define MAX_BUFFER_SIZE 1024

// Semaphore union for initialization
union semun
{
    int val;
    struct semid_ds *buf;
    unsigned short *array;
};

                                                     // P operation (wait)
void sem_wait(int semid);

                                                      // V operation (signal)
void sem_signal(int semid);

                                                     // Function to create shared memory
int create_shared_memory(key_t key);

                                                    // Function to attach shared memory
char* attach_shared_memory(int shmid);

                                                   // Function to create semaphore
int create_semaphore(int sem_key);

                                                    // Function to perform the client-side UDP socket operation
void send_data_to_server(char* data, const char* server_ip);

                                                     // Function to receive data from the server
void receive_data_from_server();

                                                     // Function to read data from shared memory and send it to UDP server
void read_from_shared_memory_and_send(int semid, char* shm_ptr);

#endif // IPC_SHARED_MEMORY_H

